#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H

#include <QObject>
#include <QHash>
#include <QSet>
#include <stdint.h>
#include "FMacPacketParser.h"
class QSettings;
class QStringList;
class QTimer;
class BaseNode;
class GsmModuleController;
class Window;

struct Preferences
{
	QString nodePort;
	QString gsmPort;
	bool deployed;
};

struct WsnParams
{
	bool hasRootNodes;
	int maxTier;
	QSet<int> rootNodeIds;
	QHash<int, int> nodeAndParentIds;
	QHash<int, NodeData> dataOfNodeIds;
};

enum WsnSteps
{
	WSN_STEP_NOT_DEPLOYED					= -2,
	WSN_STEP_DEPLOY_START					= -1,
	WSN_STEP_DEPLOY_RESET					=  0,
	WSN_STEP_DEPLOY_REQUEST_PATH			= 11,
	WSN_STEP_DEPLOY_DISTRIBUTE_TIME_SLOTS	,
	WSN_STEP_DEPLOY_FINISH					,
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
	void wsnFlowFired();
	void setMaxTier(int tier);
	void addFirstTierNode();
	void addPath(int nodeId, int parentId, bool isRelayed);
	void addData(NodeData data, bool isSupplemental);
	void wakeNetwork();

private:
	void getPreferences();
	void initMembers();
	void sendPathSmss();
	void log(QString text, bool inOwnLine = true);
	void clearLog();

	Window *window;
	BaseNode *baseNode;
	GsmModuleController *gsmControl;
	QTimer *wsnFlowTimer;
	WsnParams wsnParams;
	FMacPacketParser packParser;
	Preferences preferences;
	int step;
	bool stepSatisfied;
};

#endif // MAINCONTROLLER_H
