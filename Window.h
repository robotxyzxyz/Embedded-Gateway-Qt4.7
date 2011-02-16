#ifndef WINDOW_H
#define WINDOW_H

#include <QTabWidget>
class StatusView;

class Window : public QTabWidget
{
    Q_OBJECT

public:
	explicit Window(QWidget *parent = 0);
	virtual ~Window();

	StatusView *statusTab() const
	{
		return status;
	}

public slots:

private slots:
    void buildElements();

private:
	StatusView *status;
};

#endif // WINDOW_H
