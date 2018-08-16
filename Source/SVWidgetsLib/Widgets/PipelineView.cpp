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
PipelineView::PipelineView() :
  m_PipelineViewController(new PipelineViewController())
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
void PipelineView::addFilterFromClassName(const QString& filterClassName, int insertIndex)
{
  addFilterFromClassName(filterClassName, m_ActivePipelineIndex, insertIndex);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineView::addFilterFromClassName(const QString& filterClassName, const QModelIndex &pipelineRootIndex, int insertIndex)
{
  if(m_PipelineViewController)
  {
    m_PipelineViewController->addFilterFromClassName(filterClassName, insertIndex, pipelineRootIndex);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineView::addFilter(AbstractFilter::Pointer filter, int insertIndex)
{
  addFilter(filter, m_ActivePipelineIndex, insertIndex);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineView::addFilter(AbstractFilter::Pointer filter, const QModelIndex &pipelineRootIndex, int insertIndex)
{
  if(m_PipelineViewController)
  {
    m_PipelineViewController->addFilter(filter, insertIndex, pipelineRootIndex);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineView::addFilters(std::vector<AbstractFilter::Pointer> filters, int insertIndex)
{
  addFilters(filters, m_ActivePipelineIndex, insertIndex);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineView::addFilters(std::vector<AbstractFilter::Pointer> filters, const QModelIndex &pipelineRootIndex, int insertIndex)
{
  if(m_PipelineViewController)
  {
    m_PipelineViewController->addFilters(filters, insertIndex, pipelineRootIndex);
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
  removeFilter(filter, m_ActivePipelineIndex);
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
  removeFilters(filters, m_ActivePipelineIndex);
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
  removePipeline(m_ActivePipelineIndex);
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
  cutFilter(filter, m_ActivePipelineIndex);
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
  cutFilters(filters, m_ActivePipelineIndex);
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
void PipelineView::pasteFilters(int insertIndex)
{
  pasteFilters(m_ActivePipelineIndex, insertIndex);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineView::pasteFilters(const QModelIndex &pipelineRootIndex, int insertIndex)
{
  if(m_PipelineViewController)
  {
    m_PipelineViewController->pasteFilters(insertIndex, pipelineRootIndex);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineView::preflightPipeline()
{
  preflightPipeline(m_ActivePipelineIndex);
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
  executePipeline(m_ActivePipelineIndex);
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
  cancelPipeline(m_ActivePipelineIndex);
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
  clearPipeline(m_ActivePipelineIndex);
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
void PipelineView::copySelectedFilters()
{
  FilterPipeline::Pointer pipeline = FilterPipeline::New();
  std::vector<AbstractFilter::Pointer> filters = getSelectedFilters();
  for(int i = 0; i < filters.size(); i++)
  {
    pipeline->pushBack(filters[i]);
  }

  JsonFilterParametersWriter::Pointer jsonWriter = JsonFilterParametersWriter::New();
  QString jsonString = jsonWriter->writePipelineToString(pipeline, "Pipeline");

  QClipboard* clipboard = QApplication::clipboard();
  clipboard->setText(jsonString);
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
    AbstractFilter::Pointer filter = model->filter(selectedIndexes[i]);
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
