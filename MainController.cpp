#include "MainController.h"
#include <QDateTime>
#include <QDir>
#include <QSettings>
#include <QTextStream>
#include <QTimer>
#include <QtAlgorithms>
#include "BaseNode.h"
#include "GlobalErrorCodes.h"
#include "GsmModuleController.h"
#include "MainView.h"
#include "Packets.h"
#include "PacketSlots.h"
#include "StatusView.h"
#include "Window.h"
#include <QDebug>

MainController::MainController(QObject *parent) : QObject(parent)
{
	preferences = new Preferences();
	initMembers();
	startGsmCsqUpdateDaemon();

	switch (preferences->pendingTask())
	{
	case PendingTask::Idle:
		if (!loadNetworkParams())
			QTimer::singleShot(1000, this, SLOT(deployNetwork()));
		break;

	case PendingTask::Should_Deploy:
		QTimer::singleShot(1000, this, SLOT(deployNetwork()));
		break;

	case PendingTask::Should_Collect:
		if (loadNetworkParams())
			QTimer::singleShot(1000, this, SLOT(collectData()));
		else
			QTimer::singleShot(1000, this, SLOT(deployNetwork()));
		break;

	default:
		break;
	}
}

MainController::~MainController()
{
	delete preferences;
}

void MainController::reboot()
{
	delete preferences;
	log("Will try to reboot...");
	if (system("reboot") != 0)
		emit occuredError(GlobalErrors::Reboot_Error);
}

void MainController::timerEvent(QTimerEvent *)
{
	gsmControl->sendCarrierSignalQualityCommand();
}

void MainController::initializeBaseNodeAndGsmModule()
{
	baseNode = new BaseNode(preferences->nodePort(), this);
	gsmControl
		= new GsmModuleController(preferences->gsmPort(),
								  preferences->serverPhone(),
								  this);

	// Connect packet receiving to parsing
	connect(baseNode, SIGNAL(receivedPacket(QList<uint8_t>)),
			&packParser, SLOT(processPacket(QList<uint8_t>)));

	// Connect GSM signal strength receiving
	connect(gsmControl, SIGNAL(receivedCarrierSignalQuality(int)),
			window->mainTab(), SLOT(setGsmSignalQuality(int)));
}

void MainController::initMembers()
{
	window = new Window(preferences);
	logFile = NULL;

	initializeBaseNodeAndGsmModule();

	wsnFlowTimer = new QTimer(this);
	wsnFlowTimer->setSingleShot(true);
	connect(wsnFlowTimer, SIGNAL(timeout()), this, SLOT(wsnFlowFired()));

	// sleepCheckTimer is a 35-minute timer to check whether the network collects
	//  data periodically
	// If the network doesn't do anything for 45 minutes, something is wrong
	// This timer is stopped when collectData() or deployNetwork() is called
	sleepCheckTimer = new QTimer(this);
	sleepCheckTimer->setSingleShot(true);
	connect(sleepCheckTimer, SIGNAL(timeout()), this, SLOT(deployNetwork()));
	sleepCheckTimer->start(30 * 60 * 1000 / 10 + 30 * 60 * 1000);

	timesDeployFailed = 0;

	// Connect packet parser events
	connect(&packParser, SIGNAL(gotMaxTier(int)), this, SLOT(setMaxTier(int)));
	connect(&packParser, SIGNAL(gotNodePath(int, int, bool)),
			this, SLOT(addPath(int, int, bool)));
	connect(&packParser, SIGNAL(gotData(NodeData, bool)),
			this, SLOT(addData(NodeData, bool)));
	connect(&packParser, SIGNAL(hadAwakeNode()), this, SLOT(wakeNetwork()));
	connect(&packParser, SIGNAL(shouldReroute()), this, SLOT(deployNetwork()));

	// Connect UI events
	connect(window->statusTab(), SIGNAL(clearLogTriggered()),
			this, SLOT(clearLog()));
	connect(window->statusTab(), SIGNAL(deployTriggered()),
			this, SLOT(deployNetwork()));
	connect(window->statusTab(), SIGNAL(collectTriggered()),
			this, SLOT(collectData()));
}

void MainController::startGsmCsqUpdateDaemon()
{
	// Update GSM signal quality every 10 minutes
	QObject::startTimer(10 * 60 * 1000);
}

void MainController::deployNetwork()
{
	if (preferences->pendingTask() == PendingTask::Should_Reboot)
	{
		preferences->setPendingTask(PendingTask::Should_Deploy);
		QTimer::singleShot(3 * 1000, this, SLOT(reboot()));
		return;
	}

	preferences->setPendingTask(PendingTask::Should_Deploy);
	sleepCheckTimer->stop();
	wsnFlowTimer->stop();
	clearLog();
	step = WsnSteps::Not_Deployed;

	if (timesDeployFailed >= Deploy_Failure_Reboot_Threshold)
		reboot();
	else
		timesDeployFailed++;

	stepSatisfied = false;

	wsnParams.hasRootNodes = false;
	wsnParams.maxTier = 0;
	wsnParams.nodeAndParentIds.clear();
	wsnParams.dataOfNodeIds.clear();

	wsnFlowTimer->start(1);
}

void MainController::stepDeploySendNextStartCommand()
{
	// Print the one-time Phrase start
	// Flag used here is reset in the next step (Deploy_Request_Path)
	if (!stepSatisfied)
	{
		log("\nSending deploy command: ", false);
		stepSatisfied = true;
	}

	// Print count of deploy command
	log(QString::number(step - WsnSteps::Deploy_Reset) + " ", false);

	// Send the command
	baseNode->sendPacket(Packets::Start_Deploy);
}

void MainController::stepDeployRequestPath()
{
	// Reset flag used in previous step
	stepSatisfied = false;

	log("Requesting for path");
	baseNode->sendPacket(Packets::Request_Path);
}


void MainController::stepDeployDitributeTimeSlots()
{
	// Ask nodes to distribute time slots, thus finish deploying
	baseNode->sendPacket(Packets::Distribute_Time_Slots);
	QString s;
	s.sprintf("Deployment finished for %d nodes",
			  wsnParams.nodeAndParentIds.size());
	log(s);
}


void MainController::stepDeployFinishing()
{
	// Return the deployment info via SMS
	log("Returning path info...");
	sendPathSmss();

	// Save deploy params, then wait 10 seconds before collecting data
	preferences->setPendingTask(PendingTask::Should_Collect);
	preferences->saveDeployParams(wsnParams);
	window->mainTab()->setDeployedNodes(wsnParams.nodeAndParentIds.keys());

	timesDeployFailed = 0;
}

void MainController::stepCollectStart()
{
	log("Ordering first-tier nodes to collect data");
	wsnParams.dataOfNodeIds.clear();
	baseNode->sendPacket(Packets::Collect_Start);
}

void MainController::stepCollectSendNextRequest()
{
	int thisId = rootNodeIdsToCollect.takeFirst();
	log("Sending request to node " + QString::number(thisId));
	QList<uint8_t> req = Packets::Collect_Request;
	req[GatewayPacket::Receiver] = (uint8_t)thisId;
	baseNode->sendPacket(req);
}

bool MainController::isCollectSuccessful()
{
	if (wsnParams.dataOfNodeIds.isEmpty())
	{
		wsnFlowTimer->stop();
		log("No collectable nodes, will reroute in 3 seconds...");
		QTimer::singleShot(3000, this, SLOT(deployNetwork()));
		return false;
	}
	return true;
}

void MainController::stepCollectSynchronize()
{
	log("Sending synchronization command");
	baseNode->sendPacket(Packets::Synchronize);
	wsnFlowTimer->start(5000);
}

void MainController::stepSupplementalCollectAndSleep()
{
	QList<uint8_t> slp = Packets::Sleep;
	uint16_t timeToSleep = getTimeToSleep();
	slp[GatewayPacket::Sleep_Time_Hi] = timeToSleep / 0x100;
	slp[GatewayPacket::Sleep_Time_Lo] = timeToSleep % 0x100;
	baseNode->sendPacket(slp);
	log("Nodes will sleep for " +
		QString::number(timeToSleep) +
		" seconds after supplemental collection");

	// Start a timer to check if the network has collected data
	// If the network doesn't collect for this time interval, something's wrong
	// The time is calculated as (collect interval) * 0.1 + (sleep time)
	// This timer is stopped when collectData() or deployNetwork() is called
	sleepCheckTimer->start((int)timeToSleep * 1000 + 30 * 60 * 1000 / 10);
}

void MainController::stepCollectFinish()
{
	wsnFlowTimer->stop();
	window->mainTab()->setCollectedNodes(wsnParams.dataOfNodeIds.keys());
	log("Supplemental collection finished, nodes should be asleep now");
	log("Data collected from " +
		QString::number(wsnParams.dataOfNodeIds.size()) +
		" node(s)");
	log("Will return data via GSM...");
	sendDataSmss();
	log("Data collection finished");
	preferences->setPendingTask(PendingTask::Idle);
}

void MainController::wsnFlowFired()
{
	step++;
	if (step == WsnSteps::Deploy_Start)
	{
		log(QString("Will deploy via ") + baseNode->path());
		wsnFlowTimer->start(1000);
	}
	else if (step == WsnSteps::Deploy_Reset)
	{
		log("Nodes will reset...");
		baseNode->sendPacket(Packets::Reset);
		wsnFlowTimer->start(5000);
	}
	else if (step < WsnSteps::Deploy_Request_Path)
	{
		stepDeploySendNextStartCommand();
		wsnFlowTimer->start(1000);
	}
	else if (step == WsnSteps::Deploy_Request_Path)
	{
		stepDeployRequestPath();
		wsnFlowTimer->start(10000);
		// Now wait for max tier packets and path return packets
		// The minimum wait time is 10s, and if no-one returns this path
		//  request, we re-deploy the network
		// See comments in setMaxTier() and addPath()
	}
	else if (step == WsnSteps::Deploy_Distribute_Time_Slots)
	{
		// First check for the path result
		// Reroute in 3 seconds if the path and tier settings are not complete
		if (!isNetworkCollectable())
		{
			QTimer::singleShot(3000, this, SLOT(deployNetwork()));
			log("Network deployment failed, will reroute in 3 seconds...");
		}
		else
		{
			stepDeployDitributeTimeSlots();
			wsnFlowTimer->start(100);
		}
	}
	else if (step == WsnSteps::Deploy_Finish)
	{
		stepDeployFinishing();
		log("Will collect in 10 seconds...");
		QTimer::singleShot(10000, this, SLOT(collectData()));
	}
	else if (step == WsnSteps::Collect_Start)
	{
		stepCollectStart();
		wsnFlowTimer->start(5000);
	}
	else if (step == WsnSteps::Collect_Request)
	{
		log("Requesting data from first-tier nodes");
		rootNodeIdsToCollect = wsnParams.rootNodeIds.toList();
		wsnFlowTimer->start(1);
	}
	else if (step == WsnSteps::Collect_Requesting)
	{
		stepCollectSendNextRequest();
		if (!rootNodeIdsToCollect.isEmpty())
			step--;
		wsnFlowTimer->start(5000);
	}
	else if (step == WsnSteps::Collect_Wait_To_Receive)
	{
		log("All requests sent, waiting for data...");
		wsnFlowTimer->start(10000);
	}
	else if (step == WsnSteps::Synchronize)
	{
		if (isCollectSuccessful())
			this->stepCollectSynchronize();
	}
	else if (step == WsnSteps::Supplemental_Collect_And_Sleep)
	{
		stepSupplementalCollectAndSleep();
		wsnFlowTimer->start(5000);
	}
	else if (step == WsnSteps::Collect_Finish)
	{
		stepCollectFinish();
	}
	else
	{
		wsnFlowTimer->start(1);
	}
}
void MainController::collectData()
{
	if (preferences->pendingTask() == PendingTask::Should_Reboot)
	{
		preferences->setPendingTask(PendingTask::Should_Collect);
		QTimer::singleShot(3 * 1000, this, SLOT(reboot()));
		return;
	}

	preferences->setPendingTask(PendingTask::Should_Collect);

	sleepCheckTimer->stop();
	wsnFlowTimer->stop();
	stepSatisfied = false;
	clearLog();

	if (!isNetworkCollectable())
	{
		preferences->setPendingTask(PendingTask::Should_Reboot);
		QTimer::singleShot(1, this, SLOT(deployNetwork()));
	}
	else
	{
		step = WsnSteps::Not_Collected;
		wsnFlowTimer->start(1);
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

void MainController::addPath(int nodeId, int parentId, bool isRelayed)
{
	// Check that there are first tier nodes
	if (!wsnParams.hasRootNodes)
	{
		wsnParams.hasRootNodes = true;
		log("Root node discovered");
	}

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
	if (	(!wsnParams.hasRootNodes) ||
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
		QList<uint8_t> ack = Packets::Collect_Acknowledgement;
		ack[GatewayPacket::Receiver] = data.senderNodeId;
		ack[GatewayPacket::Depth] = data.dataSourceTier;
		baseNode->sendPacket(ack);

		l = "Data collected from node ";
	}
	else
	{
		wsnFlowTimer->setInterval(5000);
		l = "Data supplementally collected from node ";
	}
	l.append(QString::number(data.dataSourceNodeId) + ':');
	log(l);

	wsnParams.dataOfNodeIds.insert(sourceId, data);
	l.sprintf("     Temp %.1f; Hmdy %.1f; Illm %d; Pest %d",
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
	int elapsed = now.minute() % 30 * 60 + now.second();

	return (60 * 30 - elapsed);
}

void MainController::wakeNetwork()
{
	if (step < WsnSteps::Deploy_Finish)
		return;

	// Tell the nodes the gateway is awake
	baseNode->sendPacket(Packets::Awake);

	sleepCheckTimer->stop();

	// Wait for reroute command
	if (!stepSatisfied)
	{
		log("Nodes are awake, checking network status...");
		stepSatisfied = true;
		step = WsnSteps::Not_Collected;
		preferences->setPendingTask(PendingTask::Should_Reboot);
		QTimer::singleShot(30 * 1000, this, SLOT(collectData()));
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
	l.sprintf("Sending %d SMS(s) for %d node(s)", smsCount, nodeCount);
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
	l.sprintf("Sending %d SMS(s) for %d node(s)", smsCount, nodeCount);
	log(l);

	// Format and send SMSs
	QList<int> nodeIds = wsnParams.dataOfNodeIds.keys();
	qSort(nodeIds.begin(), nodeIds.end());
	QDateTime now = QDateTime::currentDateTime();
	QString timeString = now.toString("yyyy,M,d,h,m,s;");
	for (int i = 0; i < smsCount; i++)
	{
		// Construct header
		QString sms("s51;");
		sms.append(timeString);
		sms.append(QString::number(preferences->gatewayId()) + ';');

		// Construct node list
		for (int i = 0; i < 5; i++)
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

bool MainController::loadNetworkParams()
{
	log("Detected existing configuration, will resume without deploying");
	wsnParams = preferences->loadDeployParams();
	if (!wsnParams.nodeAndParentIds.isEmpty())
	{
		window->mainTab()->setDeployedNodes(wsnParams.nodeAndParentIds.keys());
		step = WsnSteps::Collect_Finish;
		return true;
	}
	else
	{
		return false;
	}
}

void MainController::log(QString text, bool inOwnLine)
{
	QString tag = QTime::currentTime().toString("[hh:mm:ss]");
	window->statusTab()->log(text, tag, inOwnLine);

	// Write to log file
	QString name = QDir::homePath() +
				   "/wsn/gateway/" +
				   QDate::currentDate().toString("yyyy-MM-dd") +
				   ".log";
	if (!logFile || logName != name)
	{
		logName = name;
		if (logFile) delete logFile;
		logFile = new QFile(name, this);
		logFile->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append);
	}

	QTextStream logger(logFile);
	if (inOwnLine)
		logger << '\n' << tag << ' ';
	logger << text;
}

void MainController::clearLog()
{
	window->statusTab()->clearLog();
}
