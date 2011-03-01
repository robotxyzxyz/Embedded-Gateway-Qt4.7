#ifndef ABSTRACTSERIALDEVICE_H
#define ABSTRACTSERIALDEVICE_H

#include <QObject>
#include <stdint.h>		// For uintXX_t
#include <termios.h>		// For termios struct and options
class QSocketNotifier;

class AbstractSerialDevice : public QObject
{
    Q_OBJECT

public:
	explicit AbstractSerialDevice(QString path, QObject *parent = 0);
	virtual ~AbstractSerialDevice();
	inline QString path() const
	{
		return serialPath;
	}

	static const int Serial_Open_Error = -1;	// Inability to open serial port
	static const int Serial_Write_Error = -2;	// Inability to write to serial
	static const int Serial_Read_Error = -3;	// Inability to read from serial

signals:
	void occuredError(const int errorCode);

protected:
	int initSerial();

	QString serialPath;
	QSocketNotifier *notifier;
	speed_t baud;
	bool valid;

protected slots:
	virtual void readData(int fd) = 0;
};

#endif // ABSTRACTSERIALDEVICE_H
