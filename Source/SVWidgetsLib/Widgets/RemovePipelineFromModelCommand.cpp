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

#include "RemovePipelineFromModelCommand.h"

#include "SVWidgetsLib/Animations/PipelineItemSlideAnimation.h"
#include "SVWidgetsLib/Widgets/FilterInputWidget.h"
#include "SVWidgetsLib/Widgets/PipelineModel.h"
#include "SVWidgetsLib/Widgets/SVPipelineView.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
RemovePipelineFromModelCommand::RemovePipelineFromModelCommand(PipelineModel* model, const QModelIndex &pipelineRootIndex, QString actionText, QUndoCommand* parent)
: QUndoCommand(parent)
, m_PipelineModel(model)
, m_PipelineRootIndex(pipelineRootIndex)
{
  if (model == nullptr)
  {
    m_ValidCommand = false;
    return;
  }

  m_Pipeline = model->tempPipeline(pipelineRootIndex);
  if (m_Pipeline.get() == nullptr)
  {
    m_ValidCommand = false;
    return;
  }

  setText(QObject::tr("\"%1 '%2' Pipeline\"").arg(actionText).arg(m_Pipeline->getName()));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
RemovePipelineFromModelCommand::~RemovePipelineFromModelCommand() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RemovePipelineFromModelCommand::redo()
{
  m_RemovalRow = m_PipelineRootIndex.row();
  m_PipelineModel->removeRow(m_RemovalRow);

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
void RemovePipelineFromModelCommand::undo()
{
  m_PipelineModel->insertRow(m_RemovalRow);
  QModelIndex pipelineRootIndex = m_PipelineModel->index(m_RemovalRow, PipelineItem::Contents);
  m_PipelineModel->setData(pipelineRootIndex, static_cast<int>(PipelineItem::ItemType::PipelineRoot), PipelineModel::Roles::ItemTypeRole);
  m_PipelineModel->setPipeline(pipelineRootIndex, m_Pipeline);

  QString statusMessage = getStatusMessage();

  statusMessage.prepend("Undo \"");
  statusMessage.append('\"');

  emit statusMessageGenerated(statusMessage);
  emit standardOutputMessageGenerated(statusMessage);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString RemovePipelineFromModelCommand::getStatusMessage()
{
  QString statusMessage = QObject::tr(m_StatusMessage.toStdString().c_str()).arg(m_Pipeline->getName()).arg(m_RemovalRow);
  return statusMessage;
}
