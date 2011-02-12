#ifndef MAINCONTROLLER_H
#define MAINCONTROLLER_H

#include <QObject>
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
	int maxLevel;
	QList<unsigned int> nodeIds;
}

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
	Preferences preferences;
	int step;
};

#endif // MAINCONTROLLER_H
