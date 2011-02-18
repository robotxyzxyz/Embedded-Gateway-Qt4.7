#ifndef WINDOW_H
#define WINDOW_H

#include <QTabWidget>
class MainView;
class StatusView;

class Window : public QTabWidget
{
    Q_OBJECT

public:
	explicit Window(QWidget *parent = 0);
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
	void initMembers();

private:
	MainView *main;
	StatusView *status;
};

#endif // WINDOW_H
