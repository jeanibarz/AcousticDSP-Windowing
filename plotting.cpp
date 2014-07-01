#include "plotting.h"
#include "ui_plotting.h"

#include <limits>

Plotting::Plotting(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Plotting)
{
    ui->setupUi(this);

    ui->plottingWidget->xAxis->setLabel("index");
    ui->plottingWidget->yAxis->setLabel("amplitude");

    ui->plottingWidget->xAxis->setRange(-1.1, 1.1);
    ui->plottingWidget->yAxis->setRange(-1.1, 1.1);

    ui->plottingWidget->addGraph(); // impulse
    ui->plottingWidget->graph(0)->setPen(QPen(Qt::blue));

    ui->plottingWidget->addGraph(); // window
    ui->plottingWidget->graph(1)->setPen(QPen(Qt::red));

    ui->plottingWidget->addGraph(); // winImpulse
    ui->plottingWidget->graph(2)->setPen(QPen(Qt::green));

    ui->plottingWidget->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom | QCP::iSelectAxes);

    // connect slot that ties some axis selections together (especially opposite axes):
    connect(ui->plottingWidget, SIGNAL(selectionChangedByUser()), this, SLOT(selectionChanged()));

    // connect slots that takes care that when an axis is selected, only that direction can be dragged and zoomed:
    connect(ui->plottingWidget, SIGNAL(mouseWheel(QWheelEvent*)), this, SLOT(mouseWheel()));
}

Plotting::~Plotting()
{
    delete ui;
}

void Plotting::setImpulseData(QVector<double> impulse, bool replot) {
    int length = impulse.count();
    QVector<double> index(length);
    for (int i = 0; i < length; ++i) {
        index[i] = i;
    }

    ui->plottingWidget->graph(0)->setData(index, impulse);

    // set axes ranges, so we see all data:
    ui->plottingWidget->xAxis->setRange(0, length-1);
    double amplitudeMin = std::numeric_limits<double>::max();
    double amplitudeMax = std::numeric_limits<double>::min();
    double val;

    for (int i = 0; i < length; ++i) {
        val = impulse[i];
        if (val < amplitudeMin) amplitudeMin = val;
        if (val > amplitudeMax) amplitudeMax = val;
    }

    ui->plottingWidget->yAxis->setRange(amplitudeMin * 1.1, amplitudeMax * 1.1);

    if (replot) ui->plottingWidget->replot();
}

void Plotting::setWindowData(QVector<double> window, bool replot) {
    int length = window.count();
    QVector<double> index(length);
    for (int i = 0; i < length; ++i) {
        index[i] = i;
    }

    ui->plottingWidget->graph(1)->setData(index, window);

    if (replot) ui->plottingWidget->replot();
}

void Plotting::setWinImpulseData(QVector<double> winImpulse, bool replot) {
    int length = winImpulse.count();
    QVector<double> index(length);
    for (int i = 0; i < length; ++i) {
        index[i] = i;
    }

    ui->plottingWidget->graph(2)->setData(index, winImpulse);

    if (replot) ui->plottingWidget->replot();
}

void Plotting::selectionChanged()
{
  /*
   normally, axis base line, axis tick labels and axis labels are selectable separately, but we want
   the user only to be able to select the axis as a whole, so we tie the selected states of the tick labels
   and the axis base line together. However, the axis label shall be selectable individually.

   The selection state of the left and right axes shall be synchronized as well as the state of the
   bottom and top axes.

   Further, we want to synchronize the selection of the graphs with the selection state of the respective
   legend item belonging to that graph. So the user can select a graph by either clicking on the graph itself
   or on its legend item.
  */

  // make top and bottom axes be selected synchronously, and handle axis and tick labels as one selectable object:
  if (ui->plottingWidget->xAxis->selectedParts().testFlag(QCPAxis::spAxis) || ui->plottingWidget->xAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
      ui->plottingWidget->xAxis2->selectedParts().testFlag(QCPAxis::spAxis) || ui->plottingWidget->xAxis2->selectedParts().testFlag(QCPAxis::spTickLabels))
  {
    ui->plottingWidget->xAxis2->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
    ui->plottingWidget->xAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
  }
  // make left and right axes be selected synchronously, and handle axis and tick labels as one selectable object:
  if (ui->plottingWidget->yAxis->selectedParts().testFlag(QCPAxis::spAxis) || ui->plottingWidget->yAxis->selectedParts().testFlag(QCPAxis::spTickLabels) ||
      ui->plottingWidget->yAxis2->selectedParts().testFlag(QCPAxis::spAxis) || ui->plottingWidget->yAxis2->selectedParts().testFlag(QCPAxis::spTickLabels))
  {
    ui->plottingWidget->yAxis2->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
    ui->plottingWidget->yAxis->setSelectedParts(QCPAxis::spAxis|QCPAxis::spTickLabels);
  }

  // synchronize selection of graphs with selection of corresponding legend items:
  for (int i=0; i<ui->plottingWidget->graphCount(); ++i)
  {
    QCPGraph *graph = ui->plottingWidget->graph(i);
    QCPPlottableLegendItem *item = ui->plottingWidget->legend->itemWithPlottable(graph);
    if (item->selected() || graph->selected())
    {
      item->setSelected(true);
      graph->setSelected(true);
    }
  }
}

void Plotting::mouseWheel() {
    if (ui->plottingWidget->xAxis->selectedParts().testFlag(QCPAxis::spAxis))
      ui->plottingWidget->axisRect()->setRangeZoom(ui->plottingWidget->xAxis->orientation());
    else if (ui->plottingWidget->yAxis->selectedParts().testFlag(QCPAxis::spAxis))
      ui->plottingWidget->axisRect()->setRangeZoom(ui->plottingWidget->yAxis->orientation());
    else
      ui->plottingWidget->axisRect()->setRangeZoom(Qt::Horizontal|Qt::Vertical);
}
