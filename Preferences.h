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

	// Setters
	void setNodePort(QString p);
	void setGsmPort(QString p);
	void setWeatherPort(QString p);
	void setServerPhone(QString p);
	void setIsDeployed(bool d);
	void setGatewayId(int id);

	// Getters
	inline QString nodePort() const
	{
		return mNodePort;
	}
	inline QString gsmPort() const
	{
		return mGsmPort;
	}
	inline QString weatherPort() const
	{
		return mWeatherPort;
	}
	inline QString serverPhone() const
	{
		return mServerPhone;
	}
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

private:
	QString fileLoc;
	QSettings *pref;
	QString mNodePort;
	QString mGsmPort;
	QString mWeatherPort;
	QString mServerPhone;
	bool mIsDeployed;
	int mGatewayId;
	int mMaxTier;
	QSet<int> mRootNodeIds;
	QHash<int, int> mNodeAndParentIds;
};


#endif // PREFERENCES_H
