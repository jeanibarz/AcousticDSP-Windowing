#ifndef WINDOWING_H
#define WINDOWING_H

#include <QWidget>
#include <vector>

#include "plotting.h"

namespace Ui {
class Windowing;
}

class Windowing : public QWidget
{
    Q_OBJECT

public:
    explicit Windowing(QWidget *parent = 0);
    ~Windowing();

private slots:
    void on_importImpulsePushButton_clicked();

    void on_sLengthValue_textChanged(const QString &arg1);

    void on_wLeftSideWidthValue_valueChanged(int arg1);

    void on_wRightSideWidthValue_valueChanged(int arg1);

    void on_wLeftSideWidthSlider_valueChanged(int value);

    void on_wRightSideWidthSlider_valueChanged(int value);

    void on_tOffsetValue_valueChanged(int arg1);

    void on_tAutoDetectPushButton_clicked();

    void on_trStartIndexValue_valueChanged(int arg1);

    void on_trEndIndexValue_valueChanged(int arg1);

    void update_trSignalLengthValue();

    void on_trSignalLengthValue_textChanged(const QString &arg1);

    void on_trTypeComboBox_currentIndexChanged(int index);

    void updateTrWindowIndexes();

    void on_exportImpulsePushButton_clicked();

    bool saveFile(const QString &fileName);

    bool writeFile(const QString &fileName);

    double sumOfCosine(double n, double k, double a0 = 0, double a1 = 0, double a2 = 0, double a3 = 0, double a4 = 0);

    void on_wLeftSideTypeComboBox_currentIndexChanged(int index);

    void on_wRightSideTypeComboBox_currentIndexChanged(int index);

    void on_plottingPushButton_clicked();

private:
    QVector<double> impulse;
    QVector<double> winImpulse;

    QString m_sSettingsFile;
    QString m_importDirectory;
    QString m_exportDirectory;

    void loadSettings();
    void saveSettings();

    bool loadFile(const QString &fileName);
    bool readFile(const QString &fileName);
    void updateSignal();

    Ui::Windowing *ui;

    Plotting p;
};

#endif // WINDOWING_H
