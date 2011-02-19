#ifndef MAINVIEW_H
#define MAINVIEW_H

#include <QWidget>
#include <QList>
class QLabel;
class QProgressBar;

class MainView : public QWidget
{
    Q_OBJECT

public:
    explicit MainView(QWidget *parent = 0);

public slots:
	void setDeployedNodes(QList<int> ns);
	void setCollectedNodes(QList<int> ns);
	void setGsmSignalQuality(int sq);			// 0-31, higher = better

signals:

protected:
	void timerEvent(QTimerEvent *e);

private:
	void initMembers();
	void layoutElements();
	QString getSystemTimeString();

	QLabel *systemTime;
	QLabel *deployedNodeIds;
	QLabel *collectedNodeIds;
	QProgressBar *gsmSignalQuality;
};

#endif // MAINVIEW_H
