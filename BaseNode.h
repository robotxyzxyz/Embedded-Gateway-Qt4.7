#ifndef BASENODE_H
#define BASENODE_H

#include <QObject>
#include <QList>
#include <stdint.h>		// For uintXX_t
class QSocketNotifier;

class BaseNode : public QObject
{
	Q_OBJECT

public:
	explicit BaseNode(QString path, QObject *parent = 0);
	~BaseNode();
	inline QString path() const
	{
		return serialPath;
	}

	static const int SERIAL_OPEN_ERROR = -1;// Inability to open serial port
	static const int SERIAL_WRITE_ERROR = 0;// Inability to write to serial
	static const int SERIAL_READ_ERROR = 1;	// Inability to read from serial
	static const int PACKET_FORM_ERROR = 2;	// Malformed packet
	static const int PACKET_CRC_ERROR = 3;	// Incorrect CRC

signals:
	void occuredError(const int errorCode);
	void receivedPacket(QList<uint8_t> packet);

public slots:
	bool sendPacket(QList<uint8_t> packet);

private slots:
	void readData(int fd);

private:
	int initSerial(QString *path);
	void initMembers();
	bool isCrcCorrect(QList<uint8_t> packet);
	uint16_t getCrcOfPacket(QList<uint8_t> packet);
	uint16_t calcCrcByte(uint16_t crc, uint8_t b);

	QString serialPath;
	QSocketNotifier *notifier;
	QList<uint8_t> bufferIn;
	bool shouldReceive;
	bool is7DBreaking;
};

#endif // BASENODE_H
