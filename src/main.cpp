#include "qtmainwindow.h"
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QElapsedTimer>
#include <utility>
//#include <QTextCodec>

#include "phonotext.h"
#include "proccessing.h"

int main(int argc, char *argv[])
{
    QElapsedTimer timer;
    timer.start();

#ifdef _WIN32
    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForLocale(codec);
#endif

    QFile file("res/data/in1.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "File cannot be opened";
        return 0;
    }

    qDebug() << "start";

    QString data;
    QTextStream in(&file);
    //in.setCodec("UTF-8"); // кодировка UTF-8
    data = in.readAll();

    file.close();
    qDebug() << "Read End";

    Phonotext pt(data.toStdString());
    Proccessing proc(std::move(pt), "rus", 0., 100.);

    proc.createJson("res/data/outJson.json");
    proc.print("res/data/out.txt");

    qint64 elapsed_ms = timer.elapsed();
    qDebug() << "The time:" << elapsed_ms << "ms";

    QApplication a(argc, argv);
    QtMainWindow w;
    w.show();
    return a.exec();
}
