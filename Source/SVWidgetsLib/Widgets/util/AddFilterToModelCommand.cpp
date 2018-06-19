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

#include "AddFilterToModelCommand.h"

#include <QtCore/QJsonArray>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonParseError>
#include <QtCore/QObject>

#include "SIMPLib/CoreFilters/Breakpoint.h"
#include "SIMPLib/FilterParameters/JsonFilterParametersReader.h"
#include "SIMPLib/FilterParameters/JsonFilterParametersWriter.h"

#include "SVWidgetsLib/Animations/PipelineItemHeightAnimation.h"
#include "SVWidgetsLib/Animations/PipelineItemSlideAnimation.h"
#include "SVWidgetsLib/Widgets/FilterInputWidget.h"
#include "SVWidgetsLib/Widgets/PipelineModel.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AddFilterToModelCommand::AddFilterToModelCommand(AbstractFilter::Pointer filter, PipelineModel* model, int insertIndex, QModelIndex pipelineIndex, QString actionText, QUndoCommand* parent)
: QUndoCommand(parent)
, m_ActionText(actionText)
, m_PipelineModel(model)
, m_PipelineIndex(pipelineIndex)
{
  if(insertIndex < 0)
  {
    insertIndex = model->rowCount();
  }

  setText(QObject::tr("\"%1 '%2'\"").arg(actionText).arg(filter->getHumanLabel()));

  m_Filters.push_back(filter);

  m_FilterRows.push_back(insertIndex);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AddFilterToModelCommand::AddFilterToModelCommand(std::vector<AbstractFilter::Pointer> filters, PipelineModel* model, int insertIndex, QModelIndex pipelineIndex, QString actionText, QUndoCommand* parent)
: QUndoCommand(parent)
, m_Filters(filters)
, m_ActionText(actionText)
, m_PipelineModel(model)
, m_PipelineIndex(pipelineIndex)
{
  if(insertIndex < 0)
  {
    insertIndex = model->rowCount();
  }

  if(filters.size() == 1)
  {
    setText(QObject::tr("\"%1 '%2'\"").arg(actionText).arg(filters[0]->getHumanLabel()));
  }
  else
  {
    setText(QObject::tr("\"%1 %2 Filters\"").arg(actionText).arg(filters.size()));
  }

  for(size_t i = 0; i < m_Filters.size(); i++)
  {
    m_FilterRows.push_back(insertIndex + i);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AddFilterToModelCommand::~AddFilterToModelCommand() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AddFilterToModelCommand::undo()
{
  if(m_Filters.size() <= 0)
  {
    return;
  }

  for(int i = 0; i < m_Filters.size(); i++)
  {
    QPersistentModelIndex filterIndex = m_PipelineModel->indexOfFilter(m_Filters[i].get());

    removeFilter(filterIndex);
  }

  QString statusMessage;
  if(m_Filters.size() > 1)
  {
    statusMessage = QObject::tr("Undo \"Added %1 filters starting at index %2\"").arg(m_Filters.size()).arg(m_FilterRows[0] + 1);
  }
  else
  {
    statusMessage = QObject::tr("Undo \"Added '%1' filter at index %2\"").arg(m_Filters[0]->getHumanLabel()).arg(m_FilterRows[0] + 1);
  }

  emit m_PipelineModel->pipelineDataChanged(m_PipelineIndex);

  emit m_PipelineModel->statusMessageGenerated(statusMessage);
  emit m_PipelineModel->standardOutputMessageGenerated(statusMessage);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AddFilterToModelCommand::redo()
{
  if(m_Filters.size() <= 0)
  {
    return;
  }

  for(size_t i = 0; i < m_Filters.size(); i++)
  {
    addFilter(m_Filters[i], m_FilterRows[i]);
  }

  emit m_PipelineModel->pipelineDataChanged(m_PipelineIndex);

  QString statusMessage;
  if(m_Filters.size() > 1)
  {
    statusMessage = QObject::tr("Added %1 filters starting at index %2").arg(m_Filters.size()).arg(m_FilterRows[0] + 1);
  }
  else
  {
    statusMessage = QObject::tr("Added '%1' filter at index %2").arg(m_Filters[0]->getHumanLabel()).arg(m_FilterRows[0] + 1);
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

  emit m_PipelineModel->statusMessageGenerated(statusMessage);
  emit m_PipelineModel->standardOutputMessageGenerated(statusMessage);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AddFilterToModelCommand::addFilter(AbstractFilter::Pointer filter, int insertionIndex)
{
  m_PipelineModel->insertRow(insertionIndex);

  QModelIndex filterIndex = m_PipelineModel->index(insertionIndex, PipelineItem::Contents);
  //  model->setData(filterIndex, filter->getHumanLabel(), Qt::DisplayRole);
  m_PipelineModel->setData(filterIndex, static_cast<int>(PipelineItem::ItemType::Filter), PipelineModel::ItemTypeRole);
  m_PipelineModel->setFilter(filterIndex, filter);

  connectFilterSignalsSlots(filter);

  if(filter->getEnabled() == false)
  {
    m_PipelineModel->setData(filterIndex, static_cast<int>(PipelineItem::WidgetState::Disabled), PipelineModel::WidgetStateRole);
  }

  if(m_FirstRun == true)
  {
    QSize size = m_PipelineModel->data(filterIndex, Qt::SizeHintRole).toSize();
    m_PipelineModel->setData(filterIndex, size.height(), PipelineModel::Roles::HeightRole);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AddFilterToModelCommand::removeFilter(const QPersistentModelIndex& index)
{
  AbstractFilter::Pointer filter = m_PipelineModel->filter(index);

  disconnectFilterSignalsSlots(filter);

  m_PipelineModel->removeRow(index.row());
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AddFilterToModelCommand::connectFilterSignalsSlots(AbstractFilter::Pointer filter)
{
  QModelIndex index = m_PipelineModel->indexOfFilter(filter.get());

  QObject::connect(filter.get(), SIGNAL(filterCompleted(AbstractFilter*)), m_PipelineModel, SLOT(listenFilterCompleted(AbstractFilter*)));

  QObject::connect(filter.get(), SIGNAL(filterInProgress(AbstractFilter*)), m_PipelineModel, SLOT(listenFilterInProgress(AbstractFilter*)));

  FilterInputWidget* fiw = m_PipelineModel->filterInputWidget(index);

  QObject::connect(fiw, &FilterInputWidget::filterParametersChanged, [=] {
    emit m_PipelineModel->preflightTriggered(m_PipelineIndex, m_PipelineModel);
    emit m_PipelineModel->filterParametersChanged(filter);
  });
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AddFilterToModelCommand::disconnectFilterSignalsSlots(AbstractFilter::Pointer filter)
{
  QModelIndex index = m_PipelineModel->indexOfFilter(filter.get());

  QObject::disconnect(filter.get(), &AbstractFilter::filterCompleted, 0, 0);

  QObject::disconnect(filter.get(), &AbstractFilter::filterInProgress, 0, 0);

  FilterInputWidget* fiw = m_PipelineModel->filterInputWidget(index);

  QObject::disconnect(fiw, &FilterInputWidget::filterParametersChanged, 0, 0);
}
