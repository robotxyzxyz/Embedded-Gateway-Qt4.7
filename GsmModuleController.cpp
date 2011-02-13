#include "GsmModuleController.h"
#include "GsmModule.h"

GsmModuleController::GsmModuleController(QString devicePath,
										 QString defaultTelNum,
										 QObject *parent)
				   : QThread(parent)
{
	initMembers(&devicePath, &defaultTelNum);
}

void GsmModuleController::initMembers(QString *devicePath,
									  QString *defaultTelNum)
{
	device = new GsmModule(*devicePath, this);
	isBusy = false;
	busyTime = 0;
	defaultNumber = *defaultTelNum;

	connect(device, SIGNAL(receivedLine(QString)),
			this, SLOT(gotLine(QString)));
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
	// Check line type
	if (line == "OK")
	{
		// OK signals command successfulness, clear business flag
		isBusy = false;

		// Arrange the received data (if any) and signal them
		for (int i = 0; i < bufferIn.size(); i++)
		{
			QString item = bufferIn[i];
			if (item.startsWith("+CSQ:"))
			{
				int csq = item.section(' ', 1, 1).toInt();
				emit receivedCarrierSignalQuality(csq);
			}
			if (item.startsWith("+CMGS:"))
			{
				int mr = item.section(' ', 1, 1).toInt();
				emit receivedSmsReference(mr);
			}
		}
	}
	else
	{
		bufferIn.append(line);
	}
}

void GsmModuleController::run()
{
	while(1)
	{
		// Check is the device is already busy
		if (!isBusy)
		{
			// If the device is idle, send a command (if needed)
			// Lock the device after sending
			if (!queue.isEmpty())
			{
				device->sendCommand(queue.takeFirst());
				isBusy = true;
			}

			// Reset busy time count
			busyTime = 0;
		}
		else if (busyTime > 60)
		{
			// If the device is busy for too long, force sending a command
			// The threshold is 1 minute (60 seconds)
			if (!queue.isEmpty())
			{
				device->sendCommand(queue.takeFirst());
				busyTime = 0;
			}
			emit occuredError(MODULE_TIMED_OUT_ERROR);
		}
		else
		{
			busyTime++;
		}

		// The above task runs every 1 second
		sleep(1);
	}
}
