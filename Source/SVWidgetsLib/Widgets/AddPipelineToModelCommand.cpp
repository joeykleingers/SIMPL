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

#include "AddPipelineToModelCommand.h"

#include "SVWidgetsLib/Widgets/PipelineModel.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AddPipelineToModelCommand::AddPipelineToModelCommand(FilterPipeline::Pointer pipeline, PipelineModel* model, int insertIndex, QString actionText, QUndoCommand* parent)
: QUndoCommand(parent)
, m_Pipeline(pipeline)
, m_ActionText(actionText)
, m_ViewModel(model)
{
  if (pipeline.get() == nullptr || model == nullptr)
  {
    m_ValidCommand = false;
    return;
  }

  if(insertIndex < 0)
  {
    insertIndex = model->rowCount();
  }
  m_InsertIndex = insertIndex;

  setText(QObject::tr("\"%1 '%2' Pipeline\"").arg(actionText).arg(pipeline->getName()));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AddPipelineToModelCommand::~AddPipelineToModelCommand() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void AddPipelineToModelCommand::redo()
{
  m_ViewModel->insertRow(m_InsertIndex);
  QModelIndex pipelineRootIndex = m_ViewModel->index(m_InsertIndex, PipelineItem::Contents);
  m_ViewModel->setData(pipelineRootIndex, static_cast<int>(PipelineItem::ItemType::PipelineRoot), PipelineModel::Roles::ItemTypeRole);
  m_ViewModel->setPipeline(pipelineRootIndex, m_Pipeline);

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
void AddPipelineToModelCommand::undo()
{
  m_ViewModel->removeRow(m_InsertIndex);

  QString statusMessage = getStatusMessage();

  statusMessage.prepend("Undo \"");
  statusMessage.append('\"');

  emit statusMessageGenerated(statusMessage);
  emit standardOutputMessageGenerated(statusMessage);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString AddPipelineToModelCommand::getStatusMessage()
{
  QString statusMessage = QObject::tr(m_StatusMessage.toStdString().c_str()).arg(m_Pipeline->getName()).arg(m_InsertIndex);
  return statusMessage;
}
