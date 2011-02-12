#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H

#include <QObject>
#include <QHash>
#include <stdint.h>
class QSettings;
class QTimer;
class BaseNode;
class Window;

struct Preferences
{
	QString nodePort;
	bool deployed;
};

struct NodeData
{
	uint8_t senderNodeId;
	uint8_t dataSourceNodeId;
	uint8_t temperature;
	uint8_t humidity;
	uint8_t pest;
	uint8_t par;
};

struct WsnParams
{
	int maxLevel;
	QList<unsigned int> nodeIds;
	QHash<QString, NodeData> dataOfNodeIds;
};

enum WsnSteps
{
	wsnNotDeployed = -1,
	wsnDeployWillStart = 0,
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

private:
	void getPreferences();
	void initMembers();
	void log(QString text, bool inOwnLine = true);
	void clearLog();

	Window *window;
	BaseNode *baseNode;
	QTimer *wsnFlowTimer;
	WsnParams wsnParams;
	Preferences preferences;
	int step;
	bool stepSatisfied;
};

#endif // MAINCONTROLLER_H
