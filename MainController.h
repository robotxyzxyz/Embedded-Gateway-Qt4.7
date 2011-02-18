#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H

#include <QObject>
#include <QHash>
#include <QSet>
#include <stdint.h>
#include "FMacPacketParser.h"
class QFile;
class QSettings;
class QStringList;
class QTimer;
class BaseNode;
class GsmModuleController;
class Window;

struct WsnParams
{
	bool hasRootNodes;
	int maxTier;
	QSet<int> rootNodeIds;
	QList<int> rootNodeIdsToCollect;
	QHash<int, int> nodeAndParentIds;
	QHash<int, NodeData> dataOfNodeIds;
};

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

class Preferences
{
public:
	explicit Preferences();
	virtual ~Preferences();

	void saveDeployParams(WsnParams p);
	void loadDeployParamsInto(WsnParams *p);

	// Setters
	void setNodePort(QString p);
	void setGsmPort(QString p);
	void setServerPhone(QString p);
	void setIsDeployed(bool d);
	void setGatewayId(int id);

	// Getters
	QString nodePort() const;
	QString gsmPort() const;
	QString serverPhone() const;
	inline bool isDeployed() const
	{
		return mIsDeployed;
	}
	inline int gatewayId() const
	{
		return mGatewayId;
	}

private:
	QSettings *pref;
	QString mNodePort;
	QString mGsmPort;
	QString mServerPhone;
	bool mIsDeployed;
	int mGatewayId;
	int mMaxTier;
	QSet<int> mRootNodeIds;
	QHash<int, int> mNodeAndParentIds;
};

class MainController : public QObject
{
    Q_OBJECT

public:
    explicit MainController(QObject *parent = 0);
	virtual ~MainController();

public slots:

signals:

private slots:
	void deployNetwork();
	void collectData();
	void wsnFlowFired();
	void setMaxTier(int tier);
	void addFirstTierNode();
	void addPath(int nodeId, int parentId, bool isRelayed);
	void addData(NodeData data, bool isSupplemental);
	void wakeNetwork();
	void clearLog();

private:
	void initMembers();
	void sendPathSmss();
	void sendDataSmss();
	bool isNetworkCollectable();
	uint16_t getTimeToSleep();
	void log(QString text, bool inOwnLine = true);

	Window *window;
	QFile *logFile;
	QString logName;
	BaseNode *baseNode;
	GsmModuleController *gsmControl;
	QTimer *wsnFlowTimer;
	WsnParams wsnParams;
	FMacPacketParser packParser;
	Preferences *preferences;
	int step;
	bool stepSatisfied;
};

#endif // MAINCONTROLLER_H
