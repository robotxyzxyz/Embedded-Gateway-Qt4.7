#include "WeatherModule.h"
#include <QSocketNotifier>
#include <QThread>
#include <sys/ioctl.h>
#include "SleepableThread.h"

WeatherModule::WeatherModule(QString path, QObject *parent) : AbstractSerialDevice(path, parent)
{
	initMembers();

	if (initSerial() < 0)
		emit occuredError(Serial_Open_Error);
}

int WeatherModule::initSerial()
{
	int fd = this->AbstractSerialDevice::initSerial();

	if (fd < 0)
		return -1;

	// Set RTS to high and DTR to low
	int ios = 0;
	ioctl(fd, TIOCMGET, &ios);
	ios |= TIOCM_RTS;
	ios &= ~TIOCM_DTR;
	if (ioctl(fd, TIOCMSET, &ios) != 0)
		return -2;

	return fd;
}

void WeatherModule::initMembers()
{
	notifier->blockSignals(true);
	currentType = Weather::Type::Not_Asking;
	connect(this, SIGNAL(bufferFull()), this, SLOT(processBufferData()));

	// Initialize memory location and bytes to ask lookup tables

	memoryLocations[Weather::Type::Indoor_Temperature]		= 0x0346;
	memoryLocations[Weather::Type::Outdoor_Temperature]		= 0x0373;
	memoryLocations[Weather::Type::Wind_Chill]				= 0x03A0;
	memoryLocations[Weather::Type::Dew_Point]				= 0x03CE;
	memoryLocations[Weather::Type::Indoor_Humidity]			= 0x03FB;
	memoryLocations[Weather::Type::Outdoor_Humidity]		= 0x0419;
	memoryLocations[Weather::Type::Wind_Speed]				= 0x0529;
	memoryLocations[Weather::Type::Wind_Direction]			= 0x052C;
	memoryLocations[Weather::Type::Atomosphere_Pressure]	= 0x05D8;
	memoryLocations[Weather::Type::Rain_Fall]				= 0x04D2;

	bytesToAsk[Weather::Type::Indoor_Temperature]		= 2;
	bytesToAsk[Weather::Type::Outdoor_Temperature]		= 2;
	bytesToAsk[Weather::Type::Wind_Chill]				= 2;
	bytesToAsk[Weather::Type::Dew_Point]					= 2;
	bytesToAsk[Weather::Type::Indoor_Humidity]			= 1;
	bytesToAsk[Weather::Type::Outdoor_Humidity]			= 1;
	bytesToAsk[Weather::Type::Wind_Speed]				= 2;
	bytesToAsk[Weather::Type::Wind_Direction]			= 1;
	bytesToAsk[Weather::Type::Atomosphere_Pressure]		= 3;
	bytesToAsk[Weather::Type::Rain_Fall]					= 3;
}

int WeatherModule::sendCommand(int type)
{
	currentType = Weather::Type::Not_Asking;

	if (type == Weather::Type::Rain_Fall_Reset)
		return resetRainFall();

	if (startCommand() != All_Ok)
		return Command_Start_Error;
	if (setAddressToRead(memoryLocations[type]) != All_Ok)
		return Set_Address_Error;
	if (setBytesToAsk(bytesToAsk[type]) != All_Ok)
		return Set_Bytes_Error;

	notifier->blockSignals(false);	// Start notifier reading
	bufferIn.clear();
	currentType = type;

	return All_Ok;
}

int WeatherModule::resetRainFall()
{
	if (startCommand() != All_Ok)
		return Command_Start_Error;
	if (setAddressToRead(memoryLocations[Weather::Type::Rain_Fall]) != All_Ok)
		return Set_Address_Error;
	for (int i = 0; i < 6; i++)
	{
		if (sendByte(writeCommandOf(0)))
			return Serial_Write_Error;
	}
	return All_Ok;
}

int WeatherModule::sendByte(uint8_t byte)
{
	int fd = notifier->socket();
	tcflush(fd, TCIOFLUSH);	// clean garbage
	uint8_t ack;

	if (write(fd, &byte, 1) != 1)
		return Serial_Write_Error;
	if (waitForAck(&ack) != All_Ok)
		return No_Acknowledgement_Error;
	if (isAckCorrect(&byte, &ack) == false)
		return Wrong_Acknowledgement_Error;

	return All_Ok;
}

int WeatherModule::waitForAck(uint8_t *ack)
{
	SleepableThread::msleep(100);		// Wait a while for ACK
	unsigned int i = 0;
	int fd = notifier->socket();
	do
	{
		if (read(fd, ack, 1) != 1)
			continue;
		if (i > 0xff)
			return No_Acknowledgement_Error;
		i++;
	} while (*ack == 0x08);		// 0x08 is idle signal of serial

	return All_Ok;
}

void WeatherModule::readData(int fd)
{
	notifier->setEnabled(false);
	uint8_t byte;
	if (read(fd, &byte, 1) != 1)
	{
		bytesToRead = 0;
		return;
	}
	bufferIn.append(byte);
	bytesToRead--;
	if (bytesToRead == 0)
	{
		emit bufferFull();
		notifier->blockSignals(true);
	}
	notifier->setEnabled(true);
	currentType = Weather::Type::Not_Asking;
}

void WeatherModule::processBufferData()
{
	int value = 0;
	bool valid = true;
	switch (currentType)
	{
		case Weather::Type::Indoor_Temperature:
		case Weather::Type::Outdoor_Temperature:
		case Weather::Type::Dew_Point:
			value = valueOfBcdDigits(&bufferIn, &valid) / 10 - 300;
			break;

		case Weather::Type::Indoor_Humidity:
		case Weather::Type::Outdoor_Humidity:
			value = valueOfBcdDigits(&bufferIn, &valid) * 10;
			break;

		case Weather::Type::Wind_Chill:
			value = valueOfBcdDigits(&bufferIn, &valid) / 10;
			break;

		case Weather::Type::Wind_Speed:
			value = bufferIn[0] + (bufferIn[1] & 0xf) * 0x100;
			if (value == Wind_Speed_Error_Code)
				valid = false;
			break;

		case Weather::Type::Wind_Direction:
			value = bufferIn[0] & 0xf;
			break;

		case Weather::Type::Atomosphere_Pressure:
			value = valueOfBcdDigits(&bufferIn, &valid) % 100000;
			break;

		case Weather::Type::Rain_Fall:
			value = valueOfBcdDigits(&bufferIn, &valid) / 10;
			break;

		default:
			valid = false;
			break;
	}
	if (valid)
	{
		Weather::Datum datum;
		datum.type = currentType;
		datum.value = value;
		emit receivedDatum(datum);
	}
	bufferIn.clear();
}
