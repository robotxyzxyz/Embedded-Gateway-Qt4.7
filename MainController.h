#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H

#include <QObject>
#include <QHash>
#include <QSet>
#include <stdint.h>
#include "FMacPacketParser.h"
#include "Preferences.h"
<<<<<<< Updated upstream
class QFile;
class QSettings;
class QStringList;
class QTimer;
class BaseNode;
class GsmModuleController;
class WeatherFetchThread;
class Window;
=======
class QFile;                    //
class QSettings;                //
class QStringList;              //
class QTimer;                   //
class BaseNode;                 //Base node 類別
class GsmModuleController;      //GSM模組類別
class Window;                   //視窗類別

namespace WsnSteps              //WSN階段的名稱空間
{
    enum WsnSteps           //列舉WSN階段
	{
        Not_Deployed					= -2,  //未佈建
        Deploy_Start					= -1,  //開始佈建
        Deploy_Reset					=  0,  //重新佈建
        Deploy_Request_Path				= 11,  //要路徑   11
        Deploy_Distribute_Time_Slots	,      //分配時槽  12
        Deploy_Finish					,      //佈建完成  13
        Not_Collected					,      //未蒐集資料  14
        Collect_Start					,      //開始蒐集資料 15
        Collect_Request					,      //要資料命令  16
        Collect_Requesting				,      //          17
        Collect_Wait_To_Receive			,      //等待接收   18
        Synchronize						,      //同步       19
        Supplemental_Collect_And_Sleep	,      //睡眠       20
        Collect_Finish					,      //完成蒐集資料   21
        Read_Weather
	};
}

class MainController : public QObject
{
    Q_OBJECT

public:
    explicit MainController(QObject *parent = 0);
	virtual ~MainController();

	static const int Deploy_Failure_Reboot_Threshold = 1;

public slots:

signals:
	void occuredError(int errorCode);

protected:
	void timerEvent(QTimerEvent *e);

private slots:
    void deployNetwork();       //佈建網路
    void collectData();         //蒐集資料
	void wsnFlowFired();
    void setMaxTier(int tier);  //設定層數(傳入 int 層數)
    void addPath(int nodeId, int parentId, bool isRelayed);//加路徑(int 節點號碼, int 父節點號碼, bool )
	void addData(NodeData data, bool isSupplemental);
	void sendPathSmss();
	void sendDataSmss();
	void sendWeatherSms();
	void wakeNetwork();
	void clearLog();
	void reboot();

private:
        void initMembers();                   //初始成員
	void initializeBaseNodeAndGsmModule();
	bool loadNetworkParams();
	void startGsmCsqUpdateDaemon();
	bool isNetworkCollectable();
	uint16_t getTimeToSleep();
	bool isCollectSuccessful();
	void log(QString text, bool inOwnLine = true);

	void stepDeploySendNextStartCommand();
	void stepDeployRequestPath();
	void stepDeployDitributeTimeSlots();
	void stepDeployFinishing();
	void stepDeployDone();
	void stepCollectStart();
	void stepCollectSendNextRequest();
	void stepCollectSynchronize();
	void stepSupplementalCollectAndSleep();
	void stepCollectFinish();
	void stepReadWeather();

	Window *window;
	QFile *logFile;
	QString logName;
	WeatherFetchThread *t;
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
