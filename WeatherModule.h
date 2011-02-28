#ifndef WEATHERMODULE_H
#define WEATHERMODULE_H

#include "AbstractSerialDevice.h"

namespace Weather
{
	namespace Type
	{
		enum Type
		{
			None,
			Indoor_Temperature,
			Indoor_Humidity,
			Outdoor_Temperature,
			Outdoor_Humidity,
			Dew_Point,
			Wind_Chill,
			Atomosphere_Pressure,
			Wind_Speed,
			Wind_Direction,
			Rain_Fall
		};
	}

	namespace WindDirection
	{
		enum WindDirection
		{
			N,
			NNE,
			NE,
			ENE,
			E,
			ESE,
			SE,
			SSE,
			S,
			SSW,
			SW,
			WSW,
			W,
			WNW,
			NW,
			NNW
		};
	}

	struct Datum
	{
		int type;
		int datum;
	};
}

class WeatherModule : public AbstractSerialDevice
{
	Q_OBJECT

public:
	explicit WeatherModule(QString path, QObject *parent = 0);

signals:
	void receivedDatum(Weather::Datum datum);

public slots:
	void fetchAllData();

private:
	void initMembers();
	int currentType;
};

#endif // WEATHERMODULE_H
