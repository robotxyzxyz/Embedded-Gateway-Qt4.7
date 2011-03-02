#include "WeatherModule.h"

WeatherModule::WeatherModule(QString path, QObject *parent) : AbstractSerialDevice(path, parent)
{
}

void WeatherModule::initMembers()
{
	currentType = Weather::Type::None;
}

void WeatherModule::fetchAllData()
{
}
