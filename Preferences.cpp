#include "Preferences.h"
#include <QDir>
#include <QSettings>

Preferences::Preferences()
{
	// Read the pref file
	fileLoc = QDir::homePath() + "/wsn/gateway/preferences";
	pref = new QSettings(fileLoc, QSettings::IniFormat);

	// Check for preference values, if not valid then re-generate
	// Notice that since we want to read from the pref file, we directly
	//  access the member variables here instead of using setters, which
	//  sync the pref file also

	// First, nodePort
	mNodePort = pref->value("serials/nodePort").toString();
	if (mNodePort == "")
		setNodePort(QString::fromAscii("/dev/ttyUSB0"));

	// Then, gsmPort
	mGsmPort = pref->value("serials/gsmPort").toString();
	if (mGsmPort == "")
		setGsmPort(QString::fromAscii("/dev/ttyUSB1"));

	// Gateway ID
	// If the setting is not set, the value would be 0
	// We check to generate it
	mGatewayId = pref->value("gateway/gatewayId").toInt();
	if (mGatewayId == 0)
		setGatewayId(0);

	// Server Phone
	mServerPhone = pref->value("gateway/serverPhone").toString();
	if (mServerPhone == "")
		setServerPhone(QString::fromAscii("0952650121"));

	// Pending Task
	mPendingTask = pref->value("wsn/pendingTask").toInt();
	if (mPendingTask == 0)
		setPendingTask(PendingTask::Idle);
}

Preferences::~Preferences()
{
	delete pref;
}

void Preferences::setNodePort(QString p)
{
	mNodePort = p;
	pref->setValue("serials/nodePort", QVariant(mNodePort));
}

void Preferences::setGsmPort(QString p)
{
	mGsmPort = p;
	pref->setValue("serials/gsmPort", QVariant(mGsmPort));
}
void Preferences::setServerPhone(QString p)
{
	mServerPhone = p;
	pref->setValue("gateway/serverPhone", QVariant(mServerPhone));
}

void Preferences::setPendingTask(int p)
{
	mPendingTask = p;
	pref->setValue("wsn/pendingTask", QVariant(mPendingTask));
}

void Preferences::setGatewayId(int id)
{
	mGatewayId = id;
	pref->setValue("gateway/gatewayId", QVariant(mGatewayId));
}

QString Preferences::nodePort() const
{
	return mNodePort;
}

QString Preferences::gsmPort() const
{
	return mGsmPort;
}

QString Preferences::serverPhone() const
{
	return mServerPhone;
}

void Preferences::saveDeployParams(WsnParams p)
{
	pref->setValue("wsn/maxTier", QVariant(p.maxTier));

	QStringList roots;
	for (int i = 0; i < p.rootNodeIds.size(); i++)
		roots.append(QString::number(p.rootNodeIds.toList()[i]));
	pref->setValue("wsn/rootNodeIds", QVariant(roots));

	QStringList nodes;
	for (int i = 0; i < p.nodeAndParentIds.keys().size(); i++)
		nodes.append(QString::number(p.nodeAndParentIds.keys()[i]));
	pref->setValue("wsn/nodeIds", QVariant(nodes));

	QStringList parents;
	for (int i = 0; i < p.nodeAndParentIds.values().size(); i++)
		parents.append(QString::number(p.nodeAndParentIds.values()[i]));
	pref->setValue("wsn/parentIds", QVariant(parents));
}

WsnParams Preferences::loadDeployParams()
{
	WsnParams p;
	p.maxTier = pref->value("wsn/maxTier").toInt();

	QStringList roots = pref->value("wsn/rootNodeIds").toStringList();
	QStringList nodes = pref->value("wsn/nodeIds").toStringList();
	QStringList parents = pref->value("wsn/parentIds").toStringList();

	for (int i = 0; i < roots.size(); i++)
		p.rootNodeIds.insert(roots[i].toInt());
	for (int i = 0; i < nodes.size(); i++)
		p.nodeAndParentIds.insert(nodes[i].toInt(), parents[i].toInt());

	if (p.rootNodeIds.size() > 0)
		p.hasRootNodes = true;

	return p;
}
