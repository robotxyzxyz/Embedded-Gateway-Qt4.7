#include "AbstractSerialDevice.h"
#include <QSocketNotifier>
#include <fcntl.h>					// For file control options

AbstractSerialDevice::AbstractSerialDevice(QString path, QObject *parent) : QObject(parent)
{
	serialPath = path;
	baud = B0;
	valid = false;
}

AbstractSerialDevice::~AbstractSerialDevice()
{
	close(notifier->socket());
}

int AbstractSerialDevice::initSerial()
{
	const char *name = serialPath.toAscii().data();
	int fd = -1;
	fd = open(name, O_RDWR | O_NOCTTY | O_NONBLOCK);
	if (fd < 0)
		return fd;

	termios options;
	memset(&options, 0, sizeof(options));
	options.c_iflag = IGNPAR | IGNBRK;
	options.c_cflag = baud | CS8 | CREAD | CLOCAL;
	cfsetispeed(&options, baud);
	cfsetospeed(&options, baud);		// Not sure if needed under Linux...
	tcflush(fd, TCIFLUSH);
	if (tcsetattr(fd, TCSAFLUSH, &options) != 0)
		return -1;

	notifier = new QSocketNotifier(fd, QSocketNotifier::Read, this);
	connect(notifier, SIGNAL(activated(int)), this, SLOT(readData(int)));

	return fd;
}
