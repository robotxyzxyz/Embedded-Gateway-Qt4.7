#ifndef WINDOW_H
#define WINDOW_H

#include <QTabWidget>
class MainView;
class Preferences;
class SettingsView;
class StatusView;

class Window : public QTabWidget
{
    Q_OBJECT

public:
	explicit Window(Preferences *p, QWidget *parent = 0);
	virtual ~Window();

	MainView *mainTab() const
	{
		return main;
	}
	StatusView *statusTab() const
	{
		return status;
	}

public slots:

private slots:
	void initMembers(Preferences *p);

private:
	MainView *main;
	StatusView *status;
	SettingsView *settings;
};

#endif // WINDOW_H
