#include "StatusView.h"
#include <QHBoxLayout>
#include <QTextEdit>
#include <QPushButton>
#include <QScrollBar>
#include <QVBoxLayout>

StatusView::StatusView(QWidget *parent) : QWidget(parent)
{
	buildElements();
	layoutElements();
	initMembers();
}

void StatusView::buildElements()
{
	logView = new QTextEdit(this);
	logView->setReadOnly(true);
	logView->setAcceptRichText(false);
	logView->setLineWrapMode(QTextEdit::WidgetWidth);

	clear = new QPushButton("Clear log", this);
	deploy = new QPushButton("Deploy", this);
	collect = new QPushButton("Collect", this);
}

void StatusView::layoutElements()
{
	QHBoxLayout *buttons = new QHBoxLayout();
	buttons->addWidget(clear, 0);
	buttons->addStretch(1);
	buttons->addWidget(deploy, 0);
	buttons->addWidget(collect, 0);

	QVBoxLayout *whole = new QVBoxLayout();
	whole->addWidget(logView, 1);
	whole->addLayout(buttons, 0);
	setLayout(whole);
}

void StatusView::initMembers()
{
	connect(clear, SIGNAL(clicked()), this, SIGNAL(clearClicked()));
	connect(deploy, SIGNAL(clicked()), this, SIGNAL(deployClicked()));
	connect(collect, SIGNAL(clicked()), this, SIGNAL(collectClicked()));
}

void StatusView::log(QString text, bool inOwnLine)
{
	// Append log
	if (inOwnLine)
		logView->append(text);
	else
		logView->insertPlainText(text);

	// Scroll to bottom
	QScrollBar *scroll = logView->verticalScrollBar();
	scroll->setValue(scroll->maximum());
}

void StatusView::clearLog()
{
	logView->clear();
}
