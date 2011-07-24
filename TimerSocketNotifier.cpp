#include <QTimer>
#include "TimerSocketNotifier.h"

TimerSocketNotifier::TimerSocketNotifier(int fd, QObject *parent)
				  : QObject(parent)
{
	this->fd = fd;
	readTimer = new QTimer(this);
	readTimer->start(TimerSocketNotifier::CHECK_INTERVAL_MS);
	connect(readTimer, SIGNAL(timeout()), this, SIGNAL(timeout()));
}
