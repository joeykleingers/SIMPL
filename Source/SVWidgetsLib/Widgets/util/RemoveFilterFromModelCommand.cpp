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

#include "RemoveFilterFromModelCommand.h"

#include "SVWidgetsLib/Animations/PipelineItemSlideAnimation.h"
#include "SVWidgetsLib/Widgets/FilterInputWidget.h"
#include "SVWidgetsLib/Widgets/PipelineModel.h"
#include "SVWidgetsLib/Widgets/SVPipelineView.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
RemoveFilterFromModelCommand::RemoveFilterFromModelCommand(AbstractFilter::Pointer filter, PipelineModel* model, QModelIndex pipelineIndex, QString actionText, QUndoCommand* parent)
: QUndoCommand(parent)
, m_PipelineModel(model)
, m_PipelineIndex(pipelineIndex)
{
  if(nullptr == filter || nullptr == model)
  {
    return;
  }

  setText(QObject::tr("\"%1 '%2'\"").arg(actionText).arg(filter->getHumanLabel()));

  m_Filters.push_back(filter);

  QModelIndex index = model->indexOfFilter(filter.get());
  m_FilterRows.push_back(index.row());
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
RemoveFilterFromModelCommand::RemoveFilterFromModelCommand(std::vector<AbstractFilter::Pointer> filters, PipelineModel* model, QModelIndex pipelineIndex, QString actionText, QUndoCommand* parent)
: QUndoCommand(parent)
, m_PipelineModel(model)
, m_PipelineIndex(pipelineIndex)
, m_Filters(filters)
{
  if(nullptr == model)
  {
    return;
  }

  setText(QObject::tr("\"%1 %2 Filters\"").arg(actionText).arg(filters.size()));

  for(size_t i = 0; i < m_Filters.size(); i++)
  {
    QModelIndex index = model->indexOfFilter(m_Filters[i].get());
    m_FilterRows.push_back(index.row());
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
RemoveFilterFromModelCommand::~RemoveFilterFromModelCommand() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RemoveFilterFromModelCommand::undo()
{
  for(size_t i = 0; i < m_FilterRows.size(); i++)
  {
    int insertIndex = m_FilterRows[i];
    AbstractFilter::Pointer filter = m_Filters[i];

    addFilter(filter, insertIndex);
  }

  emit m_PipelineModel->pipelineDataChanged(m_PipelineIndex);

  QString statusMessage;
  if(m_Filters.size() > 1)
  {
    QString indexesString = QObject::tr("%1").arg(m_FilterRows[0] + 1);
    for(size_t i = 1; i < m_FilterRows.size(); i++)
    {
      indexesString.append(", ");
      indexesString.append(QObject::tr("%1").arg(m_FilterRows[i] + 1));
    }
    statusMessage = QObject::tr("Undo \"Removed %1 filters at indexes %2\"").arg(m_Filters.size()).arg(indexesString);
  }
  else
  {
    statusMessage = QObject::tr("Undo \"Removed '%1' filter at index %2\"").arg(m_Filters[0]->getHumanLabel()).arg(m_FilterRows[0] + 1);
  }

  emit m_PipelineModel->statusMessageGenerated(statusMessage);
  emit m_PipelineModel->standardOutputMessageGenerated(statusMessage);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool variantCompare(const QVariant& v1, const QVariant& v2)
{
  return v1.toInt() > v2.toInt();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RemoveFilterFromModelCommand::redo()
{
  for(size_t i = 0; i < m_Filters.size(); i++)
  {
    removeFilter(m_Filters[i]);
  }

  QString statusMessage;
  if(m_Filters.size() > 1)
  {
    QString indexesString = QObject::tr("%1").arg(m_FilterRows[0] + 1);
    for(size_t i = 1; i < m_FilterRows.size(); i++)
    {
      indexesString.append(", ");
      indexesString.append(QObject::tr("%1").arg(m_FilterRows[i] + 1));
    }
    statusMessage = QObject::tr("Removed %1 filters at indexes %2").arg(m_Filters.size()).arg(indexesString);
  }
  else
  {
    statusMessage = QObject::tr("Removed '%1' filter at index %2").arg(m_Filters[0]->getHumanLabel()).arg(m_FilterRows[0] + 1);
  }

  if(m_FirstRun == false)
  {
    statusMessage.prepend("Redo \"");
    statusMessage.append('\"');
  }
  else
  {
    m_FirstRun = false;
  }

  emit m_PipelineModel->pipelineDataChanged(m_PipelineIndex);

  emit m_PipelineModel->statusMessageGenerated(statusMessage);
  emit m_PipelineModel->standardOutputMessageGenerated(statusMessage);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RemoveFilterFromModelCommand::addFilter(AbstractFilter::Pointer filter, int insertionIndex)
{
  m_PipelineModel->insertRow(insertionIndex);
  QModelIndex filterIndex = m_PipelineModel->index(insertionIndex, PipelineItem::Contents);
  m_PipelineModel->setData(filterIndex, static_cast<int>(PipelineItem::ItemType::Filter), PipelineModel::ItemTypeRole);
  m_PipelineModel->setFilter(filterIndex, filter);

  connectFilterSignalsSlots(filter);

  if(filter->getEnabled() == false)
  {
    m_PipelineModel->setData(filterIndex, static_cast<int>(PipelineItem::WidgetState::Disabled), PipelineModel::WidgetStateRole);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RemoveFilterFromModelCommand::removeFilter(AbstractFilter::Pointer filter)
{
  disconnectFilterSignalsSlots(filter);

  QModelIndex index = m_PipelineModel->indexOfFilter(filter.get());

  QPersistentModelIndex persistentIndex = index;

  m_PipelineModel->removeRow(persistentIndex.row());
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RemoveFilterFromModelCommand::connectFilterSignalsSlots(AbstractFilter::Pointer filter)
{
  QModelIndex index = m_PipelineModel->indexOfFilter(filter.get());

  QObject::connect(filter.get(), SIGNAL(filterCompleted(AbstractFilter*)), m_PipelineModel, SLOT(listenFilterCompleted(AbstractFilter*)));

  QObject::connect(filter.get(), SIGNAL(filterInProgress(AbstractFilter*)), m_PipelineModel, SLOT(listenFilterInProgress(AbstractFilter*)));

  FilterInputWidget* fiw = m_PipelineModel->filterInputWidget(index);

  QObject::connect(fiw, &FilterInputWidget::filterParametersChanged, [=] {
    m_PipelineModel->preflightTriggered(m_PipelineIndex, m_PipelineModel);
    emit m_PipelineModel->filterParametersChanged(filter);
  });
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RemoveFilterFromModelCommand::disconnectFilterSignalsSlots(AbstractFilter::Pointer filter)
{
  QModelIndex index = m_PipelineModel->indexOfFilter(filter.get());

  QObject::disconnect(filter.get(), &AbstractFilter::filterCompleted, 0, 0);

  QObject::disconnect(filter.get(), &AbstractFilter::filterInProgress, 0, 0);

  FilterInputWidget* fiw = m_PipelineModel->filterInputWidget(index);

  QObject::disconnect(fiw, &FilterInputWidget::filterParametersChanged, 0, 0);
}
