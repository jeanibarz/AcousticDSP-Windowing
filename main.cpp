#include "windowing.h"
#include <QApplication>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QString locale = QLocale::system().name().section('_', 0, 0);

    QTranslator translator;
        translator.load(QString("://translations/windowing_") + locale);
        a.installTranslator(&translator);

    Windowing w;
    w.show();

    return a.exec();
}
