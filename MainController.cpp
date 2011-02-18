#include "MainController.h"
#include <QDateTime>
#include <QDir>
#include <QSettings>
#include <QTextStream>
#include <QTimer>
#include <QtAlgorithms>
#include "BaseNode.h"
#include "GsmModuleController.h"
#include "Packets.h"
#include "PacketSlots.h"
#include "StatusView.h"
#include "Window.h"
#include <QDebug>

Q_DECLARE_METATYPE(QList<int>)

MainController::MainController(QObject *parent) : QObject(parent)
{
	preferences = new Preferences();
	initMembers();

	// If the network is not deployed, do it now
	// Otherwise load the deployment settings from file
	if (!preferences->isDeployed())
		QTimer::singleShot(1000, this, SLOT(deployNetwork()));
	else
	{
		log("Existing network status detected, resume without deploying");
		preferences->loadDeployParamsInto(&wsnParams);
	}
}

MainController::~MainController()
{
	delete preferences;
}

void MainController::initMembers()
{
	window = new Window();

	baseNode = new BaseNode(preferences->nodePort(), this);
	gsmControl
		= new GsmModuleController(preferences->gsmPort(),
								  preferences->serverPhone(),
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

	// Connect UI events
	connect(window, SIGNAL(clearLogTriggered()), this, SLOT(clearLog()));
	connect(window, SIGNAL(deployTriggered()), this, SLOT(deployNetwork()));
	connect(window, SIGNAL(collectTriggered()), this, SLOT(collectData()));
}

void MainController::deployNetwork()
{
	wsnFlowTimer->stop();
	clearLog();
	step = WSN_STEP_NOT_DEPLOYED;

	stepSatisfied = false;

	preferences->setIsDeployed(false);
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
		preferences->setIsDeployed(true);
		wsnFlowTimer->start(1);
	}
	else if (step == WSN_STEP_HAS_DEPLOYED)
	{
		// Wait 10 seconds before collecting data
		QTimer::singleShot(10000, this, SLOT(collectData()));
		log("Will collect in 10 seconds...");
	}
	else if (step == WSN_STEP_COLLECT_START)
	{
		log("Ordering first-tier nodes to collect data");
		wsnParams.dataOfNodeIds.clear();
		baseNode->sendPacket(PACKET_COLLECT_START);

		wsnFlowTimer->start(5000);
	}
	else if (step == WSN_STEP_COLLECT_REQUEST)
	{
		log("Requesting data from first-tier nodes");
		wsnParams.rootNodeIdsToCollect = wsnParams.rootNodeIds.toList();

		wsnFlowTimer->start(1);
	}
	else if (step == WSN_STEP_COLLECT_REQUESTING)
	{
		int thisId = wsnParams.rootNodeIdsToCollect.takeFirst();
		log("Sending request to node " + QString::number(thisId));
		QList<uint8_t> req = PACKET_COLLECT_REQUEST;
		req[GATEWAY_PACKET_RECEIVER] = (uint8_t)thisId;
		baseNode->sendPacket(req);
		if (!wsnParams.rootNodeIdsToCollect.isEmpty())
			step--;

		wsnFlowTimer->start(5000);
	}
	else if (step == WSN_STEP_COLLECT_WAIT_TO_RECEIVE)
	{
		log("All requests sent, waiting for data...");
		wsnFlowTimer->start(10000);
	}
	else if (step == WSN_STEP_SYNCHRONIZE)
	{
		log("Data collection finished");
		log("Sending synchronization command");
		baseNode->sendPacket(PACKET_SYNCHRONIZE);
		wsnFlowTimer->start(5000);
	}
	else if (step == WSN_STEP_SUPPLEMENTAL_COLLECT_AND_SLEEP)
	{
		QList<uint8_t> slp = PACKET_SLEEP;
		uint16_t timeToSleep = getTimeToSleep();
		slp[GATEWAY_PACKET_SLEEP_TIME_HI] = timeToSleep / 0x100;
		slp[GATEWAY_PACKET_SLEEP_TIME_LO] = timeToSleep % 0x100;
		baseNode->sendPacket(slp);
		log("Nodes will sleep for " +
			QString::number(timeToSleep) +
			" seconds after supplemental collection");
		wsnFlowTimer->start(5000);
	}
	else if (step == WSN_STEP_COLLECT_FINISH)
	{
		wsnFlowTimer->stop();
		log("Supplemental collection finished, nodes should be asleep now");
		log("Data collected from " +
			QString::number(wsnParams.dataOfNodeIds.size()) +
			" node(s)");
		log("Will return data via GSM...");
		sendDataSmss();
	}
	else
	{
		wsnFlowTimer->start(1);
	}
}

void MainController::collectData()
{
	wsnFlowTimer->stop();
	clearLog();

	if (isNetworkCollectable())
		step = WSN_STEP_NOT_COLLECTED;
	else
		step = WSN_STEP_NOT_DEPLOYED;

	wsnFlowTimer->start(1);
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
			s.sprintf("New path relayed:  %d via %d", nodeId, parentId);
		else
			s.sprintf("New path returned: %d via %d", nodeId, parentId);
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

bool MainController::isNetworkCollectable()
{
	// List of invalid network parameters
	if ((!wsnParams.hasRootNodes) ||
		(wsnParams.maxTier < 1) ||
		(wsnParams.rootNodeIds.size() == 0) ||
		(wsnParams.nodeAndParentIds.size() == 0))
	{
		qDebug() << "root" << wsnParams.hasRootNodes
				<< "maxTier" << wsnParams.maxTier
				<< "root ids" << wsnParams.rootNodeIds
				<< "nodes/parents" << wsnParams.nodeAndParentIds;
		return false;
	}

	return true;
}

void MainController::addData(NodeData data, bool isSupplemental)
{
	int sourceId = data.dataSourceNodeId;
	if (wsnParams.dataOfNodeIds.contains(sourceId))
		return;

	QString l;
	// ACK the data packet if needed
	if (!isSupplemental)
	{
		QList<uint8_t> ack = PACKET_COLLECT_ACK;
		ack[GATEWAY_PACKET_RECEIVER] = data.senderNodeId;
		ack[GATEWAY_PACKET_DEPTH] = data.dataSourceTier;
		baseNode->sendPacket(ack);

		l = "Received collected data from node ";
	}
	else
	{
		wsnFlowTimer->setInterval(5000);
		l = "Received supplementally collected data from node ";
	}
	l.append(QString::number(data.dataSourceNodeId) + ':');
	log(l);

	wsnParams.dataOfNodeIds.insert(sourceId, data);
	l.sprintf("Temp %.1f; Hmdy %.1f; Illm %d; Pest %d",
			  data.temperature / 10.0,
			  data.humidity / 10.0,
			  data.par,
			  data.pest);
	log(l);

	// If all nodes are collected, go to the next step immediately
	if (wsnParams.dataOfNodeIds.size() == wsnParams.nodeAndParentIds.size())
		wsnFlowTimer->setInterval(1);
}

uint16_t MainController::getTimeToSleep()
{
	QTime now = QTime::currentTime();
	int elapsed = now.minute()  % 30 * 60 + now.second();

	// Add one minute sleep time to prevent node clock drifting error
	return (60 * 30 - elapsed + 60);
}

void MainController::wakeNetwork()
{
	// The nodes the gateway is awake
	baseNode->sendPacket(PACKET_IS_AWAKE);

	// Wait for reroute command
	if (!wsnFlowTimer->isActive())
	{
		log("Nodes are awake, checking network status...");
		step = WSN_STEP_NOT_COLLECTED;
		wsnFlowTimer->start(5000);
	}
}

void MainController::sendPathSmss()
{
	// Calculate how many SMSs are needed
	int nodeCount = wsnParams.nodeAndParentIds.size();
	int smsCount = nodeCount / 10;
	if (nodeCount % 10 != 0)
		smsCount++;
	QString l;
	l.sprintf("Will send %d SMS(s) for %d node(s)", smsCount, nodeCount);
	log(l);

	// Format and send SMSs
	QList<int> nodeIds = wsnParams.nodeAndParentIds.keys();
	qSort(nodeIds.begin(), nodeIds.end());
	for (int i = 0; i < smsCount; i++)
	{
		// Construct header
		QString sms("s02;v2;");
		sms.append(QString::number(smsCount) + ';');
		sms.append(QString::number(i + 1) + ';');
		sms.append(QString::number(preferences->gatewayId()) + ';');

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

void MainController::sendDataSmss()
{
	// Calculate how many SMSs are needed
	int nodeCount = wsnParams.dataOfNodeIds.size();
	int smsCount = nodeCount / 5;
	if (nodeCount % 5 != 0)
		smsCount++;
	QString l;
	l.sprintf("Will send %d SMS(s) for %d node(s)", smsCount, nodeCount);
	log(l);

	// Format and send SMSs
	QList<int> nodeIds = wsnParams.dataOfNodeIds.keys();
	qSort(nodeIds.begin(), nodeIds.end());
	QDateTime now = QDateTime::currentDateTime();
	QString timeString = now.toString("yyyy,MM,dd,hh,mm,ss;");
	for (int i = 0; i < smsCount; i++)
	{
		// Construct header
		QString sms("s51;");
		sms.append(timeString);
		sms.append(QString::number(preferences->gatewayId()) + ';');

		// Construct node list
		for (int i = 0; i < 10; i++)
		{
			if (nodeCount == 0)
				break;
			int nodeId = nodeIds[nodeCount - 1];
			NodeData thisData = wsnParams.dataOfNodeIds[nodeId];
			sms.append(QString('n') +
					   QString::number(nodeId) +
					   ',' +
					   QString::number(thisData.temperature) +
					   ',' +
					   QString::number(thisData.humidity) +
					   ',' +
					   QString::number(thisData.pest) +
					   ',' +
					   QString::number(thisData.par) +
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

	// Write to log file
	QString name = QDir::homePath() +
				   "/wsn/gateway/log" +
				   QDate::currentDate().toString("yyyy-MM-dd");
	if (!logFile || logName != name)
	{
		logName = name;

		if (logFile) delete logFile;
		logFile = new QFile(name, this);
		logFile->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append);
	}

	QTextStream logger(logFile);
	if (inOwnLine)
		logger << '\n' << text << '\n';
	else
		logger << text;

}

void MainController::clearLog()
{
	window->statusTab()->clearLog();
}


Preferences::Preferences()
{
	// Read the pref file
	pref = new QSettings(QDir::homePath() + "/wsn/gateway/preferences",
						 QSettings::IniFormat);

	// Check for preference values, if not valid then re-generate
	// Notice that since we want to read from the pref file, we directly
	//  access the member variables here instead of using setters, which
	//  sync the pref file also

	// First, nodePort
	mNodePort = pref->value("serials/nodePort").toString();
	if (mNodePort == "")
		setNodePort(QString::fromAscii("/dev/ttyUSB0"));

	// Then, gsmPort
	mGsmPort = pref->value("serials/gsmPort").toString();
	if (mGsmPort == "")
		setGsmPort(QString::fromAscii("/dev/ttyUSB1"));

	// Gateway ID
	mGatewayId = pref->value("gateway/gatewayId").toInt();

	// Server Phone
	mServerPhone = pref->value("gateway/serverPhone").toString();

	// Finally, check if the network has already been deployed
	// If the setting is not set, the value would be false, so we don't need
	//  to check to generate it
	mIsDeployed = pref->value("wsn/isDeployed").toBool();

	// If the network is deployed, load the deploy params in
	if (mIsDeployed)
	{/*
		mMaxTier = pref->value("wsn/maxTier").toInt();
		mRootNodeIds = pref->value("wsn/rootNodeIds").toList().toSet();
		QList<int> nodes = pref->value("wsn/nodeIds").toList();
		mNodeAndParentIds =*/
	}
}

Preferences::~Preferences()
{
	delete pref;
}

void Preferences::setNodePort(QString p)
{
	mNodePort = p;
	pref->setValue("serials/nodePort", QVariant(mNodePort));
}

void Preferences::setGsmPort(QString p)
{
	mGsmPort = p;
	pref->setValue("serials/gsmPort", QVariant(mGsmPort));
}
void Preferences::setServerPhone(QString p)
{
	mServerPhone = p;
	pref->setValue("gateway/serverPhone", QVariant(mServerPhone));
}

void Preferences::setIsDeployed(bool d)
{
	mIsDeployed = d;
	pref->setValue("wsn/isDeployed", QVariant(mIsDeployed));
}

void Preferences::setGatewayId(int id)
{
	mGatewayId = id;
	pref->setValue("gateway/gatewayId", QVariant(mGatewayId));
}

QString Preferences::nodePort() const
{
	return mNodePort;
}

QString Preferences::gsmPort() const
{
	return mGsmPort;
}

QString Preferences::serverPhone() const
{
	return mServerPhone;
}

void Preferences::saveDeployParams(WsnParams p)
{
	pref->setValue("wsn/maxTier", QVariant(p.maxTier));
	pref->setValue("wsn/rootNodeIds",
				   QVariant::fromValue< QList<int> >(p.rootNodeIds.toList()));
	pref->setValue("wsn/nodeIds",
				   QVariant::fromValue< QList<int> >(p.nodeAndParentIds.keys()));
	pref->setValue("wsn/parentIds",
				   QVariant::fromValue< QList<int> >(p.nodeAndParentIds.values()));
}

void Preferences::loadDeployParamsInto(WsnParams *p)
{
	p->maxTier = pref->value("wsn/maxTier").toInt();
	p->rootNodeIds = pref->value("wsn/rootNodeIds").value< QList<int> >().toSet();
	QList<int> ns = pref->value("wsn/nodeIds").value< QList<int> >();
	QList<int> ps = pref->value("wsn/parentIds").value< QList<int> >();
	for (int i = 0; i < ns.size(); i++)
		p->nodeAndParentIds.insert(ns[i], ps[i]);
}
