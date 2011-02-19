#include <QtGui/QApplication>
#include <QFont>
#include "MainController.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

/*#ifdef ARM		// Font modification hack...Pass DEFINES+=ARM to qmake to use
	QFont f = QApplication::font();
	f.setPointSize(f.pointSize() * 3);
	QApplication::setFont(f);
#endif*/

	QFont f = QApplication::font();
	f.setPointSize(12);
	QApplication::setFont(f);

    MainController controller;

    return a.exec();
}
