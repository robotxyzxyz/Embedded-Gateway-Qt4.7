#include "MainView.h"
#include <QDateTime>
#include <QHBoxLayout>
#include <QLCDNumber>
#include <QLineEdit>
#include <QLabel>
#include <QProgressBar>
#include <QVBoxLayout>

MainView::MainView(QWidget *parent) : QWidget(parent)
{
	initMembers();
	layoutElements();
	QObject::startTimer(1000);
}

void MainView::initMembers()
{
	systemTime = new QLabel(getSystemTimeString(), this);

	deployedNodeIds = new QLabel(this);
	collectedNodeIds = new QLabel(this);
	gsmSignalQuality = new QProgressBar(this);
	gsmSignalQuality->setTextVisible(true);
	gsmSignalQuality->setMinimum(-1);
	gsmSignalQuality->setMaximum(31);
}

void MainView::layoutElements()
{
	QHBoxLayout *sysTime = new QHBoxLayout();
	sysTime->addStretch(1);
	sysTime->addWidget(new QLabel("System time:", this), 0);
	sysTime->addWidget(systemTime, 0);

	QVBoxLayout *deployedNodes = new QVBoxLayout();
	deployedNodes->setSpacing(0);
	deployedNodes->addWidget(new QLabel("Deployed nodes:", this), 0);
	deployedNodes->addWidget(deployedNodeIds, 0);

	QVBoxLayout *collectedNodes = new QVBoxLayout();
	collectedNodes->setSpacing(0);
	collectedNodes->addWidget(new QLabel("Collected nodes:", this), 0);
	collectedNodes->addWidget(collectedNodeIds, 0);

	QVBoxLayout *gsmSignal = new QVBoxLayout();
	gsmSignal->setSpacing(0);
	gsmSignal->addWidget(new QLabel("GSM signal quality:", this), 0);
	gsmSignal->addWidget(gsmSignalQuality, 0);

	QVBoxLayout *whole = new QVBoxLayout();
	whole->addLayout(sysTime, 0);
	whole->addLayout(deployedNodes, 0);
	whole->addSpacing(10);
	whole->addLayout(collectedNodes, 0);
	whole->addSpacing(10);
	whole->addStretch(1);
	whole->addLayout(gsmSignal, 0);
	setLayout(whole);
}

void MainView::timerEvent(QTimerEvent *)
{
	systemTime->setText(getSystemTimeString());
}

void MainView::setDeployedNodes(QList<int> ns)
{
	QString nodes = "";
	if (!ns.isEmpty())
	{
		nodes = QString::number(ns[0]);
		for (int i = 1; i < ns.size(); i++)
		{
			nodes.append(", ");
			nodes.append(QString::number(ns[i]));
		}
	}
	deployedNodeIds->setText(nodes);
}

void MainView::setCollectedNodes(QList<int> ns)
{
	QString nodes = QString::number(ns[0]);
	for (int i = 1; i < ns.size(); i++)
	{
		nodes.append(", ");
		nodes.append(QString::number(ns[i]));
	}
	collectedNodeIds->setText(nodes);
}

void MainView::setGsmSignalQuality(int sq)
{
	if (sq < 0 || sq > 31)
		gsmSignalQuality->setValue(-1);
	else
		gsmSignalQuality->setValue(sq);
}

QString MainView::getSystemTimeString()
{
	return QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
}
