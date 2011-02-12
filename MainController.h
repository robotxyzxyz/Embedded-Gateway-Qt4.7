#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H

#include <QObject>
#include <QHash>
#include <stdint.h>
#include "FMacPacketParser.h"
class QSettings;
class QTimer;
class BaseNode;
class Window;

struct Preferences
{
	QString nodePort;
	bool deployed;
};

struct WsnParams
{
	bool hasRootNodes;
	int maxTier;
	QSet<int> rootNodeIds;
	QHash<int, int> nodeAndParentIds;
	QHash<QString, NodeData> dataOfNodeIds;
};

enum WsnSteps
{
	WSN_NOT_DEPLOYED					= -2,
	WSN_DEPLOY_START					= -1,
	WSN_DEPLOY_RESET					=  0,
	WSN_DEPLOY_REQUEST_PATH				= 11,
	WSN_DEPLOY_CHECK_PATH				,
	WSN_DEPLOY_DISTRIBUTE_TIME_SLOTS	,
};

class MainController : public QObject
{
    Q_OBJECT

public:
    explicit MainController(QObject *parent = 0);
	~MainController();

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
	void log(QString text, bool inOwnLine = true);
	void clearLog();

	Window *window;
	BaseNode *baseNode;
	QTimer *wsnFlowTimer;
	WsnParams wsnParams;
	FMacPacketParser packParser;
	Preferences preferences;
	int step;
	bool stepSatisfied;
};

#endif // MAINCONTROLLER_H
