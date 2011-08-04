#include <QtGui/QApplication>
#include <QFont>
#include "MainController.h"

int main(int argc, char *argv[])  //進入點
{
    QApplication a(argc, argv);

#ifdef ARM		// Font modification hack...Pass DEFINES+=ARM to qmake to use  //差別編譯
	QFont f = QApplication::font();
        f.setPointSize(f.pointSize() * 4);
	QApplication::setFont(f);
#endif

    MainController controller;  //load controller

    return a.exec();
}
