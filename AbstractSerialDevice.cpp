#include "AbstractSerialDevice.h"
#include <QSocketNotifier>
#include <fcntl.h>					// For file control options

AbstractSerialDevice::AbstractSerialDevice(QString path, QObject *parent) : QObject(parent)  //GSM�M��H�Ҳլ��~�ӳo���O
{
	serialPath = path;
	baud = B0;
}

AbstractSerialDevice::~AbstractSerialDevice()
{
	close(notifier->socket());
}

int AbstractSerialDevice::initSerial()
{
	const char *name = serialPath.toAscii().data();
	int fd = -1;
        fd = open(name, O_RDWR/*IO �iŪ�i�g*/ | O_NOCTTY/*NO ctrl TTY Ūԣ�Yԣ*/ | O_NONBLOCK /*Ū����F��*/);
	if (fd < 0)
		return fd;

        termios/*C�зǨ禡�w����*/ options;
	memset(&options, 0, sizeof(options));
        options.c_iflag = IGNPAR | IGNBRK/*����brk*/;
	options.c_cflag = baud | CS8 | CREAD | CLOCAL;
	cfsetispeed(&options, baud);
	cfsetospeed(&options, baud);		// Not sure if needed under Linux...
	tcflush(fd, TCIFLUSH);
        if (tcsetattr(fd, TCSAFLUSH/*�@�}�� �M�������S��*/, &options) != 0)
		return -1;

	notifier = new QSocketNotifier(fd, QSocketNotifier::Read, this);
	connect(notifier, SIGNAL(activated(int)), this, SLOT(readData(int)));

	return fd;
}
