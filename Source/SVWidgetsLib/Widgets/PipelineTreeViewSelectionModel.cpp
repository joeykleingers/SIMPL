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

#include "PipelineTreeViewSelectionModel.h"

#include "SVWidgetsLib/Widgets/PipelineModel.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PipelineTreeViewSelectionModel::PipelineTreeViewSelectionModel(PipelineModel* model)
: QItemSelectionModel(model)
, m_PipelineModel(model)
{
  
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PipelineTreeViewSelectionModel::~PipelineTreeViewSelectionModel()
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineTreeViewSelectionModel::select(const QModelIndex &index, QItemSelectionModel::SelectionFlags command)
{
  QItemSelection selection(index, index);
  select(selection, command);
}

// -----------------------------------------------------------------------------
// This slot is overridden so that it is not possible to select filter indexes and pipeline root indexes at the same time
// -----------------------------------------------------------------------------
void PipelineTreeViewSelectionModel::select(const QItemSelection &selection, QItemSelectionModel::SelectionFlags command)
{
  // Run the superclass's select slot
  QItemSelectionModel::select(selection, command);

  bool hasFilters = false;

  QModelIndexList currentSelections = selectedRows();
  for (int i = 0; i < currentSelections.size(); i++)
  {
    QModelIndex index = currentSelections[i];
    PipelineItem::ItemType itemType = static_cast<PipelineItem::ItemType>(m_PipelineModel->data(index, PipelineModel::Roles::ItemTypeRole).toInt());
    if (itemType == PipelineItem::ItemType::Filter)
    {
      hasFilters = true;
    }
  }

  QItemSelection deselectList;
  for (int i = 0; i < currentSelections.size(); i++)
  {
    QModelIndex index = currentSelections[i];
    PipelineItem::ItemType itemType = static_cast<PipelineItem::ItemType>(m_PipelineModel->data(index, PipelineModel::Roles::ItemTypeRole).toInt());
    // If filters are selected and this is a pipeline root index, add it to the deselectList
    if (hasFilters && itemType == PipelineItem::ItemType::PipelineRoot)
    {
      deselectList.append(QItemSelectionRange(index, index));
    }
  }

  QItemSelectionModel::select(deselectList, QItemSelectionModel::Deselect);
}
