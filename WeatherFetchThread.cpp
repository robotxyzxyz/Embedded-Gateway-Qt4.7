#include <QSettings>
#include <QFile>
#include <cstdlib>
#include "WeatherFetchThread.h"

WeatherFetchThread::WeatherFetchThread(QString prefix, QString logName, QString configName)
{
	mDatum = new Weather::Datum;
	mPrefix = prefix;
	mLog = logName;
	mConfig = configName;
}

WeatherFetchThread::~WeatherFetchThread()
{
	delete mDatum;
}

void WeatherFetchThread::run()
{
	QString cmd = mPrefix + "/conout2300 " + mLog + " " + mConfig;
	if (system(cmd.toAscii().data()))
		emit fetchError(Get_Con_File_Error);
	if (readInData())
		return;
	cmd = mPrefix + "/minmax2300 rtotal " + mConfig;	// Reset rain total
	if (system(cmd.toAscii().data()))
		emit fetchError(Reset_Rain_Error);
}

int WeatherFetchThread::readInData()
{
	// Get the setting file
	QSettings *log = new QSettings(mLog, QSettings::IniFormat);

	if (!log)
		return -1;

	mDatum->temperature = log->value("temperature/outdoor").toDouble();
	mDatum->humidity = log->value("humidity/outdoor").toInt();
	mDatum->dewPoint = log->value("misc/dewpoint").toDouble();
	mDatum->windChill = log->value("misc/windchill").toDouble();
	mDatum->windSpeed = log->value("wind/speed").toDouble();
	mDatum->windDirStr = log->value("wind/dirStr").toString();
	mDatum->windDirInt = log->value("wind/dirInt").toInt();
	mDatum->rain = log->value("rain/total").toDouble();
	mDatum->pressure = log->value("pressure/absolute").toDouble();

	delete log;

	return 0;
}
