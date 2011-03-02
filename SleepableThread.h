#ifndef SLEEPABLETHREAD_H
#define SLEEPABLETHREAD_H

#include <QThread>

class SleepableThread : public QThread
{
    Q_OBJECT

public:
    explicit SleepableThread(QObject *parent = 0);

	static void msleep(unsigned long msecs)
	{
		QThread::msleep(msecs);
	}
};

#endif // SLEEPABLETHREAD_H
