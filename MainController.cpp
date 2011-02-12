#include "MainController.h"
#include <QDebug>
#include <QSettings>
#include <QTimer>
#include "BaseNode.h"
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
	preferences.deployed = pref.value("wsn/deployed").toBool();
	qDebug() << preferences.deployed;
}

void MainController::initMembers()
{
	baseNode = new BaseNode(preferences.nodePort, this);
	wsnFlowTimer = new QTimer(this);
	connect(wsnFlowTimer, SIGNAL(fired()), this, SLOT(wsnFlowFired()));
}

void MainController::deployNetwork()
{
	clearLog();
	step = wsnNotDeployed;
	wsnFlowTimer->start(1, true);
}

void MainController::wsnFlowFired()
{
	step++;

	switch (step)
	{
	case wsnDeployWillStart:
		QString m = QString("Will deploy via");
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
