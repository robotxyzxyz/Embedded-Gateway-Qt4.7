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
	void setServerPhone(QString p);
	void setIsDeployed(bool d);
	void setGatewayId(int id);

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

private:
	QString fileLoc;
	QSettings *pref;
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
