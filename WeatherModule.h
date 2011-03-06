#ifndef WEATHERMODULE_H
#define WEATHERMODULE_H

#include "AbstractSerialDevice.h"
#include <QList>
#include "Weather.h"
class QThread;

class WeatherModule : public AbstractSerialDevice
{
	Q_OBJECT

public:
	explicit WeatherModule(QString path, QObject *parent = 0);
	int sendCommand(int type);

	static const int All_Ok = 0;
	static const int Command_Start_Error = 1;
	static const int Set_Address_Error = 2;
	static const int Set_Bytes_Error = 3;
	static const int No_Acknowledgement_Error = 4;
	static const int Wrong_Acknowledgement_Error = 5;

signals:
	void receivedDatum(Weather::Datum datum);
	void bufferFull();

private slots:
	void readData(int fd);
	void processBufferData();

private:
	void initMembers();
	void initSerial();
	int resetRainFall();
	int sendByte(uint8_t byte);
	int waitForAck(uint8_t *ack);
	inline int startCommand()
	{
		return sendByte(0x06);
	}
	inline int setAddressToRead(uint16_t addr)
	{
		// Memory setting commands are 4-digit chars, starting from MSB
		// Each char represents a hex digit of addr, from high to low
		for (int i = 0; i < 4; i++)
		{
			if (sendByte(addresssCommandOf(addr >> (3*4))) != 0)
				return (i + 1);
			addr = addr << 4;
		}
		return 0;
	}
	inline int setBytesToAsk(uint8_t count)
	{
		if (count > 15)		// Too many bytes
			return 1;
		bytesToRead = count;
		if (sendByte(bytesToAskCommandOf(count)) != 0)
			return 2;

		return 0;
	}
	inline uint8_t addresssCommandOf(uint8_t hex)
	{
		return (hex * 4 + 0x82);
	}
	inline uint8_t bytesToAskCommandOf(uint8_t hex)
	{
		return (hex * 4 + 0xc2);
	}
	inline uint8_t writeCommandOf(uint8_t hex)
	{
		return (hex * 4 + 0x42);
	}
	inline bool isAckCorrect(uint8_t *cmd, uint8_t *ack)
	{
		return ((*ack % 0x10 * 4 + 2 == *cmd % 0x40) ||
				(*ack == 0x02 && *cmd == 0x06)			);
	}
	inline int valueOfBcdDigits(QList<uint8_t> *digits, bool *valid = 0)
	{
		*valid = true;
		int value = 0;
		int multiplier = 1;
		for (int i = 0; i < digits->size(); i++)
		{
			// Get the two nibbles out
			uint8_t d = digits->at(i);
			uint8_t lo = d & 0xf;
			uint8_t hi = (d & 0xf0) >> 4;

			// If the lower nibble is not 0-9, the value is invalid
			if (lo > 0x9)
				*valid = false;
			value += lo * multiplier;
			multiplier *= 10;

			// The higher niblble, however, might just be garbage, we can't check it
			// But in that case, we want to break the loop
			if (hi > 0x9)
				break;
			value += hi * multiplier;
			multiplier *= 10;
		}

		return value;
	}

	int currentType;
	uint8_t bytesToRead;
	QList<uint8_t> bufferIn;

	uint16_t memoryLocations[Weather::Type::Not_Asking];
	unsigned int bytesToAsk[Weather::Type::Not_Asking];
	static const int Wind_Speed_Error_Code = 0x1fe;
};

#endif // WEATHERMODULE_H
