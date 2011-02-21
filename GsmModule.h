#ifndef GSMMODULE_H
#define GSMMODULE_H

#include <QObject>
#include <stdint.h>		// For uintXX_t
class QSocketNotifier;

class GsmModule : public QObject
{
    Q_OBJECT

public:
	explicit GsmModule(QString path, QObject *parent = 0);
	virtual ~GsmModule();
	inline QString path() const
	{
		return serialPath;
	}

	static const int Serial_Open_Error = -1;// Inability to open serial port
	static const int Serial_Write_Error = 0;// Inability to write to serial
	static const int Serial_Read_Error = 1;	// Inability to read from serial

signals:
	void occuredError(const int errorCode);
	void receivedLine(QString line);

public slots:
	bool sendCommand(QString command);

private slots:
	void readData(int fd);

private:
	int initSerial(QString *path);
	void initMembers();

	QString serialPath;
	QSocketNotifier *notifier;
	QString bufferIn;

};

#endif // GSMMODULE_H
