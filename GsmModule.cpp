#include "GsmModule.h"
#include <QSocketNotifier>
#include <fcntl.h>			// For file control options
#include <termios.h>		// For termios struct and options

GsmModule::GsmModule(QString path, QObject *parent) : QObject(parent)
{
	initMembers();

	if (initSerial(&path) < 0)
		emit occuredError(SERIAL_OPEN_ERROR);
}

GsmModule::~GsmModule()
{
	close(notifier->socket());
}

int GsmModule::initSerial(QString *path)
{
	serialPath = *path;
	const char *name = path->toAscii().data();
	int fd = -1;
	fd = open(name, O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (fd < 0)
		return fd;

	termios options;
	memset(&options, 0, sizeof(options));
	options.c_iflag = IGNPAR | IGNBRK;
	options.c_cflag = B115200 | CS8 | CREAD | CLOCAL;
	cfsetispeed(&options, B115200);
	cfsetospeed(&options, B115200);		// Not sure if needed under Linux...
	tcflush(fd, TCIFLUSH);
	if (tcsetattr(fd, TCSAFLUSH, &options) != 0)
		return -1;

	notifier = new QSocketNotifier(fd, QSocketNotifier::Read, this);
	connect(notifier, SIGNAL(activated(int)), this, SLOT(readData(int)));

	return fd;
}

void GsmModule::initMembers()
{
}

bool GsmModule::sendCommand(QString command)
{
	int fd = notifier->socket();
	for (int i = 0; i < command.length(); i++)
	{
		char c = command[i].toAscii();
		if (write(fd, &c, 1) != 1)
		{
			emit occuredError(SERIAL_WRITE_ERROR);
			return false;
		}
	}
	return true;
}

void GsmModule::readData(int fd)
{
	// Read the byte
	char byte;
	if (read(fd, &byte, 1) != 1)
	{
		emit occuredError(SERIAL_READ_ERROR);
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


