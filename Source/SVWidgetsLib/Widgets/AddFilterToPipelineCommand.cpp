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

#include "AddFilterToPipelineCommand.h"

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
AddFilterToPipelineCommand::AddFilterToPipelineCommand(AbstractFilter::Pointer filter, FilterPipeline::Pointer pipeline, int insertIndex, PipelineModel* model, QUndoCommand* parent)
: QUndoCommand(parent)
, m_Pipeline(pipeline)
, m_PipelineModel(model)
{
  std::vector<AbstractFilter::Pointer> filters;
  filters.push_back(filter);
  AddFilterToPipelineCommand(filters, pipeline, insertIndex, model, parent);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AddFilterToPipelineCommand::AddFilterToPipelineCommand(std::vector<AbstractFilter::Pointer> filters, FilterPipeline::Pointer pipeline, int insertIndex, PipelineModel* model, QUndoCommand* parent)
: QUndoCommand(parent)
, m_Filters(filters)
, m_Pipeline(pipeline)
, m_PipelineModel(model)
{
  for(size_t i = 0; i < m_Filters.size(); i++)
  {
    m_FilterRows.push_back(insertIndex + i);
  }

  if (pipeline.get() == nullptr)
  {
    insertIndex = 0;
  }
  else if(insertIndex < 0 || insertIndex > pipeline->size())
  {
    insertIndex = pipeline->size();
  }
  m_InsertIndex = insertIndex;

  connect(this, &AddFilterToPipelineCommand::statusMessageGenerated, m_PipelineModel, &PipelineModel::statusMessage);
  connect(this, &AddFilterToPipelineCommand::standardOutputMessageGenerated, m_PipelineModel, &PipelineModel::stdOutMessage);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AddFilterToPipelineCommand::~AddFilterToPipelineCommand() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AddFilterToPipelineCommand::redo()
{
  if(m_Filters.size() <= 0)
  {
    return;
  }

  if (m_Pipeline.get() == nullptr)
  {
    // The pipeline that these filters are being inserted into does not exist.  Creating one now.
    m_Pipeline = FilterPipeline::New();
    m_PipelineModel->addPipeline(m_Pipeline);
    m_CreatedPipeline = true;
  }

  QModelIndex rootIndex = m_PipelineModel->getPipelineRootIndexFromPipeline(m_Pipeline);
  m_PreviousModifiedState = m_PipelineModel->data(rootIndex, PipelineModel::Roles::PipelineModifiedRole).toBool();
  m_PipelineModel->setData(rootIndex, true, PipelineModel::Roles::PipelineModifiedRole);

  m_Pipeline->insert(m_InsertIndex, m_Filters);

  QString statusMessage = getStatusMessage();

  if(m_FirstRun == false)
  {
    statusMessage.prepend("Redo \"");
    statusMessage.append('\"');
  }
  else
  {
    m_FirstRun = false;
  }

  emit statusMessageGenerated(statusMessage);
  emit standardOutputMessageGenerated(statusMessage);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AddFilterToPipelineCommand::undo()
{
  if(m_Filters.size() <= 0 || m_Pipeline.get() == nullptr)
  {
    return;
  }

  int count = m_Filters.size();
  while (count > 0)
  {
    m_Pipeline->erase(m_InsertIndex);
    count--;
  }

  if (m_CreatedPipeline == true)
  {
    m_Pipeline = FilterPipeline::NullPointer();
  }
  else
  {
    QModelIndex rootIndex = m_PipelineModel->getPipelineRootIndexFromPipeline(m_Pipeline);
    m_PipelineModel->setData(rootIndex, m_PreviousModifiedState, PipelineModel::Roles::PipelineModifiedRole);
  }

  QString statusMessage = getStatusMessage();

  statusMessage.prepend("Undo \"");
  statusMessage.append('\"');

  emit statusMessageGenerated(statusMessage);
  emit standardOutputMessageGenerated(statusMessage);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString AddFilterToPipelineCommand::getStatusMessage()
{
  QString statusMessage;
  if(m_Filters.size() > 1)
  {
    statusMessage = QObject::tr(m_MultipleFiltersStatusMessage.toStdString().c_str()).arg(m_Filters.size()).arg(m_FilterRows[0] + 1).arg(m_Pipeline->getName());
  }
  else
  {
    statusMessage = QObject::tr(m_SingleFilterStatusMessage.toStdString().c_str()).arg(m_Filters[0]->getHumanLabel()).arg(m_FilterRows[0] + 1).arg(m_Pipeline->getName());
  }
  return statusMessage;
}
