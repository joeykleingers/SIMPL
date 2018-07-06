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

#include "RemoveFilterFromPipelineCommand.h"

#include "SVWidgetsLib/Animations/PipelineItemSlideAnimation.h"
#include "SVWidgetsLib/Widgets/FilterInputWidget.h"
#include "SVWidgetsLib/Widgets/PipelineModel.h"
#include "SVWidgetsLib/Widgets/SVPipelineView.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
RemoveFilterFromPipelineCommand::RemoveFilterFromPipelineCommand(AbstractFilter::Pointer filter, FilterPipeline::Pointer pipeline, QString actionText, QUndoCommand* parent)
: QUndoCommand(parent)
, m_Pipeline(pipeline)
{
  if (filter.get() == nullptr || pipeline.get() == nullptr)
  {
    m_ValidCommand = false;
    return;
  }

  setText(QObject::tr("\"%1 '%2' to pipeline '%3'\"").arg(actionText).arg(filter->getHumanLabel()));

  m_Filters.push_back(filter);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
RemoveFilterFromPipelineCommand::RemoveFilterFromPipelineCommand(std::vector<AbstractFilter::Pointer> filters, FilterPipeline::Pointer pipeline, QString actionText, QUndoCommand* parent)
: QUndoCommand(parent)
, m_Pipeline(pipeline)
, m_Filters(filters)
{
  if (pipeline.get() == nullptr || filters.size() <= 0)
  {
    m_ValidCommand = false;
    return;
  }

  for (int i = 0; i < filters.size(); i++)
  {
    if (m_Filters[i].get() == nullptr)
    {
      m_ValidCommand = false;
      return;
    }
  }

  FilterPipeline::FilterContainerType container = m_Pipeline->getFilterContainer();
  for (int i = 0; i < m_Filters.size(); i++)
  {
    AbstractFilter::Pointer filter = m_Filters[i];
    int index = container.indexOf(filter);
    m_RemovalRows.push_back(index);
  }

  setText(QObject::tr("\"%1 %2 Filters\"").arg(actionText).arg(filters.size()));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
RemoveFilterFromPipelineCommand::~RemoveFilterFromPipelineCommand() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RemoveFilterFromPipelineCommand::redo()
{
  if(m_Filters.size() <= 0 || m_Pipeline.get() == nullptr)
  {
    return;
  }

  for (int i = 0; i < m_Filters.size(); i++)
  {
    AbstractFilter::Pointer filter = m_Filters[i];
    m_Pipeline->erase(m_RemovalRows[i]);
  }

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

  emit pipelineChanged(m_Pipeline);

  emit statusMessageGenerated(statusMessage);
  emit standardOutputMessageGenerated(statusMessage);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RemoveFilterFromPipelineCommand::undo()
{
  if(m_Filters.size() <= 0 || m_Pipeline.get() == nullptr)
  {
    return;
  }

  for (int i = 0; i < m_Filters.size(); i++)
  {
    AbstractFilter::Pointer filter = m_Filters[i];
    m_Pipeline->insert(m_RemovalRows[i], filter);
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
QString RemoveFilterFromPipelineCommand::getStatusMessage()
{
  QString statusMessage;
  if(m_Filters.size() > 1)
  {
    statusMessage = QObject::tr(m_MultipleFiltersStatusMsg.toStdString().c_str()).arg(m_Filters.size()).arg(m_RemovalRows[0] + 1).arg(m_Pipeline->getName());
  }
  else
  {
    statusMessage = QObject::tr(m_SingleFilterStatusMsg.toStdString().c_str()).arg(m_Filters[0]->getHumanLabel()).arg(m_RemovalRows[0] + 1).arg(m_Pipeline->getName());
  }
  return statusMessage;
}
