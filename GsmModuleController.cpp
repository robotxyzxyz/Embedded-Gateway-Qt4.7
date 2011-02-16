#include "GsmModuleController.h"
#include "GsmModule.h"

GsmModuleController::GsmModuleController(QString devicePath,
										 QString defaultTelNum,
										 QObject *parent)
				   : QObject(parent)
{
	initMembers(&devicePath, &defaultTelNum);
	QObject::startTimer(5000);
	// The checking runs every 5 seconds, which is a bit long under normal
	//  circumstances...Maybe should use different delay time depending on
	//  different commands (CSQ is very quick, CMGS header is fine, and CMGS
	//  message sending part might even take loooooonger)
}

void GsmModuleController::initMembers(QString *devicePath,
									  QString *defaultTelNum)
{
	device = new GsmModule(*devicePath, this);
	busyTime = 0;
	defaultNumber = *defaultTelNum;

	connect(device, SIGNAL(receivedLine(QString)),
			this, SLOT(onReceivedLine(QString)));
	connect(device, SIGNAL(occuredError(const int)),
			this, SIGNAL(occuredError(const int)));
}

void GsmModuleController::sendRawString(QString string)
{
	queue.append(string);
}

void GsmModuleController::sendCarrierSignalQualityCommand()
{
	sendRawString("AT+CSQ\r\n");
}

void GsmModuleController::sendSmsCommand(QString message, QString *number)
{
	QString numberTo;
	if (number)
		numberTo = *number;
	else
		numberTo = defaultNumber;

	// The message is sent in to parts, otherwise it won't work...........
	// After the \r character we need to give the module a little time
	// One second (dafault separation between commands) is too long, but it
	//  doesn't hurt anyway...
	QString cmd;
	cmd = "AT+CMGS=\"";
	cmd.append(numberTo + "\"\r");
	sendRawString(cmd);

	cmd = message + "\x1a";
	sendRawString(cmd);
}

void GsmModuleController::onReceivedLine(QString line)
{
	if (line.contains("+CSQ:"))
	{
		int csq = line.section(' ', 1, 1)
					  .section(',', 0, 0)
					  .toInt();
		emit receivedCarrierSignalQuality(csq);
	}
	if (line.contains("+CMGS:"))
	{
		int mr = line.section(' ', 1, 1)
					 .section('\r', 0, 0)
					 .section('\n', 0, 0)
					 .toInt();
		emit receivedSmsReference(mr);
	}
}

void GsmModuleController::timerEvent(QTimerEvent *)
{
	// Ssend a command if needed
	if (!queue.isEmpty())
		device->sendCommand(queue.takeFirst());
}
