#include "SettingsView.h"
#include <QDateTime>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include "Preferences.h"

SettingsView::SettingsView(Preferences *p, QWidget *parent) : QWidget(parent)
{
	initMembers(p);
	layoutElements();
}

void SettingsView::initMembers(Preferences *p)
{
	pref = p;
	gatewayId = new QLineEdit(QString::number(pref->gatewayId()), this);
	serverPhone = new QLineEdit(pref->serverPhone(), this);
	nodePort = new QLineEdit(pref->nodePort(), this);
	gsmPort = new QLineEdit(pref->gsmPort(), this);
	systemTime = new QLineEdit(this);
	apply = new QPushButton("Apply", this);

	connect(apply, SIGNAL(clicked()), this, SLOT(onApplyClicked()));
}

void SettingsView::layoutElements()
{
	// Save file location
	QString fileLoc = pref->settingFilePath();
	QLabel *loc = new QLabel(QString("Settings loaded from: ") + fileLoc, this);

	// Gateway settings
	QVBoxLayout *gId = new QVBoxLayout();
	gId->addWidget(new QLabel("Gateway ID", this), 0);
	gId->addWidget(gatewayId, 0);

	QVBoxLayout *sPh = new QVBoxLayout();
	sPh->addWidget(new QLabel("Server phone", this), 0);
	sPh->addWidget(serverPhone, 0);

	QVBoxLayout *sTm = new QVBoxLayout();
	QString label("Set system time");
	label.append(" (yyyy-MM-dd HH:mm:ss, or leave blank to use current setting.");
	label.append(" Only works with superuser permission)");
	sTm->addWidget(new QLabel(label, this), 0);
	sTm->addWidget(systemTime, 0);

	QGridLayout *gateway = new QGridLayout();
	gateway->addLayout(gId, 0, 0);
	gateway->addLayout(sPh, 0, 1);
	gateway->addLayout(sTm, 1, 0, 1, -1);	// -1 spans system time to all columns

	QGroupBox *gatewayGroup = new QGroupBox("General gateway settings", this);
	gatewayGroup->setLayout(gateway);

	// Serial settings
	QVBoxLayout *nPt = new QVBoxLayout();
	nPt->addWidget(new QLabel("Base node", this), 0);
	nPt->addWidget(nodePort, 0);

	QVBoxLayout *gPt = new QVBoxLayout();
	gPt->addWidget(new QLabel("GSM module", this), 0);
	gPt->addWidget(gsmPort, 0);

	QGridLayout *serials = new QGridLayout();
	serials->addLayout(nPt, 0, 0);
	serials->addLayout(gPt, 0, 1);

	QGroupBox *serialGroup = new QGroupBox("Serial settings (needs restart)",
										 this);
	serialGroup->setLayout(serials);

	// Apply button
	QHBoxLayout *applyRow = new QHBoxLayout();
	applyRow->addStretch(1);
	applyRow->addWidget(apply, 0);

	// Layout group boxes and apply button
	QVBoxLayout *whole = new QVBoxLayout();
	whole->addWidget(loc, 0);
	whole->addWidget(gatewayGroup, 0);
	whole->addWidget(serialGroup, 0);
	whole->addStretch(1);
	whole->addLayout(applyRow, 0);
	setLayout(whole);
}

void SettingsView::onApplyClicked()
{
	bool suc;

	// Varify gateway number input before write to setting
	int gId = gatewayId->text().toInt(&suc);
	if (!suc || gId > 255)
		gatewayId->clear();
	else if (pref->gatewayId() != gId)
		pref->setGatewayId(gId);

	QString sPh = serverPhone->text();
	sPh.toInt(&suc);
	if (!suc)
		serverPhone->clear();
	else if (pref->serverPhone() != sPh)
		pref->setServerPhone(sPh);

	QString nPt = nodePort->text();
	if (!nPt.startsWith("/dev/tty"))
		nodePort->clear();
	else if (pref->nodePort() != nPt)
		pref->setNodePort(nPt);

	QString gPt = gsmPort->text();
	if (!gPt.startsWith("/dev/tty"))
		gsmPort->clear();
	else if (pref->gsmPort() != gPt)
		pref->setGsmPort(gPt);

	QString sTm = systemTime->text();
	if (!sTm.isEmpty())
	{
		QDateTime dt = QDateTime::fromString(sTm, "yyyy-MM-dd HH:mm:ss");
		if (dt.isValid())
		{
			QString dateString = dt.toString("MMddHHmmyyyy.ss");
			dateString.prepend("date ");
			system(dateString.toAscii().data());
		}
	}
}
