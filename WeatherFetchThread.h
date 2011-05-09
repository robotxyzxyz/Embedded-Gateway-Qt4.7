#ifndef WEATHERFETCHTHREAD_H
#define WEATHERFETCHTHREAD_H

#include <QThread>

namespace Weather
{
	class Datum
	{
	public:
		QString temperature;
		QString humidity;
		QString dewPoint;
		QString windChill;
		QString rain;
		QString pressure;
		QString windSpeed;
		QString windDirStr;
	};
}

class WeatherFetchThread : public QThread
{
	Q_OBJECT

public:
	WeatherFetchThread(QString prefix, QString logName, QString configName);
	~WeatherFetchThread();
	inline Weather::Datum *datum() const
	{
		return mDatum;
	}

protected:
	void run();

private:
	int parseXml();
	QString mPrefix;
	QString mLog;
	QString mConfig;
	Weather::Datum *mDatum;
};

#endif // WEATHERFETCHTHREAD_H
