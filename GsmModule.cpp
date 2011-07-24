#include "GsmModule.h"
#include "TimerSocketNotifier.h"

GsmModule::GsmModule(QString path, QObject *parent) : AbstractSerialDevice(path, parent)
{
	initMembers();

	if (initSerial() < 0)
		emit occuredError(Serial_Open_Error);
}

void GsmModule::initMembers()
{
	baud = B115200;
}

bool GsmModule::sendCommand(QString command)
{
	int fd = notifier->socket();
	for (int i = 0; i < command.length(); i++)
	{
		char c = command[i].toAscii();
		if (write(fd, &c, 1) != 1)
		{
			emit occuredError(Serial_Write_Error);
			return false;
		}
	}
	return true;
}

void GsmModule::readByte(int fd)
{
	// Read the byte
	char byte;
	if (read(fd, &byte, 1) != 1)
	{
		emit occuredError(Serial_Read_Error);
		return;
	}

	// Check if is an end-of character; if so, signal a receivedLine() event
	if (byte == '\n')
	{
		// If the last character in the buffer is '\r', this is a \r\n
		//  combination, we remove it
		int last = bufferIn.length() - 1;
		if (bufferIn.endsWith('\r'))
			bufferIn.remove(last);

		// Signal line receiving event if needed
		if (bufferIn.length() > 0)
			emit receivedLine(bufferIn);
		bufferIn.clear();
	}
	else if (byte == '\x1a')
	{
		// If the character is EOF (ASCII 0x1a), this is the end of an SMS
		//  input. Break this as a line too.
		if (bufferIn.length() > 0)
			emit receivedLine(bufferIn);
		bufferIn.clear();
	}

	// Otherwise just append the byte to the end of the buffer
	bufferIn.append(byte);
}


