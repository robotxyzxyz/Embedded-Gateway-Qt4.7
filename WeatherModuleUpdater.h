#ifndef WEATHERMODULEUPDATER_H
#define WEATHERMODULEUPDATER_H

#include <QThread>
#include "Weather.h"
class WeatherModule;

class WeatherModuleUpdater : public QThread
{
    Q_OBJECT

public:
    explicit WeatherModuleUpdater(QString path, QObject *parent = 0);

signals:
	void receivedDatum(Weather::Datum datum);

protected:
	virtual void run();

private slots:
	void fetchNextDatum();

private:
	WeatherModule *module;
	int currentType;

};

#endif // WEATHERMODULEUPDATER_H
