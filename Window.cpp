#include "Window.h"
#include <QLabel>
#include "StatusView.h"

Window::Window(QWidget *parent) : QTabWidget(parent)
{
    buildElements();
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
