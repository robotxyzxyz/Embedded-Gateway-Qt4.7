#ifndef BASENODE_H
#define BASENODE_H

#include "AbstractSerialDevice.h"
#include <QList>
class QSocketNotifier;
class QTimer;

class BaseNode : public AbstractSerialDevice
{
	Q_OBJECT

public:
	explicit BaseNode(QString path, QObject *parent = 0);

	static const int Packet_Form_Error = 0;	// Malformed packet
	static const int Packet_Crc_Error = 1;	// Incorrect CRC

signals:
	void receivedPacket(QList<uint8_t> packet);

public slots:
	bool sendPacket(QList<uint8_t> packet);

private slots:
	void readData(int fd);
        void readTimerFired();

private:
	void initMembers();
	bool isCrcCorrect(QList<uint8_t> packet);
	uint16_t getCrcOfPacket(QList<uint8_t> packet);
	uint16_t calcCrcByte(uint16_t crc, uint8_t b);

        QTimer *readTimer;

	QList<uint8_t> bufferIn;
	bool shouldReceive;
	bool is7DBreaking;
};

#endif // BASENODE_H
