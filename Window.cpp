#include "Window.h"
#include <QLabel>
#include "MainView.h"
#include "StatusView.h"

Window::Window(QWidget *parent) : QTabWidget(parent)
{
	initMembers();
	this->setWindowTitle("WSN Embedded Gateway");
    this->showMaximized();
	this->show();
}

Window::~Window()
{
}

void Window::initMembers()
{
	main = new MainView(this);
	status = new StatusView(this);

	this->addTab(main, "Main");
	this->addTab(new QLabel("Weather"), "Weather");
	this->addTab(status, "Status");
	this->addTab(new QLabel("Settings"), "Settings");
}
