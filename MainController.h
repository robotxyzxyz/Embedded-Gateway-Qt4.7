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

namespace WsnSteps
{
	enum WsnSteps
	{
		Not_Deployed						= -2,
		Deploy_Start						= -1,
		Deploy_Reset						=  0,
		Deploy_Request_Path				= 11,
		Deploy_Distribute_Time_Slots		,
		Deploy_Finish					,
		Not_Collected					,
		Collect_Start					,
		Collect_Request					,
		Collect_Requesting				,
		Collect_Wait_To_Receive			,
		Synchronize						,
		Supplemental_Collect_And_Sleep	,
		Collect_Finish					,
	};
}

class MainController : public QObject
{
    Q_OBJECT

public:
    explicit MainController(QObject *parent = 0);
	virtual ~MainController();

	static const int Deploy_Failure_Reboot_Threshold = 3;
	static const int Sleep_Check_Timer_Interval_Milliseconds = 35 * 60 * 1000;

public slots:

signals:
	void occuredError(int errorCode);

protected:
	void timerEvent(QTimerEvent *e);

private slots:
	void deployNetwork();
	void collectData();
	void wsnFlowFired();
	void setMaxTier(int tier);
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

	void stepDeploySendNextStartCommand();
	void stepDeployRequestPath();
	bool stepDeployDitributeTimeSlots();
	void stepDeployFinishing();
	void stepDeployDone();
	void stepCollectStart();
	void stepCollectSendNextRequest();
	void stepSupplementalCollectAndSleep();
	void stepCollectFinish();

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
