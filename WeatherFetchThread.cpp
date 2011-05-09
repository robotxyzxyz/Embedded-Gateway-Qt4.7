#include <QDomDocument>
#include <QFile>
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
	QString cmd = mPrefix + "/xml2300 " + mLog + " " + mConfig;
	system(cmd.toAscii().data());
	if (parseXml())
		return;
	cmd = mPrefix + "/minmax2300 rtotal " + mConfig;	// Reset rain total
	system(cmd.toAscii().data());
}

int WeatherFetchThread::parseXml()
{
	// Get the DOM document
	QDomDocument dom;
	QFile xml(mLog);
	QString errorStr;
	int errorLine;
	int errorCol;
	if (!xml.open(QIODevice::ReadOnly))
	{
		return -1;
	}
	if (!dom.setContent(&xml, true, &errorStr, &errorLine, &errorCol))
	{
		xml.close();
		return -2;
	}
	xml.close();

	// XML order: temperature, humidity, dew point, wind, wind chill, rain, pressure
	QDomElement root = dom.documentElement();
	QDomNode n = root.firstChild().nextSibling();

	// Temperature -> Outdoor -> Value
	n = n.nextSibling();
	mDatum->temperature = n.firstChild().nextSibling().firstChildElement().text();

	// Humidity -> Outdoor -> Value
	n = n.nextSibling();
	mDatum->humidity = n.firstChild().nextSibling().firstChildElement().text();

	// Dewpoint -> Value
	n = n.nextSibling();
	mDatum->dewPoint = n.firstChildElement().text();

	// Wind -> Value
	//      -> Direction -> Text
	n = n.nextSibling();
	QDomNode v = n.firstChild();
	mDatum->windSpeed = v.toElement().text();
	mDatum->windDirStr = v.nextSibling().firstChildElement().text();

	// Windchill -> Value
	n = n.nextSibling();
	mDatum->windChill = n.firstChildElement().text();

	// Rain -> Total -> Value
	n = n.nextSibling();
	mDatum->rain = n.firstChild().nextSibling().nextSibling().firstChildElement().text();

	// Pressure -> Value
	n = n.nextSibling();
	mDatum->pressure = n.firstChildElement().text();

	return 0;
}
