#include "windowing.h"
#include <QApplication>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
        translator.load("://translations/windowing_fr");
        a.installTranslator(&translator);

    Windowing w;
    w.show();

    return a.exec();
}
