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
    winImpulse = QVector<double>(0);

    ui->setupUi(this);

    m_sSettingsFile = QApplication::applicationDirPath() + "/settings.ini";
    if (QFile::exists(m_sSettingsFile)) loadSettings();
    else saveSettings();
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
        loadFile(fileName);
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

    updateSignal();
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

}

void Windowing::on_wLeftSideWidthValue_valueChanged(int arg1)
{
    if (arg1 <= 1)
        ui->wLeftSideWidthUnit->setText(tr("sample"));
    else
        ui->wLeftSideWidthUnit->setText(tr("samples"));

    ui->wLeftSideWidthSlider->setValue(arg1);

    if (ui->trTypeComboBox->currentIndex() == 0)
    {
        updateTrWindowIndexes();
    }
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

    ui->wRightSideWidthSlider->setValue(arg1);

    if (ui->trTypeComboBox->currentIndex() == 0)
    {
        updateTrWindowIndexes();
    }
}

void Windowing::on_wRightSideWidthSlider_valueChanged(int value)
{
    ui->wRightSideWidthValue->setValue(value);
}

void Windowing::on_tOffsetValue_valueChanged(int arg1)
{
    int sLength = impulse.size();

    ui->wLeftSideWidthValue->setMaximum(arg1);
    ui->wLeftSideWidthSlider->setMaximum(arg1);
    ui->wRightSideWidthValue->setMaximum(sLength - 1 - arg1);
    ui->wRightSideWidthSlider->setMaximum(sLength - 1 - arg1);

    ui->wLeftSideWidthValue->setValue(arg1);
    ui->wRightSideWidthValue->setValue(sLength - 1 - arg1);
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
    on_tOffsetValue_valueChanged(peakIndex);
    updateTrWindowIndexes();
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
    int trSignalLength = 1 + ui->trEndIndexValue->value() - ui->trStartIndexValue->value();
    if (trSignalLength <= 0)
    {
        ui->exportImpulsePushButton->setEnabled(false);
        ui->trSignalLengthValue->setText(QString::number(trSignalLength));
    }
    else
    {
        ui->exportImpulsePushButton->setEnabled(true);
        ui->trSignalLengthValue->setText(QString::number(trSignalLength));
    }
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
    ui->trStartIndexValue->setMaximum(impulse.size()-1);
    ui->trEndIndexValue->setMinimum(0);

    ui->trEndIndexValue->setValue(ui->tOffsetValue->value() + ui->wRightSideWidthValue->value());
    ui->trStartIndexValue->setValue(ui->tOffsetValue->value() - ui->wLeftSideWidthValue->value());
    update_trSignalLengthValue();
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
    winImpulse.clear();

    // Start/end signal indexes
    int startIndex = ui->trStartIndexValue->value();
    int endIndex = ui->trEndIndexValue->value();

    // Left side window
    double width = (double)ui->wLeftSideWidthValue->value();
    double center = (double)ui->tOffsetValue->value();
    double n = 0;
    double L = 2*width;
    double k = 2*M_PI/L;
    double value = 0;

    // Left side window applying
    if (width > 0)
    {
        for (int i = startIndex; i < center && i <= endIndex; ++i)
        {
            n = width-center+i;

            switch (ui->wLeftSideTypeComboBox->currentIndex())
            {
                case 0: // Rectangular
                    if (i < (center-width))
                        value = 0;
                    else
                        value = 1;
                    break;
                case 1: // Bartlett
                    if (i < (center - width))
                        value = 0;
                    else
                        value = 1.0 + (n-width) / (width+1.0);
                    break;
                case 2: // Welch
                    if (i < (center - width))
                        value = 0;
                    else
                        value = 1.0 - pow((n-width) / (width+1.0), 2.0);
                    break;
                case 3: // Hann
                    if (i < (center - width))
                        value = 0;
                    else
                        value = sumOfCosine(n, k, 0.5, 0.5);
                    break;
                case 4: // Hamming
                    if (i < (center - width))
                        value = 0;
                    else
                        value = sumOfCosine(n, k, 0.54, 0.46);
                    break;
                case 5: // Blackman
                    if (i < (center - width))
                        value = 0;
                    else
                        value = sumOfCosine(n, k, 0.42659, 0.49656, 0.076849);
                    break;
                case 6: // Nuttall
                    if (i < (center - width))
                        value = 0;
                    else
                        value = sumOfCosine(n, k, 0.355768, 0.487396, 0.144232, 0.012604);
                    break;
                case 7: // Blackman-Nuttall
                    if (i < (center - width))
                        value = 0;
                    else
                        value = sumOfCosine(n, k, 0.3635819, 0.4891775, 0.1365995, 0.0106411);
                    break;
                case 8: // Blackman-Harris
                    if (i < (center - width))
                        value = 0;
                    else
                        value = sumOfCosine(n, k, 0.35875, 0.48829, 0.14128, 0.01168);
                    break;
                case 9: // Flat top (peak normalized <==> rms attenuation)
                    if (i < (center - width))
                        value = 0;
                    else
                        value = sumOfCosine(n, k, 1, 1.93, 1.29, 0.388, 0.028) / 4.636;
                    break;
                default:
                    QMessageBox::critical(this, tr("Error !"), tr("Left window type is invalid ?"));
                    break;
            };
            winImpulse.push_back(impulse[i] * value);
        }
    }

    // Center
    if (center >= startIndex && center <= endIndex)
        winImpulse.push_back(impulse[center]);

    // Right side window
    width = (double)ui->wRightSideWidthValue->value();
    L = 2*width;
    k = 2*M_PI/L;
    value = 0;

    // Right side window applying
    if (width > 0)
    {
        for (int i = center+1; i <= endIndex; ++i)
        {
            n = width-i+center;

            switch (ui->wRightSideTypeComboBox->currentIndex())
            {
                case 0: // Rectangular
                    if (i > (center+width))
                        value = 0;
                    else
                        value = 1;
                    break;
                case 1: // Bartlett
                    if (i > (center + width))
                        value = 0;
                    else
                        value = 1.0 + (n-width) / (width+1.0);
                    break;
                case 2: // Welch
                    if (i > (center + width))
                        value = 0;
                    else
                        value = 1.0 - pow((n-width) / (width+1.0), 2.0);
                    break;
                case 3: // Hann
                    if (i > (center + width))
                        value = 0;
                    else
                        value = sumOfCosine(n, k, 0.5, 0.5);
                    break;
                case 4: // Hamming
                    if (i > (center + width))
                        value = 0;
                    else
                        value = sumOfCosine(n, k, 0.54, 0.46);
                    break;
                case 5: // Blackman
                    if (i > (center + width))
                        value = 0;
                    else
                        value = sumOfCosine(n, k, 0.42659, 0.49656, 0.076849);
                    break;
                case 6: // Nuttall
                    if (i > (center + width))
                        value = 0;
                    else
                        value = sumOfCosine(n, k, 0.355768, 0.487396, 0.144232, 0.012604);
                    break;
                case 7: // Blackman-Nuttall
                    if (i > (center + width))
                        value = 0;
                    else
                        value = sumOfCosine(n, k, 0.3635819, 0.4891775, 0.1365995, 0.0106411);
                    break;
                case 8: // Blackman-Harris
                    if (i > (center + width))
                        value = 0;
                    else
                        value = sumOfCosine(n, k, 0.35875, 0.48829, 0.14128, 0.01168);
                    break;
                case 9: // Flat top (peak normalized <==> rms attenuation)
                    if (i > (center + width))
                        value = 0;
                    else
                        value = sumOfCosine(n, k, 1, 1.93, 1.29, 0.388, 0.028) / 4.636;
                    break;
                default:
                    QMessageBox::critical(this, tr("Error !"), tr("Right window type is invalid ?"));
                    break;
            };
            winImpulse.push_back(impulse[i] * value);
        }
    }

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
    if (winImpulse.empty())
    {
        QMessageBox::critical(this, tr("Error !"), tr("Error writing windowed impulse : windowed impulse is empty !"));
        return false;
    }

    QFile file(fileName);
    file.open(QIODevice::WriteOnly | QIODevice::Text);

    QTextStream out(&file);

    if (!file.isWritable()) return false;

    for (unsigned int i = 0; i < winImpulse.size(); ++i)
    {
        out << winImpulse[i] << "\n";
        if (file.error())
            return false;
    }

    return true;
}

double Windowing::sumOfCosine(double n, double k, double a0, double a1, double a2, double a3, double a4)
{
    double nk = n*k;
    return a0 - a1*cos(nk) + a2*cos(2*nk) - a3*cos(3*nk) + a4*cos(4*nk);
}

void Windowing::on_wLeftSideTypeComboBox_currentIndexChanged(int index)
{
    saveSettings();
}

void Windowing::on_wRightSideTypeComboBox_currentIndexChanged(int index)
{
    saveSettings();
}

void Windowing::on_plottingPushButton_clicked()
{
    p.setData(impulse, winImpulse, ui->trStartIndexValue->value(), ui->trEndIndexValue->value());
    p.show();
}
