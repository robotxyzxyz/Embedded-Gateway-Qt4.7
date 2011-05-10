#ifndef WEATHERFETCHTHREAD_H
#define WEATHERFETCHTHREAD_H

#include <QThread>

namespace Weather
{
	class Datum
	{
	public:
		double temperature;
		int humidity;
		double dewPoint;
		double windChill;
		double rain;
		double pressure;
		double windSpeed;
		int windDirInt;
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
	int readInData();
	QString mPrefix;
	QString mLog;
	QString mConfig;
	Weather::Datum *mDatum;
};

#endif // WEATHERFETCHTHREAD_H
