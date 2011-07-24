#ifndef ABSTRACTSERIALDEVICE_H
#define ABSTRACTSERIALDEVICE_H

#include <QObject>
#include <stdint.h>		    // For uintXX_t
#include <termios.h>		// For termios struct and options
class TimerSocketNotifier;

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

	static const int Serial_Open_Error = -1;	// Inability to open port
	static const int Serial_Write_Error = -2;	// Inability to write
	static const int Serial_Read_Error = -3;	// Inability to read

signals:
	void occuredError(const int errorCode);

protected:
	int initSerial();

	QString serialPath;
	TimerSocketNotifier *notifier;
	speed_t baud;

protected slots:
    void readData();
	virtual void readByte(int fd) = 0;
};

#endif // ABSTRACTSERIALDEVICE_H
