#ifndef WEATHER_H
#define WEATHER_H

namespace Weather
{
	namespace Type
	{
		// The Not_Asking value is always set to the last of the enum,
		// thus promising this is the largest of all Types. This value
		// is used as the limit of walk-throughs, and the needed size
		// of weather data arrays
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
		int value;
	};
}


#endif // WEATHER_H
