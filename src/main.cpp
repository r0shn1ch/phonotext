#include "qtmainwindow.h"

#include <QApplication>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    QtMainWindow w;
    w.show();
    return app.exec();
}
