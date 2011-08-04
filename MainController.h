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
class BaseNode;                 //Base node ���O
class GsmModuleController;      //GSM�Ҳ����O
class Window;                   //�������O

namespace WsnSteps              //WSN���q���W�٪Ŷ�
{
    enum WsnSteps           //�C�|WSN���q
	{
        Not_Deployed					= -2,  //���G��
        Deploy_Start					= -1,  //�}�l�G��
        Deploy_Reset					=  0,  //���s�G��
        Deploy_Request_Path				= 11,  //�n���|   11
        Deploy_Distribute_Time_Slots	,      //���t�ɼ�  12
        Deploy_Finish					,      //�G�ا���  13
        Not_Collected					,      //���`�����  14
        Collect_Start					,      //�}�l�`����� 15
        Collect_Request					,      //�n��ƩR�O  16
        Collect_Requesting				,      //          17
        Collect_Wait_To_Receive			,      //���ݱ���   18
        Synchronize						,      //�P�B       19
        Supplemental_Collect_And_Sleep	,      //�ίv       20
        Collect_Finish					,      //�����`�����   21
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
    void deployNetwork();       //�G�غ���
    void collectData();         //�`�����
	void wsnFlowFired();
    void setMaxTier(int tier);  //�]�w�h��(�ǤJ int �h��)
    void addPath(int nodeId, int parentId, bool isRelayed);//�[���|(int �`�I���X, int ���`�I���X, bool )
	void addData(NodeData data, bool isSupplemental);
	void sendPathSmss();
	void sendDataSmss();
	void sendWeatherSms();
	void wakeNetwork();
	void clearLog();
	void reboot();

private:
        void initMembers();                   //��l����
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
