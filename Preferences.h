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

class Preferences
{
public:
	explicit Preferences();
	virtual ~Preferences();

	void saveDeployParams(WsnParams p);
	WsnParams loadDeployParams();

	static const int Collects_Per_Hour_Minimum = 1;
	static const int Collects_Per_Hour_Maximum = 30;
	static const int Collects_Per_Hour_Default = 2;

	// Setters
	void setNodePort(QString p);
	void setGsmPort(QString p);
	void setServerPhone(QString p);
	void setIsDeployed(bool d);
	void setGatewayId(int id);
	void setCollectsPerHour(int c);

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
	inline QString settingFilePath() const
	{
		return fileLoc;
	}
	inline int collectsPerHour() const
	{
		return mCollectsPerHour;
	}
	inline int sleepCheckTimerIntervalMilliseconds() const
	{
		return mSleepCheckTimerIntervalMilliseconds;
	}

private:
	QString fileLoc;
	QSettings *pref;
	int mCollectsPerHour;
	int mSleepCheckTimerIntervalMilliseconds;
	QString mNodePort;
	QString mGsmPort;
	QString mServerPhone;
	bool mIsDeployed;
	int mGatewayId;
	int mMaxTier;
	QSet<int> mRootNodeIds;
	QHash<int, int> mNodeAndParentIds;
};


#endif // PREFERENCES_H
