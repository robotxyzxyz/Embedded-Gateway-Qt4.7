#ifndef GSMMODULE_H
#define GSMMODULE_H

#include "AbstractSerialDevice.h"

class GsmModule : public AbstractSerialDevice
{
    Q_OBJECT

public:
	explicit GsmModule(QString path, QObject *parent = 0);

signals:
	void receivedLine(QString line);

public slots:
	bool sendCommand(QString command);

private slots:
	void readData(int fd);

private:
	void initMembers();
	QString bufferIn;
};

#endif // GSMMODULE_H
