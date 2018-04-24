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

#ifndef _movefiltercommand_h_
#define _movefiltercommand_h_

#include <QtCore/QVariant>

#include <QtWidgets/QUndoCommand>

#include <SIMPLib/Filtering/AbstractFilter.h>

#include "SVWidgetsLib/SVWidgetsLib.h"

class SVPipelineView;
class PipelineModel;

/**
 * @brief The MoveFilterCommand class
 */
class SVWidgetsLib_EXPORT MoveFilterCommand : public QUndoCommand
{
public:

    /**
   * @brief MoveFilterCommand
   * @param filter
   * @param originView
   * @param destinationView
   * @param destinationIndex
   * @param parent
   */
  MoveFilterCommand(AbstractFilter::Pointer filter, SVPipelineView* originView, SVPipelineView* destinationView, int destinationIndex = -1, QUndoCommand* parent = 0);

  /**
   * @brief MoveFilterCommand
   * @param filters
   * @param originView
   * @param destinationView
   * @param destinationIndex
   * @param parent
   */
  MoveFilterCommand(std::vector<AbstractFilter::Pointer> filters, SVPipelineView* originView, SVPipelineView* destinationView, int destinationIndex = -1, QUndoCommand* parent = 0);

  virtual ~MoveFilterCommand();

  /**
   * @brief undo
   */
  virtual void undo();

  /**
   * @brief redo
   */
  virtual void redo();

private:
  std::vector<AbstractFilter::Pointer> m_Filters;
  SVPipelineView* m_OriginView;
  SVPipelineView* m_DestinationView;
  size_t m_OriginIndex;
  size_t m_DestinationIndex;
  std::vector<size_t> originIndexes;

  /**
   * @brief addFilter
   * @param filter
   * @param model
   * @param insertionIndex
   */
  void addFilter(AbstractFilter::Pointer filter, PipelineModel* model, size_t insertionIndex);

  /**
   * @brief removeFilter
   * @param filterIndex
   * @param model
   */
  void removeFilter(int filterIndex, PipelineModel* model);

  MoveFilterCommand(const MoveFilterCommand&) = delete; // Copy Constructor Not Implemented
  void operator=(const MoveFilterCommand&);             // Move assignment Not Implemented
};

#endif /* _movefiltercommand_h_ */
