/* ============================================================================
 * Copyright (c) 2010, Michael A. Jackson (BlueQuartz Software)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * Neither the name of Michael A. Jackson nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#include "StatsGenODFWidget.h"

//-- C++ Includes
#include <iostream>

//-- Qt Includes
#include <QtGui/QAbstractItemDelegate>

#include <qwt.h>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_abstract_scale_draw.h>
#include <qwt_scale_draw.h>

#include "AIM/Common/Texture.h"
#include "StatsGenerator/TableModels/SGODFTableModel.h"
#include "StatsGenerator/StatsGenMDFWidget.h"
#include "StatsGenerator/TextureDialog.h"
#include "StatsGen.h"


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
StatsGenODFWidget::StatsGenODFWidget(QWidget *parent) :
QWidget(parent),
m_Initializing(true),
m_PhaseIndex(-1),
m_CrystalStructure(AIM::Reconstruction::Cubic),
m_ODFTableModel(NULL),
m_MDFWidget(NULL)
{
  this->setupUi(this);
  this->setupGui();
}


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
StatsGenODFWidget::~StatsGenODFWidget()
{
  if (NULL != m_ODFTableModel)
  {
    m_ODFTableModel->deleteLater();
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int StatsGenODFWidget::readDataFromHDF5(H5ReconStatsReader::Pointer reader,
                                         QVector<double>  &bins,
                                         const std::string &hdf5GroupName)
{
  int err = -1;

  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int StatsGenODFWidget::writeDataToHDF5(H5ReconStatsWriter::Pointer writer)
{
  int err = 0;
  double totalWeight = 0.0;

  QwtArray<double> e1s;
  QwtArray<double> e2s;
  QwtArray<double> e3s;
  QwtArray<double> weights;
  QwtArray<double> sigmas;
  QwtArray<double> odf;

  // Initialize xMax and yMax....
  e1s = m_ODFTableModel->getData(SGODFTableModel::Euler1);
  e2s = m_ODFTableModel->getData(SGODFTableModel::Euler2);
  e3s = m_ODFTableModel->getData(SGODFTableModel::Euler3);
  weights = m_ODFTableModel->getData(SGODFTableModel::Weight);
  sigmas = m_ODFTableModel->getData(SGODFTableModel::Sigma);

  if (m_CrystalStructure == AIM::Reconstruction::Cubic)
  {
    Texture::calculateCubicODFData(e1s, e2s, e3s, weights, sigmas, true, odf, totalWeight);
  }
  else if (m_CrystalStructure == AIM::Reconstruction::Hexagonal)
  {
    Texture::calculateHexODFData(e1s, e2s, e3s, weights, sigmas, true, odf, totalWeight);
  }
  if (odf.size() > 0)
  {
    double* odfPtr = &(odf.front());
    err = -1;
    if (odfPtr != NULL)
    {
      err = writer->writeODFData(m_PhaseIndex, m_CrystalStructure, odfPtr);
    }
  }
  // Write the MDF Data if we have that functionality enabled
  if (m_MDFWidget != NULL)
  {
    m_MDFWidget->writeDataToHDF5(writer);
  }
  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGenODFWidget::enableMDFTab(bool b)
{
  if (NULL != m_MDFWidget)
  {
    m_MDFWidget->deleteLater();
  }
  m_MDFWidget = new StatsGenMDFWidget();
  m_MDFWidget->setODFTableModel(m_ODFTableModel);
  tabWidget->addTab(m_MDFWidget, QString("MDF"));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGenODFWidget::setCrystalStructure(AIM::Reconstruction::CrystalStructure value)
{
  if (m_CrystalStructure != value)
  {
    this->m_CrystalStructure = value;
    switch(value)
    {
      case AIM::Reconstruction::Cubic:
        setPlotTabTitles("<001> PF", "<011> PF", "<111> PF");
        break;
      case AIM::Reconstruction::Hexagonal:
        setPlotTabTitles("<0001> PF", "<11-20> PF", "<10-10> PF");
        break;
      default:
        setPlotTabTitles("Unkown", "Unknown", "Unknown");
    }
    on_m_CalculateODFBtn_clicked();
  }
  if (m_MDFWidget != NULL)
  {
    m_MDFWidget->setCrystalStructure(m_CrystalStructure);
  }

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AIM::Reconstruction::CrystalStructure StatsGenODFWidget::getCrystalStructure()
{
  return m_CrystalStructure;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGenODFWidget::setPhaseIndex(int value)
{
  this->m_PhaseIndex = value;
  if (m_MDFWidget != NULL)
  {
    m_MDFWidget->setPhaseIndex(m_PhaseIndex);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int StatsGenODFWidget::getPhaseIndex()
{
return m_PhaseIndex;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGenODFWidget::setPlotTabTitles(QString t1, QString t2, QString t3)
{
  tabWidget->setTabText(1, t1);
  tabWidget->setTabText(2, t2);
  tabWidget->setTabText(3, t3);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGenODFWidget::setupGui()
{
  // Setup the TableView and Table Models
  QHeaderView* headerView = new QHeaderView(Qt::Horizontal, m_ODFTableView);
  headerView->setResizeMode(QHeaderView::Interactive);
  m_ODFTableView->setHorizontalHeader(headerView);
  headerView->show();

  m_ODFTableModel = new SGODFTableModel;
  m_ODFTableModel->setInitialValues();
  m_ODFTableView->setModel(m_ODFTableModel);
  QAbstractItemDelegate* idelegate = m_ODFTableModel->getItemDelegate();
  m_ODFTableView->setItemDelegate(idelegate);

  initQwtPlot("RD", "TD", m_ODF_001Plot);
  initQwtPlot("RD", "TD", m_ODF_011Plot);
  initQwtPlot("RD", "TD", m_ODF_111Plot);

  m_PlotCurves.push_back(new QwtPlotCurve);
  m_PlotCurves.push_back(new QwtPlotCurve);
  m_PlotCurves.push_back(new QwtPlotCurve);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGenODFWidget::initQwtPlot(QString xAxisName, QString yAxisName, QwtPlot* plot)
{
  plot->setAxisTitle(QwtPlot::xBottom, xAxisName);
  plot->setAxisTitle(QwtPlot::yLeft, yAxisName);
  plot->setCanvasBackground(QColor(Qt::white));

// These set the plot axis to NOT show anything except the axis labels.
  plot->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Backbone, false);
  plot->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Ticks, false);
  plot->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Labels, false);
  plot->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Backbone, false);
  plot->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Ticks, false);
  plot->axisScaleDraw(QwtPlot::xBottom)->enableComponent(QwtAbstractScaleDraw::Labels, false);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGenODFWidget::on_m_CalculateODFBtn_clicked()
{
  int err = 0;
 // std::cout << "StatsGenODFWidget[" << objectName().toStdString() << "]::on_m_CalculateODFBtn_clicked" << std::endl;
  QwtArray<double> x001;
  QwtArray<double> y001;
  QwtArray<double> x011;
  QwtArray<double> y011;
  QwtArray<double> x111;
  QwtArray<double> y111;

  QwtArray<double> e1s;
  QwtArray<double> e2s;
  QwtArray<double> e3s;
  QwtArray<double> weights;
  QwtArray<double> sigmas;
  QwtArray<double> odf;


  e1s = m_ODFTableModel->getData(SGODFTableModel::Euler1);
  e2s = m_ODFTableModel->getData(SGODFTableModel::Euler2);
  e3s = m_ODFTableModel->getData(SGODFTableModel::Euler3);
  weights = m_ODFTableModel->getData(SGODFTableModel::Weight);
  sigmas = m_ODFTableModel->getData(SGODFTableModel::Sigma);

  StatsGen sg;
  int size = 1000;

  if (m_CrystalStructure == AIM::Reconstruction::Cubic) {
    err = sg.GenCubicODFPlotData(e1s, e2s, e3s, weights, sigmas, x001, y001, x011, y011, x111, y111, size);
  }
  else if (m_CrystalStructure == AIM::Reconstruction::Hexagonal) {
    err = sg.GenHexODFPlotData(e1s, e2s, e3s, weights, sigmas, x001, y001, x011, y011, x111, y111, size);
  }
  if (err == 1)
  {
    //TODO: Present Error Message
    return;
  }

  QwtPlotCurve* curve = m_PlotCurves[0];
  curve->setData(x001, y001);
  curve->setStyle(QwtPlotCurve::Dots);
  curve->attach(m_ODF_001Plot);
  m_ODF_001Plot->replot();

  curve = m_PlotCurves[1];
  curve->setData(x011, y011);
  curve->setStyle(QwtPlotCurve::Dots);
  curve->attach(m_ODF_011Plot);
  m_ODF_011Plot->replot();

  curve = m_PlotCurves[2];
  curve->setData(x111, y111);
  curve->setStyle(QwtPlotCurve::Dots);
  curve->attach(m_ODF_111Plot);
  m_ODF_111Plot->replot();

  // Enable the MDF tab
  if (m_MDFWidget != NULL)
  {
    m_MDFWidget->setEnabled(true);
    m_MDFWidget->updateMDFPlots();
  }
}



// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGenODFWidget::on_addODFTextureBtn_clicked()
{
  TextureDialog t(m_CrystalStructure, NULL);
  int r = t.exec();
  if (r == QDialog::Accepted)
  {
    if (!m_ODFTableModel->insertRow(m_ODFTableModel->rowCount())) return;
    // Gather values from the dialog and push them to the Table Model
    double e1 = 0.0;
    double e2 = 0.0;
    double e3 = 0.0;
    double weight = 1.0;
    double sigma = 1.0;

    t.getODFEntry(e1, e2, e3, weight, sigma);
    int row = m_ODFTableModel->rowCount() - 1;
    m_ODFTableModel->setRowData(row, e1, e2, e3, weight, sigma);

    m_ODFTableView->resizeColumnsToContents();
    m_ODFTableView->scrollToBottom();
    m_ODFTableView->setFocus();
    QModelIndex index = m_ODFTableModel->index(m_ODFTableModel->rowCount() - 1, 0);
    m_ODFTableView->setCurrentIndex(index);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void StatsGenODFWidget::on_deleteODFTextureBtn_clicked()
{
  QItemSelectionModel *selectionModel = m_ODFTableView->selectionModel();
  if (!selectionModel->hasSelection()) return;
  QModelIndex index = selectionModel->currentIndex();
  if (!index.isValid()) return;
  m_ODFTableModel->removeRow(index.row(), index.parent());
  if (m_ODFTableModel->rowCount() > 0)
  {
    m_ODFTableView->resizeColumnsToContents();
  }
}

