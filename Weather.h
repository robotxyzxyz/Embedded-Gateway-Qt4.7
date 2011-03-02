#ifndef WEATHER_H
#define WEATHER_H

namespace Weather
{
	namespace Type
	{
		enum Type
		{
			Indoor_Temperature = 0,
			Outdoor_Temperature,
			Wind_Chill,
			Dew_Point,
			Indoor_Humidity,
			Outdoor_Humidity,
			Wind_Speed,
			Wind_Direction,
			Atomosphere_Pressure,
			Rain_Fall,
			Rain_Fall_Reset,
			Not_Asking
		};
	}

	struct Datum
	{
		int type;
		int datum;
	};
}


#endif // WEATHER_H
