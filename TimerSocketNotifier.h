#ifndef TIMERSOCKETNOTIFIER_H
#define TIMERSOCKETNOTIFIER_H

#include <QObject>
class QTimer;

// Substitute for QSocketNotifier
// Relays SIGNAL(timeout()) when the timer is fired

class TimerSocketNotifier : public QObject
{
    Q_OBJECT

public:
    explicit TimerSocketNotifier(int fd, QObject *parent = 0);
	inline int socket()
	{
		return fd;
	}

	static const int CHECK_INTERVAL_MS = 1000;

signals:
	void timeout();

private:
	int fd;
	QTimer *readTimer;

};

#endif // TIMERSOCKETNOTIFIER_H
