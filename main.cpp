#include "windowing.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Windowing w;
    w.show();

    return a.exec();
}
