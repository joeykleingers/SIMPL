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

#pragma once

#include <QtCore/QModelIndex>

#include <QtWidgets/QUndoCommand>

#include "SIMPLib/Common/SIMPLibSetGetMacros.h"
#include "SIMPLib/Filtering/AbstractFilter.h"
#include "SIMPLib/Filtering/FilterPipeline.h"

#include "SVWidgetsLib/SVWidgetsLib.h"

class PipelineModel;

class SVWidgetsLib_EXPORT AddFilterToPipelineCommand : public QObject, public QUndoCommand
{
    Q_OBJECT

public:
  AddFilterToPipelineCommand(AbstractFilter::Pointer filter, FilterPipeline::Pointer pipeline, int insertIndex, PipelineModel* model, QUndoCommand* parent = nullptr);

  AddFilterToPipelineCommand(std::vector<AbstractFilter::Pointer> filters, FilterPipeline::Pointer pipeline, int insertIndex, PipelineModel* model, QUndoCommand* parent = nullptr);

  ~AddFilterToPipelineCommand() override;

  void undo() override;

  void redo() override;

signals:
  void statusMessageGenerated(const QString &msg);
  void standardOutputMessageGenerated(const QString &msg);

private:
  std::vector<AbstractFilter::Pointer> m_Filters;
  FilterPipeline::Pointer m_Pipeline;
  PipelineModel* m_PipelineModel = nullptr;
  std::vector<int> m_FilterRows;
  int m_InsertIndex = -1;
  bool m_FirstRun = true;
  bool m_CreatedPipeline = false;
  bool m_PreviousModifiedState = false;

  const QString m_MultipleFiltersStatusMessage = "Added %1 filters starting at index %2 to pipeline '%3'";
  const QString m_SingleFilterStatusMessage = "Added '%1' filter at index %2 to pipeline '%3'";

  /**
   * @brief getStatusMessage
   * @return
   */
  QString getStatusMessage();

  AddFilterToPipelineCommand(const AddFilterToPipelineCommand&) = delete; // Copy Constructor Not Implemented
  void operator=(const AddFilterToPipelineCommand&) = delete;   // Move assignment Not Implemented
};