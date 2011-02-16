#include "MainController.h"
#include <QSettings>
#include <QTimer>
#include <QtAlgorithms>
#include "BaseNode.h"
#include "GsmModuleController.h"
#include "Packets.h"
#include "PacketSlots.h"
#include "StatusView.h"
#include "Window.h"

MainController::MainController(QObject *parent) : QObject(parent)
{
    window = new Window();
	getPreferences();
	initMembers();

	if (!preferences.deployed)
		QTimer::singleShot(1000, this, SLOT(deployNetwork()));
}

MainController::~MainController()
{
}

void MainController::getPreferences()
{
	// Read the pref file
	QSettings pref("/wsn/gateway/preferences", QSettings::IniFormat);

	// Check for preference values, if not valid then re-generate
	preferences.nodePort = pref.value("serials/nodePort").toString();
	if (preferences.nodePort == "")
	{
		preferences.nodePort = QString::fromAscii("/dev/ttyUSB0");
		pref.setValue("serials/nodePort", QVariant(preferences.nodePort));
	}
	preferences.nodePort = pref.value("serials/nodePort").toString();
	if (preferences.nodePort == "")
	{
		preferences.nodePort = QString::fromAscii("/dev/ttyUSB1");
		pref.setValue("serials/nodePort", QVariant(preferences.nodePort));
	}
	preferences.deployed = pref.value("wsn/deployed").toBool();
}

void MainController::initMembers()
{
	baseNode = new BaseNode(preferences.nodePort, this);
	gsmControl = new GsmModuleController(preferences.gsmPort,
										 "0952650121",
										 this);
	wsnFlowTimer = new QTimer(this);
	wsnFlowTimer->setSingleShot(true);
	connect(wsnFlowTimer, SIGNAL(timeout()), this, SLOT(wsnFlowFired()));

	// Connect packet receiving to parsing
	connect(baseNode, SIGNAL(receivedPacket(QList<uint8_t>)),
			&packParser, SLOT(processPacket(QList<uint8_t>)));

	// Connect packet parser events
	connect(&packParser, SIGNAL(gotMaxTier(int)), this, SLOT(setMaxTier(int)));
	connect(&packParser, SIGNAL(gotFirstTierNode()),
			this, SLOT(addFirstTierNode()));
	connect(&packParser, SIGNAL(gotNodePath(int, int, bool)),
			this, SLOT(addPath(int, int, bool)));
	connect(&packParser, SIGNAL(gotData(NodeData, bool)),
			this, SLOT(addData(NodeData, bool)));
	connect(&packParser, SIGNAL(hadAwakeNode()), this, SLOT(wakeNetwork()));
	connect(&packParser, SIGNAL(shouldReroute()), this, SLOT(deployNetwork()));
}

void MainController::deployNetwork()
{
	wsnFlowTimer->stop();
	clearLog();
	step = WSN_STEP_NOT_DEPLOYED;

	stepSatisfied = false;

	wsnParams.hasRootNodes = false;
	wsnParams.maxTier = 0;
	wsnParams.nodeAndParentIds.clear();
	wsnParams.dataOfNodeIds.clear();

	wsnFlowTimer->start(1);
}

void MainController::wsnFlowFired()
{
	step++;
	if (step == WSN_STEP_DEPLOY_START)
	{
		log(QString("Will deploy via ") + baseNode->path());
		wsnFlowTimer->start(1000);
	}
	else if (step == WSN_STEP_DEPLOY_RESET)
	{
		log("Nodes will reset...");
		baseNode->sendPacket(PACKET_RESET);
		wsnFlowTimer->start(5000);
	}
	else if (step < WSN_STEP_DEPLOY_REQUEST_PATH)
	{
		// Print the one-time Phrase start
		// Flag used here is reset in the next step (WSN_DEPLOY_REQUEST_PATH)
		if (!stepSatisfied)
		{
			log("\nSending deploy command: ", false);
			stepSatisfied = true;
		}

		// Print count of deploy command
		log(QString::number(step - WSN_STEP_DEPLOY_RESET) + " ", false);

		// Send the command
		baseNode->sendPacket(PACKET_START_DEPLOY);

		// Run timer
		wsnFlowTimer->start(1000);
	}
	else if (step == WSN_STEP_DEPLOY_REQUEST_PATH)
	{
		// Reset flag used in previous step (WSN_DEPLOY_REQUEST_PATH)
		stepSatisfied = false;

		log("Requesting for path");
		baseNode->sendPacket(PACKET_REQUEST_PATH);
		wsnFlowTimer->start(10000);
		// Now wait for max tier packets and path return packets
		// The minimum wait time is 10s, and if no-one returns this path
		//  request, we re-deploy the network
		// If there are packets received, waiting ends after no more packets
		//  are received for more than 3 seconds
		// See comments in setMaxTier() and addPath()
	}
	else if (step == WSN_STEP_DEPLOY_DISTRIBUTE_TIME_SLOTS)
	{
		// First check for the path result from the previous step
		// Reroute in 3 seconds if the path and tier settings are not complete
		if ((!wsnParams.hasRootNodes) ||
			(wsnParams.nodeAndParentIds.size() == 0))
		{
			QTimer::singleShot(3000, this, SLOT(deployNetwork()));
			log("Network deployment failed, will reroute in 3 seconds...");
			return;
		}

		// Ask nodes to distribute time slots, thus finish deploying
		baseNode->sendPacket(PACKET_DISTRIBUTE_TIME_SLOTS);
		QString s;
		s.sprintf("Deployment finished for %d nodes",
				  wsnParams.nodeAndParentIds.size());
		log(s);

		wsnFlowTimer->start(100);
	}
	else if (step == WSN_STEP_DEPLOY_FINISH)
	{
		// Return the deployment info via SMS
		log("Returning path info...");
		sendPathSmss();
	}
}

void MainController::setMaxTier(int tier)
{
	// Update max tier
	if (wsnParams.maxTier < tier)
	{
		wsnParams.maxTier = tier;
		log("Max tier updated: " + QString::number(tier));
	}

	// Refresh timer if needed
	// The minimum waiting time is 10 seconds, or no more packets are received
	//  for 3 seconds
	// See comments in wsnFlowFired()
	if (wsnFlowTimer->interval() < 3000)
		wsnFlowTimer->setInterval(3000);
}

void MainController::addFirstTierNode()
{
	if (!wsnParams.hasRootNodes)
	{
		wsnParams.hasRootNodes = true;
		log("Root node discovered");
	}
}

void MainController::addPath(int nodeId, int parentId, bool isRelayed)
{
	// Add the node/parent combination into table if not existed
	if (!wsnParams.nodeAndParentIds.contains(nodeId))
	{
		wsnParams.nodeAndParentIds.insert(nodeId, parentId);
		QString s;
		if (isRelayed)
			s.sprintf("New path relayed:  %2d via %2d", nodeId, parentId);
		else
			s.sprintf("New path returned: %2d via %2d", nodeId, parentId);
		log(s);

		// If the node has parent ID 1 (sink), add its ID as a root node ID
		if (parentId == 1 && !wsnParams.rootNodeIds.contains(nodeId))
			wsnParams.rootNodeIds.insert(nodeId);
	}

	// Refresh timer if needed
	// The minimum waiting time is 10 seconds, or no more packets are received
	//  for 3 seconds
	// See comments in wsnFlowFired()
	if (wsnFlowTimer->interval() < 3000)
		wsnFlowTimer->setInterval(3000);
}

//void MainController::addData(NodeData data, bool isSupplemental)
void MainController::addData(NodeData, bool)
{
}

void MainController::wakeNetwork()
{
}

void MainController::sendPathSmss()
{
	// Calculate how many SMSs are needed
	int nodeCount = wsnParams.nodeAndParentIds.size();
	int smsCount = nodeCount / 10;
	if (nodeCount % 10 != 0)
		smsCount++;

	// Format and send SMSs
	QList<int> nodeIds = wsnParams.nodeAndParentIds.keys();
	qSort(nodeIds.begin(), nodeIds.end());
	for (int i = 0; i < smsCount; i++)
	{
		// Construct header
		QString sms("s02;v2;");
		sms.append(QString::number(smsCount) + ';');
		sms.append(QString::number(i + 1) + ';');

		// Construct node list
		for (int i = 0; i < 10; i++)
		{
			if (nodeCount == 0)
				break;
			int nodeId = nodeIds[nodeCount - 1];
			sms.append(QString::number(nodeId) +
					   ",1," +
					   QString::number(wsnParams.nodeAndParentIds[nodeId]) +
					   ';');
			nodeCount--;
		}

		// Send the constructed SMS
		gsmControl->sendSmsCommand(sms);
	}
}

void MainController::log(QString text, bool inOwnLine)
{
	window->statusTab()->log(text, inOwnLine);
}

void MainController::clearLog()
{
	window->statusTab()->clearLog();
}
