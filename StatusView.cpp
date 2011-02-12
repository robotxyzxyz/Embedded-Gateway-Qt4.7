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
}

void StatusView::buildElements()
{
	logView = new QTextEdit(this);
	logView->setReadOnly(true);
	logView->setAcceptRichText(false);
	logView->setLineWrapMode(QTextEdit::WidgetWidth);

	deploy = new QPushButton("Deploy", this);
	collect = new QPushButton("Collect", this);
}

void StatusView::layoutElements()
{
	QHBoxLayout *bottom = new QHBoxLayout();
	bottom->addStretch(1);
	bottom->addWidget(deploy, 0);
	bottom->addWidget(collect, 0);

	QVBoxLayout *whole = new QVBoxLayout();
	whole->addWidget(logView, 1);
	whole->addLayout(bottom, 0);
	setLayout(whole);
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
