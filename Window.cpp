#include "Window.h"
#include <QLabel>
#include "MainView.h"
#include "SettingsView.h"
#include "StatusView.h"

Window::Window(Preferences *p, QWidget *parent) : QTabWidget(parent)
{
	initMembers(p);
	this->setWindowTitle("WSN Embedded Gateway");
    this->showMaximized();
	this->show();
}

Window::~Window()
{
}

void Window::initMembers(Preferences *p)
{
	main = new MainView(this);
	status = new StatusView(this);
	settings = new SettingsView(p, this);

	this->addTab(main, "Main");
	this->addTab(new QLabel("Weather"), "Weather");
	this->addTab(status, "Status");
	this->addTab(settings, "Settings");
}
