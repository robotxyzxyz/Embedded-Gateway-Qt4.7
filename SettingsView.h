#ifndef SETTINGSVIEW_H
#define SETTINGSVIEW_H

#include <QWidget>
class QLineEdit;
class QPushButton;
class Preferences;

class SettingsView : public QWidget
{
    Q_OBJECT

public:
	explicit SettingsView(Preferences *p, QWidget *parent = 0);

signals:

public slots:

private slots:
	void onApplyClicked();

private:
	void initMembers(Preferences *p);
	void layoutElements();

	QLineEdit *gatewayId;
	QLineEdit *serverPhone;
	QLineEdit *nodePort;
	QLineEdit *gsmPort;
	QLineEdit *systemTime;
	QPushButton *apply;
	Preferences *pref;
};

#endif // SETTINGSVIEW_H
