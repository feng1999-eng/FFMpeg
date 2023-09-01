#include "QtWidgetsApplication2.h"
#include <QtWidgets/QApplication>
#include "FFmpegWidget.h"
int main(int argc, char *argv[])
{

    QApplication a(argc, argv);

    QtWidgetsApplication2 w;
    w.show();
    return a.exec();
}
