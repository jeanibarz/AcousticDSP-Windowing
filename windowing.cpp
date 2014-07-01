#define _USE_MATH_DEFINES
#include <cmath>

#include "windowing.h"
#include "ui_windowing.h"
#include "QFileDialog"
#include "QTextStream"
#include "QMessageBox"
#include "QSettings"
#include "QDebug"

#include <vector>

Windowing::Windowing(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Windowing)
{
    impulse = QVector<double>(0);
    window = QVector<double>(0);
    winImpulse = QVector<double>(0);

    ui->setupUi(this);

    m_sSettingsFile = QApplication::applicationDirPath() + "/settings.ini";
    if (QFile::exists(m_sSettingsFile)) loadSettings();
    else saveSettings();

    ui->importImpulsePushButton->setFocus();
}

Windowing::~Windowing()
{
    delete ui;
}

void Windowing::loadSettings()
{
    QSettings settings(m_sSettingsFile, QSettings::IniFormat);
    m_importDirectory = settings.value("importDirectory", "").toString();
    m_exportDirectory = settings.value("exportDirectory", "").toString();

    int value = settings.value("wLeftSideType", "").toInt();
    ui->wLeftSideTypeComboBox->setCurrentIndex(value);
    value = settings.value("wRightSideType", "").toInt();
    ui->wRightSideTypeComboBox->setCurrentIndex(value);
}

void Windowing::saveSettings()
{
    QSettings settings(m_sSettingsFile, QSettings::IniFormat);
    settings.setValue("importDirectory", m_importDirectory);
    settings.setValue("exportDirectory", m_exportDirectory);

    int value = ui->wLeftSideTypeComboBox->currentIndex();
    settings.setValue("wLeftSideType", value);
    value = ui->wRightSideTypeComboBox->currentIndex();
    settings.setValue("wRightSideType", value);
}

void Windowing::on_importImpulsePushButton_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
    tr("Import impulse"), m_importDirectory,
    tr("Text files (*.txt)"));
    if (!fileName.isEmpty())
    {
        if(loadFile(fileName)) {
            // Refresh plot
            p.setImpulseData(impulse);

            // Signal has been successfully imported, auto detect offset
            on_tAutoDetectPushButton_clicked();

            updateSignal();
        }
    }
}

bool Windowing::loadFile(const QString &fileName)
{
    m_importDirectory = QFileInfo(fileName).absoluteDir().absolutePath();
    saveSettings();

    if (!readFile(fileName)) {
        QMessageBox::critical(this, tr("Error !"), tr("Importing impulse failed !"));
        //statusBar()->showMessage(tr("Loading canceled"), 2000);
        return false;
    }

    QMessageBox::information(this, tr("Success !"), tr("Impulse successfully imported !"));
    //statusBar()->showMessage(tr("File loaded"), 2000);*/
    return true;
}

bool Windowing::readFile(const QString &fileName)
{
    impulse.clear();

    QFile file(fileName);
    file.open(QIODevice::ReadOnly | QIODevice::Text);

    QTextStream in(&file);
    unsigned int iLine = 0;

    while(!in.atEnd())
    {
        QString line = in.readLine();
        ++iLine;

        if (line.size() == 0)
            continue;
        else if (line.startsWith(QChar('#')))
            continue;

        line.replace(',', '.');
        bool ok = false;
        double value = line.toDouble(&ok);
        if (!ok)
        {
            QMessageBox::critical(this, tr("Error !"), tr("Error importing impulse : parsing line number ") + QString::number(iLine) + tr(" failed !"));
            impulse.clear();
            return false;
        }
        else
            impulse.push_back(value);
    }

    if (impulse.empty())
        return false;
    else
        return true;
}

void Windowing::updateSignal()
{
    unsigned int length = impulse.size();
    double peak = 0;
    double rms = 0;
    double value = 0;

    for (unsigned int i = 0; i < length; ++i)
    {
        value = std::abs(impulse[i]);
        rms += value*value;

        if (value > peak)
            peak = value;
    }

    rms = std::sqrt(rms) / (double)length;

    ui->wLeftSideTypeComboBox->setEnabled(true);
    ui->wRightSideTypeComboBox->setEnabled(true);
    ui->tAutoDetectComboBox->setEnabled(true);
    ui->tAutoDetectPushButton->setEnabled(true);
    ui->trTypeComboBox->setEnabled(true);
    ui->sLengthValue->setText(QString::number(length));

    on_tAutoDetectPushButton_clicked();

    updateWindow();

    //ui->sPeakValue->setText(QString::number(20*std::log10(peak), 'f', 2));
    //ui->sRmsLevelValue->setText(QString::number(20*std::log10(rms), 'f', 2));
}

void Windowing::on_sLengthValue_textChanged(const QString &arg1)
{
    if (arg1.size() == 0 || (arg1.toInt() >= 0 && arg1.toInt() <= 1))
        ui->sSignalLengthUnit->setText(tr("sample"));
    else if (arg1.toInt() > 1)
        ui->sSignalLengthUnit->setText(tr("samples"));
    else
    {
        QMessageBox::critical(this, tr("Error !"), tr("The signal length value is invalid ?"));
        close();
    }

    ui->wLeftSideWidthSlider->setEnabled(true);
    ui->wLeftSideWidthValue->setEnabled(true);
    ui->wRightSideWidthSlider->setEnabled(true);
    ui->wRightSideWidthValue->setEnabled(true);
    ui->tOffsetValue->setEnabled(true);

    ui->tOffsetValue->setMaximum(impulse.size());
    ui->trStartIndexValue->setMaximum(impulse.size() - 1);
    ui->trEndIndexValue->setMaximum(impulse.size() - 1);

    on_tAutoDetectPushButton_clicked();

    if (p.isVisible())
        updateWindow();
}

void Windowing::on_wLeftSideWidthValue_valueChanged(int arg1)
{
    if (arg1 <= 1)
        ui->wLeftSideWidthUnit->setText(tr("sample"));
    else
        ui->wLeftSideWidthUnit->setText(tr("samples"));

    if (ui->trTypeComboBox->currentIndex() == 0)
        updateTrWindowIndexes();

    if (p.isVisible())
        updateWindow();
}

void Windowing::on_wLeftSideWidthSlider_valueChanged(int value)
{
    ui->wLeftSideWidthValue->setValue(value);
}

void Windowing::on_wRightSideWidthValue_valueChanged(int arg1)
{
    if (arg1 <= 1)
        ui->wRightSideWidthUnit->setText(tr("sample"));
    else
        ui->wRightSideWidthUnit->setText(tr("samples"));

    // Update truncation indexes if truncation type is "window"
    if (ui->trTypeComboBox->currentIndex() == 0)
        updateTrWindowIndexes();

    if (p.isVisible())
        updateWindow();
}

void Windowing::on_wRightSideWidthSlider_valueChanged(int value)
{
    ui->wRightSideWidthValue->setValue(value);
}

void Windowing::on_tOffsetValue_valueChanged(int offset)
{
    int iMax = impulse.size() - 1;

    ui->wLeftSideWidthValue->setMaximum(offset);
    ui->wLeftSideWidthSlider->setMaximum(offset);
    ui->wRightSideWidthValue->setMaximum(iMax - offset);
    ui->wRightSideWidthSlider->setMaximum(iMax - offset);

    ui->wLeftSideWidthValue->setValue(offset);
    ui->wRightSideWidthValue->setValue(iMax - offset);
}

void Windowing::on_tAutoDetectPushButton_clicked()
{
    unsigned int length = impulse.size();
    double peakValue = 0;
    int peakIndex = 0;
    double value = 0;

    switch (ui->tAutoDetectComboBox->currentIndex())
    {
        case 0:
            for (unsigned int i = 0; i < length; ++i)
            {
                value = std::abs(impulse[i]);

                if (value > peakValue)
                {
                    peakValue = value;
                    peakIndex = i;
                }
            }
            break;
        case 1:
            for (unsigned int i = 0; i < length; ++i)
            {
                value = impulse[i];

                if (value > peakValue)
                {
                    peakValue = value;
                    peakIndex = i;
                }
            }
            break;
        default:
            QMessageBox::critical(this, tr("Error !"), tr("The time detection method is invalid ?"));
            break;
    };

    ui->tOffsetValue->setValue(peakIndex);
}

void Windowing::on_trStartIndexValue_valueChanged(int arg1)
{
    ui->trEndIndexValue->setMinimum(arg1);
    update_trSignalLengthValue();
}

void Windowing::on_trEndIndexValue_valueChanged(int arg1)
{
    ui->trStartIndexValue->setMaximum(arg1);
    update_trSignalLengthValue();
}

void Windowing::update_trSignalLengthValue()
{
    // Calculus of truncated signal length
    int trSignalLength = ui->trEndIndexValue->value() - ui->trStartIndexValue->value() + 1;

    // Disable export impulse push button if truncated signal length <= 0
    if (trSignalLength <= 0)
        ui->exportImpulsePushButton->setEnabled(false);
    // Enable export impulse push button if truncated signal length > 0
    else
        ui->exportImpulsePushButton->setEnabled(true);

    // Write truncated signal length to label
    ui->trSignalLengthValue->setText(QString::number(trSignalLength));
}

void Windowing::on_trSignalLengthValue_textChanged(const QString &arg1)
{
    if (arg1.toInt() > 1)
        ui->trSignalLengthUnit->setText(tr("samples"));
    else
        ui->trSignalLengthUnit->setText(tr("sample"));
}

void Windowing::updateTrWindowIndexes()
{
    int iMax = impulse.count() - 1;
    int offset = ui->tOffsetValue->value();
    int startIndex = offset - ui->wLeftSideWidthValue->value();
    int endIndex = offset + ui->wRightSideWidthValue->value();

    ui->trStartIndexValue->setMinimum(0);
    ui->trStartIndexValue->setMaximum(iMax);
    ui->trEndIndexValue->setMinimum(0);
    ui->trEndIndexValue->setMaximum(iMax);

    ui->trStartIndexValue->setValue(startIndex);
    ui->trEndIndexValue->setValue(endIndex);
}

void Windowing::on_trTypeComboBox_currentIndexChanged(int index)
{
    switch (index)
    {
        case 0:
            ui->trStartIndexValue->setEnabled(false);
            ui->trEndIndexValue->setEnabled(false);
            updateTrWindowIndexes();
            break;
        case 1:
            ui->trStartIndexValue->setEnabled(true);
            ui->trEndIndexValue->setEnabled(true);
            break;
        default:
            QMessageBox::critical(this, tr("Error !"), tr("Truncation type is invalid ?"));
            break;
    };
}

void Windowing::on_exportImpulsePushButton_clicked()
{
    updateWindow();

    QString fileName = QFileDialog::getSaveFileName(this,
    tr("Export impulse"), m_exportDirectory,
    tr("Text files (*.txt)"));
    if (!fileName.isEmpty())
        saveFile(fileName);
}

bool Windowing::saveFile(const QString &fileName)
{
    m_exportDirectory = QFileInfo(fileName).absoluteDir().absolutePath();
    saveSettings();

    if (!writeFile(fileName)) {
        QMessageBox::critical(this, tr("Error !"), tr("Exporting impulse failed !"));
        return false;
    }

    QMessageBox::information(this, tr("Success !"), tr("Impulse successfully exported !"));
    return true;
}

bool Windowing::writeFile(const QString &fileName)
{
    if (window.empty())
    {
        QMessageBox::critical(this, tr("Error !"), tr("Error writing windowed impulse : windowed impulse is empty !"));
        return false;
    }

    QFile file(fileName);
    file.open(QIODevice::WriteOnly | QIODevice::Text);

    QTextStream out(&file);

    if (!file.isWritable()) return false;

    int length = winImpulse.count();

    for (int i = 0; i < length; ++i)
    {
        out << winImpulse[i] << "\n";
        if (file.error())
            return false;
    }

    return true;
}

double Windowing::getWindowCoefficient(int windowType, int i, int width) {
    // The window coefficient is a half window, starting at index i = 0 (<=> window center) and bounded at index = width (the window generally reach value 0 here)

    if (i < 0) QMessageBox::critical(this, tr("Error !"), tr("Error getting window coefficient : index < 0 !"));

    double k;

    if (width != 0) k = M_PI/(width);
    else k = 0;

    switch (windowType)
    {
        case 0: // Rectangular
            if (i > width)
                return 0;
            else
                return 1;
            break;
        case 1: // Bartlett
            if (i > width)
                return 0;
            else
                return 1.0 - (i / (width + 1.0));
            break;
        case 2: // Welch
            if (i > width)
                return 0;
            else
                return 1.0 - pow(i / (width + 1.0), 2.0);
            break;
        case 3: // Hann
            if (i > width)
                return 0;
            else
                i = width - i;
                return sumOfCosine(i, k, 0.5, 0.5);
            break;
        case 4: // Hamming
            if (i > width)
                return 0;
            else
                i = width - i;
                return sumOfCosine(i, k, 0.54, 0.46);
            break;
        case 5: // Blackman
            if (i > width)
                return 0;
            else
                i = width - i;
                return sumOfCosine(i, k, 0.42659, 0.49656, 0.076849);
            break;
        case 6: // Nuttall
            if (i > width)
                return 0;
            else
                i = width - i;
                return sumOfCosine(i, k, 0.355768, 0.487396, 0.144232, 0.012604);
            break;
        case 7: // Blackman-Nuttall
            if (i > width)
                return 0;
            else
                i = width - i;
                return sumOfCosine(i, k, 0.3635819, 0.4891775, 0.1365995, 0.0106411);
            break;
        case 8: // Blackman-Harris
            if (i > width)
                return 0;
            else
                i = width - i;
                return sumOfCosine(i, k, 0.35875, 0.48829, 0.14128, 0.01168);
            break;
        case 9: // Flat top (peak normalized <==> rms attenuation)
            if (i > width)
                return 0;
            else
                i = width - i;
                return sumOfCosine(i, k, 1, 1.93, 1.29, 0.388, 0.028) / 4.636;
            break;
        default:
            QMessageBox::critical(this, tr("Error !"), tr("Left window type is invalid ?"));
            break;
    };
}

double Windowing::sumOfCosine(double n, double k, double a0, double a1, double a2, double a3, double a4)
{
    double nk = n*k;
    return a0 - a1*cos(nk) + a2*cos(2*nk) - a3*cos(3*nk) + a4*cos(4*nk);
}

void Windowing::on_wLeftSideTypeComboBox_currentIndexChanged(int index)
{
    saveSettings();
    if (p.isVisible()) updateWindow();
}

void Windowing::on_wRightSideTypeComboBox_currentIndexChanged(int index)
{
    saveSettings();
    if (p.isVisible()) updateWindow();
}

void Windowing::updateWindow() {
    int length = impulse.count();

    window = QVector<double>(length); // preallocating winImpulse

    int center = ui->tOffsetValue->value();
    int leftWidth = ui->wLeftSideWidthValue->value();
    int rightWidth = ui->wRightSideWidthValue->value();

    int leftType = ui->wLeftSideTypeComboBox->currentIndex();
    int rightType = ui->wRightSideTypeComboBox->currentIndex();

    int n;

    for (int i = 0; i < length; ++i) {
        if (i < center) { // left side
            n = center - i;
            window[i] = getWindowCoefficient(leftType, n, leftWidth);
        }
        else { // right side
            n = i - center;
            window[i] = getWindowCoefficient(rightType, n, rightWidth);
        }
    }

    updateWinImpulse();


}

void Windowing::updateWinImpulse() {
    int length = impulse.count();

    winImpulse = QVector<double>(length); // preallocating winImpulse

    for (int i = 0; i < length; ++i) {
        winImpulse[i] = impulse[i] * window[i];
    }

    if (p.isVisible()) {
        p.setWindowData(window, false);
        p.setWinImpulseData(winImpulse);
    }
}

void Windowing::on_plottingPushButton_clicked()
{
    if (!p.isVisible()) {
        updateWindow();
        p.setWindowData(window, false);
        p.setWinImpulseData(winImpulse);
        p.show();
    }
    else p.close();
}
