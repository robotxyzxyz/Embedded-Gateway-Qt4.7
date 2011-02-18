#include "Window.h"
#include <QLabel>
#include "StatusView.h"

Window::Window(QWidget *parent) : QTabWidget(parent)
{
    buildElements();
	initMembers();
    this->showMaximized();
	this->show();
}

Window::~Window()
{
}

void Window::buildElements()
{
	status = new StatusView(this);

    this->addTab(new QLabel("Main"), "Main");
    this->addTab(new QLabel("Weather"), "Weather");
	this->addTab(status, "Status");
    this->addTab(new QLabel("Settings"), "Settings");
}

void Window::initMembers()
{
	connect(status, SIGNAL(clearClicked()), this, SIGNAL(clearLogTriggered()));
	connect(status, SIGNAL(deployClicked()), this, SIGNAL(deployTriggered()));
	connect(status, SIGNAL(collectClicked()), this, SIGNAL(collectTriggered()));
}
