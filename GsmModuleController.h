#ifndef GSMMODULECONTROLLER_H
#define GSMMODULECONTROLLER_H

#include <QObject>
#include <QStringList>
class GsmModule;

class GsmModuleController : public QObject
{
    Q_OBJECT

public:
	explicit GsmModuleController(QString devicePath,
								 QString defaultTelNum,
								 QObject *parent = 0);
	void run();
	GsmModule *module() const
	{
		return device;
	}

	static const int MODULE_TIMED_OUT_ERROR = 11;

signals:
	/*********** IMPORTANT: Some error codes are in GsmMoudle.h *************/
	void occuredError(const int errorCode);
	void receivedCarrierSignalQuality(int csq);
	void receivedSmsReference(int mr);

public slots:
	void sendRawString(QString string);
	void sendCarrierSignalQualityCommand();
	void sendSmsCommand(QString message, QString *number = 0);

protected:
	void timerEvent(QTimerEvent *e);

private slots:
	void onReceivedLine(QString line);

private:
	void initMembers(QString *devicePath, QString *defaultTelNum);

	GsmModule *device;
	QString defaultNumber;
	QStringList queue;
	QStringList bufferIn;
	int busyTime;

};

#endif // GSMMODULECONTROLLER_H
