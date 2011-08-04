#include "AbstractSerialDevice.h"
#include <QSocketNotifier>
#include <fcntl.h>					// For file control options

AbstractSerialDevice::AbstractSerialDevice(QString path, QObject *parent) : QObject(parent)  //GSM和氣象模組皆繼承這類別
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
        fd = open(name, O_RDWR/*IO 可讀可寫*/ | O_NOCTTY/*NO ctrl TTY 讀啥吃啥*/ | O_NONBLOCK /*讀不到東西*/);
	if (fd < 0)
		return fd;

        termios/*C標準函式庫內建*/ options;
	memset(&options, 0, sizeof(options));
        options.c_iflag = IGNPAR | IGNBRK/*忽略brk*/;
	options.c_cflag = baud | CS8 | CREAD | CLOCAL;
	cfsetispeed(&options, baud);
	cfsetospeed(&options, baud);		// Not sure if needed under Linux...
	tcflush(fd, TCIFLUSH);
        if (tcsetattr(fd, TCSAFLUSH/*一開機 清掉有的沒的*/, &options) != 0)
		return -1;

	notifier = new QSocketNotifier(fd, QSocketNotifier::Read, this);
	connect(notifier, SIGNAL(activated(int)), this, SLOT(readData(int)));

	return fd;
}
