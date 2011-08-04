#include "BaseNode.h"
#include <sys/ioctl.h>
#include <QTimer>
#include <QSocketNotifier>

BaseNode::BaseNode(QString path, QObject *parent) : AbstractSerialDevice(path, parent)  //跟BASENODE溝通
{
	initMembers();

	if (initSerial() < 0)
	{
		shouldReceive = false;
		emit occuredError(Serial_Open_Error);
	}
	else
        {
                disconnect(notifier, SIGNAL(activated(int)), this, 0);
                readTimer = new QTimer(this);
                connect(readTimer, SIGNAL(timeout()), this, SLOT(readTimerFired()));
                readTimer->start(1000);
        }
}

void BaseNode::readTimerFired()
{
    int bytes;
    int fd = notifier->socket();
    ioctl(fd, FIONREAD, &bytes);
    while (bytes)
    {
        readData(fd);
        ioctl(fd, FIONREAD, &bytes);
    }
}

void BaseNode::initMembers()
{
	baud = B57600;
	shouldReceive = true;
	is7DBreaking = false;
}

bool BaseNode::sendPacket(QList<uint8_t> packet)
{
	int size = packet.size();
	uint16_t crc = getCrcOfPacket(packet);
	packet[size - 2] = crc % 0x100;
	packet[size - 1] = crc / 0x100;

	packet.prepend(0x7e);
	packet.append(0x7e);
        size += 2;/*加頭和尾0x7e的大小*/

	int fd = notifier->socket();
	for (int i = 0; i < size; i++)
	{
		if (write(fd, &(packet[i]), 1) != 1)
		{
			emit occuredError(Serial_Write_Error);
			return false;
		}
	}
	return true;
}

void BaseNode::readData(int fd)
{
	// Read the byte
	uint8_t byte;
	if (read(fd, &byte, 1) != 1)
	{
		shouldReceive = false;
		is7DBreaking = false;
		emit occuredError(Serial_Read_Error);
		return;
	}

	// Check the byte for packet manipulation
	if (byte == 0x7e)
	{
		// Packet should have a reasonable size to be valid
		// The size of 5 is quite arbitrary though, should check TinyOS docs
		if (bufferIn.size() > 5)
		{
			if (isCrcCorrect(bufferIn))
				emit receivedPacket(bufferIn);
			else
				emit occuredError(Packet_Crc_Error);
		}
		bufferIn.clear();
		shouldReceive = true;
	}
	else if (!shouldReceive)
		return;
	else if (is7DBreaking)
	{
		is7DBreaking = false;
		switch (byte)
		{
		case 0x5d:
			bufferIn.append(0x7d);
			break;
		case 0x5e:
			bufferIn.append(0x7e);
			break;
		default:
			shouldReceive = false;
			emit occuredError(Packet_Form_Error);
		}
	}
	else if (byte == 0x7d)
		is7DBreaking = true;
	else
		bufferIn.append(byte);
}

bool BaseNode::isCrcCorrect(QList<uint8_t> packet)
{
	// Get packet CRC
	int s = packet.size();
	uint16_t pCrc = packet[s - 1] * 0x100 + packet[s - 2];

	// Calculate packet CRC
	uint16_t crc = getCrcOfPacket(packet);

	return (crc == pCrc);
}

uint16_t BaseNode::getCrcOfPacket(QList<uint8_t> packet)
{
	uint16_t crc = 0;
	for (int i = 0; i < packet.size() - 2; i++)
	{
		crc = calcCrcByte(crc, packet[i]);
	}

	return crc;
}

// CRC byte-wise checking code from TinyOS devel forum post by Andreas Koepke
// The post is about TinyOS 2, but the CRC implementation in 1 is the same
// https://www.millennium.berkeley.edu/pipermail/tinyos-devel/2007-April/001646.html
uint16_t BaseNode::calcCrcByte(uint16_t crc, uint8_t b)
{
	crc = (uint8_t)(crc >> 8) | (crc << 8);
	crc ^= b;
	crc ^= (uint8_t)(crc & 0xff) >> 4;
	crc ^= crc << 12;
	crc ^= (crc & 0xff) << 5;
	return crc;
}
