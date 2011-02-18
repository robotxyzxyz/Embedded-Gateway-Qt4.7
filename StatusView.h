#ifndef STATUSVIEW_H
#define STATUSVIEW_H

#include <QWidget>
class QTextEdit;
class QPushButton;

class StatusView : public QWidget
{
    Q_OBJECT

public:
    explicit StatusView(QWidget *parent = 0);

public slots:
	void log(QString text, bool inOwnLine = true);
	void clearLog();

signals:
	void clearLogTriggered();
	void deployTriggered();
	void collectTriggered();

private:
	void initMembers();
	void layoutElements();

	QTextEdit *logView;
	QPushButton *clear;
	QPushButton *deploy;
	QPushButton *collect;

};

#endif // STATUSVIEW_H
