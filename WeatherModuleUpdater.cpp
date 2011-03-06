#include "WeatherModuleUpdater.h"
#include <QTimer>
#include "WeatherModule.h"

WeatherModuleUpdater::WeatherModuleUpdater(QString path, QObject *parent) : QThread(parent)
{
	currentType = Weather::Type::Not_Asking;
	module = new WeatherModule(path, 0);
	module->moveToThread(this);
	module->setParent(this);
	connect(module, SIGNAL(receivedDatum(Weather::Datum)),
			this, SLOT(receivedDatum(Weather::Datum)));
}

void WeatherModuleUpdater::run()
{
	QTimer::singleShot(500, this, SLOT(fetchNextDatum()));
}

void WeatherModuleUpdater::fetchNextDatum()
{
	currentType++;
	if (currentType == Weather::Type::Not_Asking)
		currentType = Weather::Type::Indoor_Temperature;

	for (unsigned int i = 0; i < 3; i++)
	{
		msleep(100);
		if (!module->sendCommand(currentType))
			break;
	}

	QTimer::singleShot(500, this, SLOT(fetchNextDatum()));
}
