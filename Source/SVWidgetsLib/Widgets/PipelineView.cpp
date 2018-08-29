/* ============================================================================
* Copyright (c) 2009-2016 BlueQuartz Software, LLC
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
* Neither the name of BlueQuartz Software, the US Air Force, nor the names of its
* contributors may be used to endorse or promote products derived from this software
* without specific prior written permission.
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
*
* The code contained herein was partially funded by the followig contracts:
*    United States Air Force Prime Contract FA8650-07-D-5800
*    United States Air Force Prime Contract FA8650-10-D-5210
*    United States Prime Contract Navy N00173-07-C-2068
*
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#include "PipelineView.h"

#include <QtCore/QJsonDocument>

#include <QtGui/QClipboard>

#include <QtWidgets/QApplication>
#include <QtWidgets/QMenu>

#include "SIMPLib/Common/DocRequestManager.h"
#include "SIMPLib/FilterParameters/JsonFilterParametersReader.h"
#include "SIMPLib/FilterParameters/JsonFilterParametersWriter.h"

#include "SVWidgetsLib/Core/SVWidgetsLibConstants.h"
#include "SVWidgetsLib/Widgets/PipelineModel.h"
#include "SVWidgetsLib/Widgets/PipelineViewController.h"
#include "SVWidgetsLib/Widgets/IssuesWidget.h"
#include "SVWidgetsLib/Widgets/StandardOutputWidget.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PipelineView::PipelineView()
: m_PipelineViewController(new PipelineViewController())
, m_PipelineState(PipelineViewState::Idle)
{
  setupGui();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PipelineView::~PipelineView()
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineView::setupGui()
{
  m_PipelineViewController->setPipelineView(this);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineView::addPipelineMessageObserver(QObject* pipelineMessageObserver)
{
  if(m_PipelineViewController)
  {
    m_PipelineViewController->addPipelineMessageObserver(pipelineMessageObserver);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool PipelineView::arePipelinesRunning()
{
  PipelineModel* pipelineModel = getPipelineModel();
  if (!pipelineModel)
  {
    return false;
  }

  for (int i = 0; i < pipelineModel->rowCount(); i++)
  {
    QModelIndex pipelineRootIndex = pipelineModel->index(i, PipelineItem::Contents);
    PipelineItem::ItemType itemType = static_cast<PipelineItem::ItemType>(pipelineModel->data(pipelineRootIndex, PipelineModel::Roles::ItemTypeRole).toInt());
    if (itemType != PipelineItem::ItemType::PipelineRoot)
    {
      continue;
    }

    FilterPipeline::Pointer pipeline = pipelineModel->tempPipeline(pipelineRootIndex);
    if (pipeline->isExecuting())
    {
      return true;
    }
  }

  return false;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineView::addFilterFromClassName(const QString& filterClassName, int insertIndex)
{
  PipelineModel* model = getPipelineModel();
  if (model)
  {
    addFilterFromClassName(filterClassName, model->getActivePipeline(), insertIndex);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineView::addFilterFromClassName(const QString& filterClassName, const QModelIndex &pipelineRootIndex, int insertIndex)
{
  if(m_PipelineViewController)
  {
    m_PipelineViewController->addFilterFromClassName(filterClassName, pipelineRootIndex, insertIndex);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineView::addFilter(AbstractFilter::Pointer filter, int insertIndex)
{
  PipelineModel* model = getPipelineModel();
  if (model)
  {
    addFilter(filter, model->getActivePipeline(), insertIndex);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineView::addFilter(AbstractFilter::Pointer filter, const QModelIndex &pipelineRootIndex, int insertIndex)
{
  if(m_PipelineViewController)
  {
    m_PipelineViewController->addFilter(filter, pipelineRootIndex, insertIndex);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineView::addFilters(std::vector<AbstractFilter::Pointer> filters, int insertIndex)
{
  PipelineModel* model = getPipelineModel();
  if (model)
  {
    addFilters(filters, model->getActivePipeline(), insertIndex);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineView::addFilters(std::vector<AbstractFilter::Pointer> filters, const QModelIndex &pipelineRootIndex, int insertIndex)
{
  if(m_PipelineViewController)
  {
    m_PipelineViewController->addFilters(filters, pipelineRootIndex, insertIndex);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineView::addPipeline(FilterPipeline::Pointer pipeline, int insertIndex)
{
  if(m_PipelineViewController)
  {
    m_PipelineViewController->addPipeline(pipeline, insertIndex);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineView::removeFilter(AbstractFilter::Pointer filter)
{
  PipelineModel* model = getPipelineModel();
  if (model)
  {
    removeFilter(filter, model->getActivePipeline());
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineView::removeFilter(AbstractFilter::Pointer filter, const QModelIndex &pipelineRootIndex)
{
  if(m_PipelineViewController)
  {
    m_PipelineViewController->removeFilter(filter, pipelineRootIndex);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineView::removeFilters(std::vector<AbstractFilter::Pointer> filters)
{
  PipelineModel* model = getPipelineModel();
  if (model)
  {
    removeFilters(filters, model->getActivePipeline());
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineView::removeFilters(std::vector<AbstractFilter::Pointer> filters, const QModelIndex &pipelineRootIndex)
{
  if (m_PipelineViewController)
  {
    m_PipelineViewController->removeFilters(filters, pipelineRootIndex);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineView::removePipeline()
{
  PipelineModel* model = getPipelineModel();
  if (model)
  {
    removePipeline(model->getActivePipeline());
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineView::removePipeline(const QModelIndex &pipelineRootIndex)
{
  if(m_PipelineViewController)
  {
    PipelineModel* pipelineModel = getPipelineModel();
    FilterPipeline::Pointer pipeline = pipelineModel->tempPipeline(pipelineRootIndex);
    m_PipelineViewController->removePipeline(pipeline);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineView::removePipeline(FilterPipeline::Pointer pipeline)
{
  if(m_PipelineViewController)
  {
    m_PipelineViewController->removePipeline(pipeline);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineView::cutFilter(AbstractFilter::Pointer filter)
{
  PipelineModel* model = getPipelineModel();
  if (model)
  {
    cutFilter(filter, model->getActivePipeline());
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineView::cutFilter(AbstractFilter::Pointer filter, const QModelIndex &pipelineRootIndex)
{
  if(m_PipelineViewController)
  {
    m_PipelineViewController->cutFilter(filter, pipelineRootIndex);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineView::cutFilters(std::vector<AbstractFilter::Pointer> filters)
{
  PipelineModel* model = getPipelineModel();
  if (model)
  {
    cutFilters(filters, model->getActivePipeline());
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineView::cutPipeline(FilterPipeline::Pointer pipeline)
{
  if(m_PipelineViewController)
  {
    m_PipelineViewController->cutPipeline(pipeline);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineView::cutPipeline(const QModelIndex &pipelineRootIndex)
{
  if(m_PipelineViewController)
  {
    m_PipelineViewController->cutPipeline(pipelineRootIndex);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineView::cutFilters(std::vector<AbstractFilter::Pointer> filters, const QModelIndex &pipelineRootIndex)
{
  if(m_PipelineViewController)
  {
    m_PipelineViewController->cutFilters(filters, pipelineRootIndex);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineView::pasteFilters(std::vector<AbstractFilter::Pointer> filters, int insertIndex)
{
  PipelineModel* model = getPipelineModel();
  if (model)
  {
    pasteFilters(filters, model->getActivePipeline(), insertIndex);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineView::pasteFilters(std::vector<AbstractFilter::Pointer> filters, const QModelIndex &pipelineRootIndex, int insertIndex)
{
  if(m_PipelineViewController)
  {
    m_PipelineViewController->pasteFilters(filters, pipelineRootIndex, insertIndex);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineView::pastePipeline(FilterPipeline::Pointer pipeline, int insertIndex)
{
  if(m_PipelineViewController)
  {
    m_PipelineViewController->pastePipeline(pipeline, insertIndex);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool PipelineView::savePipeline()
{
  PipelineModel* model = getPipelineModel();
  if (model)
  {
    return savePipeline(model->getActivePipeline());
  }
  return false;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool PipelineView::savePipeline(const QModelIndex &pipelineRootIndex)
{
  if(m_PipelineViewController)
  {
    return m_PipelineViewController->savePipeline(pipelineRootIndex);
  }

  return false;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool PipelineView::savePipelineAs()
{
  PipelineModel* model = getPipelineModel();
  if (model)
  {
    return savePipelineAs(model->getActivePipeline());
  }
  return false;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool PipelineView::savePipelineAs(const QModelIndex &pipelineRootIndex)
{
  if(m_PipelineViewController)
  {
    return m_PipelineViewController->savePipelineAs(pipelineRootIndex);
  }

  return false;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineView::preflightPipeline()
{
  PipelineModel* model = getPipelineModel();
  if (model)
  {
    preflightPipeline(model->getActivePipeline());
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineView::preflightPipeline(const QModelIndex &pipelineRootIndex)
{
  if(m_PipelineViewController)
  {
    m_PipelineViewController->preflightPipeline(pipelineRootIndex);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineView::executePipeline()
{
  PipelineModel* model = getPipelineModel();
  if (model)
  {
    executePipeline(model->getActivePipeline());
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineView::executePipeline(const QModelIndex &pipelineRootIndex)
{
  if(m_PipelineViewController)
  {
    m_PipelineViewController->executePipeline(pipelineRootIndex);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineView::cancelPipeline()
{
  PipelineModel* model = getPipelineModel();
  if (model)
  {
    cancelPipeline(model->getActivePipeline());
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineView::cancelPipeline(const QModelIndex &pipelineRootIndex)
{
  if(m_PipelineViewController)
  {
    m_PipelineViewController->cancelPipeline(pipelineRootIndex);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineView::clearPipeline()
{
  PipelineModel* model = getPipelineModel();
  if (model)
  {
    clearPipeline(model->getActivePipeline());
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineView::clearPipeline(const QModelIndex &pipelineRootIndex)
{
  if(m_PipelineViewController)
  {
    m_PipelineViewController->clearPipeline(pipelineRootIndex);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int PipelineView::writePipeline(const QModelIndex& pipelineRootIndex, const QString& outputPath)
{
  if(m_PipelineViewController)
  {
    return m_PipelineViewController->writePipeline(pipelineRootIndex, outputPath);
  }

  return -1;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineView::copySelection()
{
  if(m_PipelineViewController)
  {
    return m_PipelineViewController->copySelection();
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QAction* PipelineView::getActionUndo()
{
  if (m_PipelineViewController)
  {
    return m_PipelineViewController->getActionUndo();
  }
  return nullptr;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QAction* PipelineView::getActionRedo()
{
  if (m_PipelineViewController)
  {
    return m_PipelineViewController->getActionRedo();
  }
  return nullptr;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
std::vector<AbstractFilter::Pointer> PipelineView::getSelectedFilters()
{
  QModelIndexList selectedIndexes = getSelectedRows();
  qSort(selectedIndexes);

  std::vector<AbstractFilter::Pointer> filters;
  PipelineModel* model = getPipelineModel();
  for(int i = 0; i < selectedIndexes.size(); i++)
  {
    QModelIndex selectedIndex = selectedIndexes[i];
    PipelineItem::ItemType itemType = static_cast<PipelineItem::ItemType>(model->data(selectedIndex, PipelineModel::Roles::ItemTypeRole).toInt());
    if (itemType != PipelineItem::ItemType::Filter)
    {
      continue;
    }

    AbstractFilter::Pointer filter = model->filter(selectedIndex);
    filters.push_back(filter);
  }

  return filters;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PipelineModel* PipelineView::getPipelineModel()
{
  if (m_PipelineViewController)
  {
    return m_PipelineViewController->getPipelineModel();
  }
  return nullptr;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineView::setSelectedFiltersEnabled()
{
  if (m_PipelineViewController)
  {
    QModelIndexList indexes = getSelectedRows();
    qSort(indexes);
    m_PipelineViewController->setFiltersEnabled(indexes);
  }
}
