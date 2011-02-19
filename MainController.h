#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H

#include <QObject>
#include <QHash>
#include <QSet>
#include <stdint.h>
#include "FMacPacketParser.h"
#include "Preferences.h"
class QFile;
class QSettings;
class QStringList;
class QTimer;
class BaseNode;
class GsmModuleController;
class Window;

enum WsnSteps
{
	WSN_STEP_NOT_DEPLOYED						= -2,
	WSN_STEP_DEPLOY_START						= -1,
	WSN_STEP_DEPLOY_RESET						=  0,
	WSN_STEP_DEPLOY_REQUEST_PATH				= 11,
	WSN_STEP_DEPLOY_DISTRIBUTE_TIME_SLOTS		,
	WSN_STEP_DEPLOY_FINISH					,
	WSN_STEP_HAS_DEPLOYED						,
	WSN_STEP_NOT_COLLECTED					,
	WSN_STEP_COLLECT_START					,
	WSN_STEP_COLLECT_REQUEST					,
	WSN_STEP_COLLECT_REQUESTING				,
	WSN_STEP_COLLECT_WAIT_TO_RECEIVE			,
	WSN_STEP_SYNCHRONIZE						,
	WSN_STEP_SUPPLEMENTAL_COLLECT_AND_SLEEP	,
	WSN_STEP_COLLECT_FINISH					,
};


class MainController : public QObject
{
    Q_OBJECT

public:
    explicit MainController(QObject *parent = 0);
	virtual ~MainController();

	static const int DEPLOY_FAILURE_RESET_THRESHOLD = 3;

public slots:

signals:

protected:
	void timerEvent(QTimerEvent *e);

private slots:
	void deployNetwork();
	void collectData();
	void wsnFlowFired();
	void setMaxTier(int tier);
	void addFirstTierNode();
	void addPath(int nodeId, int parentId, bool isRelayed);
	void addData(NodeData data, bool isSupplemental);
	void wakeNetwork();
	void loadNetworkParams();
	void clearLog();

private:
	void initMembers();
	void startGsmCsqUpdateDaemon();
	void sendPathSmss();
	void sendDataSmss();
	bool isNetworkCollectable();
	uint16_t getTimeToSleep();
	void log(QString text, bool inOwnLine = true);

	Window *window;
	QFile *logFile;
	QString logName;
	QTimer *wsnFlowTimer;
	QTimer *sleepCheckTimer;
	BaseNode *baseNode;
	GsmModuleController *gsmControl;
	FMacPacketParser packParser;
	Preferences *preferences;
	WsnParams wsnParams;
	QList<int> rootNodeIdsToCollect;
	int step;
	bool stepSatisfied;
	int timesDeployFailed;
};

#endif // MAINCONTROLLER_H
