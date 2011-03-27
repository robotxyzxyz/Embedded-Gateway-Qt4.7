#ifndef PREFERENCES_H
#define PREFERENCES_H

#include <QHash>
#include <QSet>
#include <QString>
#include "FMacPacketParser.h"
class QSettings;

struct WsnParams
{
	bool hasRootNodes;
	int maxTier;
	QSet<int> rootNodeIds;
	QHash<int, int> nodeAndParentIds;
	QHash<int, NodeData> dataOfNodeIds;
};

namespace PendingTask
{
	enum PendingTask
	{
		Idle,
		Should_Collect,
		Should_Deploy,
		Should_Reboot
	};
}

class Preferences
{
public:
	explicit Preferences();
	virtual ~Preferences();

	void saveDeployParams(WsnParams p);
	WsnParams loadDeployParams();

	// Setters
	void setNodePort(QString p);
	void setGsmPort(QString p);
	void setServerPhone(QString p);
	void setPendingTask(int p);
	void setGatewayId(int id);

	// Getters
	QString nodePort() const;
	QString gsmPort() const;
	QString serverPhone() const;
	inline int pendingTask() const
	{
		return mPendingTask;
	}
	inline int gatewayId() const
	{
		return mGatewayId;
	}
	inline QString settingFilePath() const
	{
		return fileLoc;
	}

private:
	QString fileLoc;
	QSettings *pref;
	QString mNodePort;
	QString mGsmPort;
	QString mServerPhone;
	int mPendingTask;
	int mGatewayId;
	int mMaxTier;
	QSet<int> mRootNodeIds;
	QHash<int, int> mNodeAndParentIds;
};


#endif // PREFERENCES_H
