#ifndef PLOTTING_H
#define PLOTTING_H

#include <QWidget>

namespace Ui {
class Plotting;
}

class Plotting : public QWidget
{
    Q_OBJECT

public:
    explicit Plotting(QWidget *parent = 0);
    ~Plotting();

    void setImpulseData(QVector<double> impulse, bool replot = true);
    void setWindowData(QVector<double> window, bool replot = true);
    void setWinImpulseData(QVector<double> winImpulse, bool replot = true);
   // void setData(QVector<double> impulse, QVector<double> window, int start, int end);

private slots:
    void selectionChanged();
    void mouseWheel();

private:
    Ui::Plotting *ui;
};

#endif // PLOTTING_H
