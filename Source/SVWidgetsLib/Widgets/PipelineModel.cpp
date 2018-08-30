/* ============================================================================
* Copyright (c) 2017 BlueQuartz Software, LLC
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
* Neither the name of BlueQuartz Software nor the names of its
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
*
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#include "PipelineModel.h"

#include <QtWidgets>

#include "SIMPLib/FilterParameters/JsonFilterParametersReader.h"
#include "SIMPLib/FilterParameters/H5FilterParametersReader.h"
#include "SIMPLib/FilterParameters/JsonFilterParametersWriter.h"
#include "SIMPLib/FilterParameters/H5FilterParametersWriter.h"

#include "SVWidgetsLib/Core/SVWidgetsLibConstants.h"
#include "SVWidgetsLib/Widgets/PipelineItem.h"
#include "SVWidgetsLib/Widgets/PipelineFilterMimeData.h"
#include "SVWidgetsLib/Widgets/FilterInputWidget.h"
#include "SVWidgetsLib/Widgets/SVStyle.h"
#include "SVWidgetsLib/QtSupport/QtSSettings.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PipelineModel::PipelineModel(QObject* parent)
: QAbstractItemModel(parent)
{
  QVector<QVariant> vector;
  vector.push_back("");
  m_RootItem = new PipelineItem(vector);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PipelineModel::PipelineModel(size_t maxPipelineCount, QObject* parent)
: QAbstractItemModel(parent)
, m_MaxPipelineCount(maxPipelineCount)
{
  QVector<QVariant> vector;
  vector.push_back("");
  m_RootItem = new PipelineItem(vector);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PipelineModel::~PipelineModel()
{
  delete m_RootItem;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool PipelineModel::savePipeline(const QModelIndex &pipelineRootIndex, const QString &pipelineName)
{
  PipelineItem* item = getItem(pipelineRootIndex);
  if (item == nullptr || item->getItemType() != PipelineItem::ItemType::PipelineRoot)
  {
    return false;
  }

  FilterPipeline::Pointer tempPipeline = item->getTempPipeline();
  FilterPipeline::Pointer savedPipeline = tempPipeline->deepCopy();
  item->setSavedPipeline(savedPipeline);
  tempPipeline->setName(pipelineName);
  savedPipeline->setName(pipelineName);

  emit pipelineSaved(savedPipeline, pipelineRootIndex);
  return true;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineModel::updateActivePipeline(const QModelIndex &pipelineIdx)
{
  emit clearIssuesTriggered();

  setActivePipeline(pipelineIdx);

  m_ActivePipelineIndex = pipelineIdx;

  if (m_ActivePipelineIndex.isValid() == true)
  {
    emit preflightTriggered(m_ActivePipelineIndex);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int PipelineModel::columnCount(const QModelIndex& parent) const
{
  return m_RootItem->columnCount();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QVariant PipelineModel::data(const QModelIndex& index, int role) const
{
  if(!index.isValid())
  {
    return QVariant();
  }

  PipelineItem* item = getItem(index);
  SVStyle* style = SVStyle::Instance();

  if (role == PipelineModel::Roles::WidgetStateRole)
  {
    return static_cast<int>(item->getWidgetState());
  }
  else if (role == PipelineModel::Roles::ErrorStateRole)
  {
    return static_cast<int>(item->getErrorState());
  }
  else if (role == PipelineModel::Roles::PipelineStateRole)
  {
    return static_cast<int>(item->getPipelineState());
  }
  else if (role == PipelineModel::Roles::PipelineModifiedRole)
  {
    return item->isPipelineModified();
  }
  else if (role == PipelineModel::Roles::ItemTypeRole)
  {
    return static_cast<int>(item->getItemType());
  }
  else if (role == PipelineModel::Roles::ExpandedRole)
  {
    return item->getExpanded();
  }
  else if(role == Qt::DisplayRole && m_UseModelDisplayText)
  {
    if (item->getItemType() == PipelineItem::ItemType::PipelineRoot)
    {
      FilterPipeline::Pointer pipeline = item->getTempPipeline();
      return pipeline->getName();
    }
    else if (item->getItemType() == PipelineItem::ItemType::Filter)
    {
      PipelineItem* parentItem = item->parent();
      if (parentItem)
      {
        FilterPipeline::Pointer pipeline = parentItem->getTempPipeline();
        FilterPipeline::FilterContainerType container = pipeline->getFilterContainer();
        if (index.row() < container.size())
        {
          return container[index.row()]->getHumanLabel();
        }
      }
    }

    return item->data(index.column());
  }
  else if(role == PipelineModel::Roles::PipelinePathRole)
  {
    return item->getPipelineFilePath();
  }
  else if(role == Qt::ForegroundRole)
  {
    QColor fgColor = getForegroundColor(index);
    return fgColor;
  }
  else if(role == Qt::ToolTipRole)
  {
    return QString();
  }
  else if(role == Qt::DecorationRole)
  {
    QModelIndex nameIndex = this->index(index.row(), PipelineItem::Contents, index.parent());
    if(nameIndex == index)
    {
      PipelineItem* item = getItem(index);
      return item->getIcon();
    }
    else
    {
      return QVariant();
    }
  }

  return QVariant();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FilterPipeline::Pointer PipelineModel::tempPipeline(const QModelIndex &index) const
{
  PipelineItem* item = getItem(index);
  if (item == nullptr || item->getItemType() != PipelineItem::ItemType::PipelineRoot)
  {
    return FilterPipeline::NullPointer();
  }

  return item->getTempPipeline();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FilterPipeline::Pointer PipelineModel::savedPipeline(const QModelIndex &index) const
{
  PipelineItem* item = getItem(index);
  if (item == nullptr || item->getItemType() != PipelineItem::ItemType::PipelineRoot)
  {
    return FilterPipeline::NullPointer();
  }

  return item->getSavedPipeline();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool PipelineModel::addPipeline(FilterPipeline::Pointer pipeline)
{
  if (pipeline.get() == nullptr)
  {
    return false;
  }

  int row = rowCount();
  insertRow(row);
  QModelIndex index = this->index(row, PipelineItem::Contents);
  setData(index, static_cast<int>(PipelineItem::ItemType::PipelineRoot), Roles::ItemTypeRole);

  return setPipeline(index, pipeline);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool PipelineModel::setPipeline(const QModelIndex &index, FilterPipeline::Pointer pipeline)
{
  QModelIndex pipelineRootIndex = index;
  PipelineItem* pipelineRootItem = getItem(pipelineRootIndex);
  if (pipelineRootItem == nullptr || pipelineRootItem->getItemType() != PipelineItem::ItemType::PipelineRoot)
  {
    return false;
  }

  if (this->savedPipeline(pipelineRootIndex).get() != nullptr)
  {
    for (int i = 0; i < rowCount(pipelineRootIndex); i++)
    {
      QModelIndex oldFilterIndex = this->index(i, PipelineItem::Contents, pipelineRootIndex);
      FilterInputWidget* fiw = filterInputWidget(oldFilterIndex);

      disconnect(fiw, &FilterInputWidget::filterParametersChanged, 0, 0);
    }

    bool success = removeRows(0, rowCount(pipelineRootIndex), pipelineRootIndex);
    if (!success)
    {
      return false;
    }
  }

  if (pipelineRootItem->getTempPipeline().get() != nullptr)
  {
    FilterPipeline::Pointer oldTempPipeline = pipelineRootItem->getTempPipeline();

    FilterPipeline::FilterContainerType oldTempContainer = oldTempPipeline->getFilterContainer();
    for (int i = 0; i < oldTempContainer.size(); i++)
    {
      AbstractFilter::Pointer oldFilter = oldTempContainer[i];

      disconnect(oldFilter.get(), SIGNAL(filterCompleted(AbstractFilter*)), this, SLOT(listenFilterCompleted(AbstractFilter*)));

      disconnect(oldFilter.get(), SIGNAL(filterInProgress(AbstractFilter*)), this, SLOT(listenFilterInProgress(AbstractFilter*)));
    }

    disconnect(oldTempPipeline.get(), &FilterPipeline::filtersWereAdded, nullptr, nullptr);
    disconnect(oldTempPipeline.get(), &FilterPipeline::filtersWereRemoved, nullptr, nullptr);
    disconnect(oldTempPipeline.get(), &FilterPipeline::pipelineWasEdited, nullptr, nullptr);
  }

  if (pipeline != FilterPipeline::NullPointer())
  {
    bool success = insertRows(0, pipeline->size(), pipelineRootIndex);
    if (!success)
    {
      return false;
    }

    pipelineRootItem->setTempPipeline(pipeline);
    pipelineRootItem->setSavedPipeline(pipeline->deepCopy());

    FilterPipeline::FilterContainerType container = pipeline->getFilterContainer();
    for (int i = 0; i < rowCount(pipelineRootIndex); i++)
    {
      AbstractFilter::Pointer filter = container[i];
      QModelIndex filterIndex = this->index(i, PipelineItem::Contents, pipelineRootIndex);
      addFilterData(filter, filterIndex);
    }

    if (getActivePipeline() == pipelineRootIndex)
    {
      emit clearIssuesTriggered();
      emit preflightTriggered(pipelineRootIndex);
    }

    connect(pipeline.get(), &FilterPipeline::filtersWereAdded, [=] (std::vector<AbstractFilter::Pointer> filters, std::vector<size_t> indices) {
      if (filters.size() != indices.size())
      {
        return;
      }

      for (int i = 0; i < filters.size(); i++)
      {
        insertFilter(filters[i], indices[i], pipelineRootIndex);
      }

      if (getActivePipeline() == pipelineRootIndex)
      {
        emit clearIssuesTriggered();
        emit preflightTriggered(pipelineRootIndex);
      }
    });

    // Connection that automatically updates the model when a filter gets removed from the FilterPipeline
    connect(pipeline.get(), &FilterPipeline::filtersWereRemoved, [=] (std::vector<size_t> indices) {
      size_t offset = 0;
      for (int i = 0; i < indices.size(); i++)
      {
        removeRow(indices[i] - offset, pipelineRootIndex);
        offset++;
      }

      if (getActivePipeline() == pipelineRootIndex)
      {
        emit clearIssuesTriggered();
        emit preflightTriggered(pipelineRootIndex);
      }
    });

    emit dataChanged(index, index);
    emit pipelineAdded(pipeline, pipelineRootIndex);
  }
  else if (getActivePipeline() == pipelineRootIndex)
  {
    emit clearIssuesTriggered();
  }

  return true;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineModel::insertFilter(AbstractFilter::Pointer filter, int index, const QModelIndex &pipelineRootIndex)
{
  insertRow(index, pipelineRootIndex);
  QModelIndex filterIndex = this->index(index, PipelineItem::Contents, pipelineRootIndex);
  addFilterData(filter, filterIndex);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString PipelineModel::pipelineFilePath(const QModelIndex &index)
{
  PipelineItem* item = getItem(index);
  if (item == nullptr || item->getItemType() != PipelineItem::ItemType::PipelineRoot)
  {
    return QString();
  }

  return item->getPipelineFilePath();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineModel::setPipelineFilePath(const QModelIndex& index, const QString &filePath)
{
  PipelineItem* item = getItem(index);
  if (item == nullptr || item->getItemType() != PipelineItem::ItemType::PipelineRoot)
  {
    return;
  }

  item->setPipelineFilePath(filePath);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractFilter::Pointer PipelineModel::filter(const QModelIndex &index) const
{
  if (isFilterItem(index))
  {
    QModelIndex parentIndex = index.parent();
    FilterPipeline::Pointer pipeline = tempPipeline(parentIndex);
    FilterPipeline::FilterContainerType container = pipeline->getFilterContainer();
    if (index.row() < container.size())
    {
      return container[index.row()];
    }
  }

  return AbstractFilter::NullPointer();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QModelIndex PipelineModel::getPipelineRootIndexFromPipeline(FilterPipeline::Pointer pipeline)
{
  for (int i = 0; i < rowCount(); i++)
  {
    QModelIndex index = this->index(i, PipelineItem::Contents);
    FilterPipeline::Pointer storedPipeline = tempPipeline(index);
    if (storedPipeline == pipeline)
    {
      return index;
    }
  }

  return QModelIndex();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineModel::addFilterData(AbstractFilter::Pointer filter, const QModelIndex &filterIndex)
{
  setData(filterIndex, static_cast<int>(PipelineItem::ItemType::Filter), Roles::ItemTypeRole);

  if(filter->getEnabled() == false)
  {
    setData(filterIndex, static_cast<int>(PipelineItem::WidgetState::Disabled), PipelineModel::WidgetStateRole);
  }

  // Connection that changes the filter state to Completed when the filter is completed
  connect(filter.get(), SIGNAL(filterCompleted(AbstractFilter*)), this, SLOT(listenFilterCompleted(AbstractFilter*)));

  // Connection that changes the filter state to In Progress when the filter is in progress
  connect(filter.get(), SIGNAL(filterInProgress(AbstractFilter*)), this, SLOT(listenFilterInProgress(AbstractFilter*)));

  FilterInputWidget* fiw = new FilterInputWidget(filter, nullptr);
  fiw->displayFilterParameters(filter);
  setFilterInputWidget(filterIndex, fiw);

  // Connection that triggers a preflight and re-emits that filter parameters have changed if the filter input widget is edited
  connect(fiw, &FilterInputWidget::filterParametersChanged, [=] {
    emit preflightTriggered(filterIndex.parent());
    emit filterParametersChanged(filter);
  });
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString PipelineModel::dropIndicatorText(const QModelIndex &index) const
{
  if(!index.isValid())
  {
    return QString();
  }

  PipelineItem* item = getItem(index);
  if (item == nullptr)
  {
    return QString();
  }

  return item->getDropIndicatorText();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineModel::setDropIndicatorText(const QModelIndex &index, const QString &text)
{
  if(!index.isValid())
  {
    return;
  }

  PipelineItem* item = getItem(index);
  if (item == nullptr)
  {
    return;
  }

  item->setDropIndicatorText(text);

  emit dataChanged(index, index);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QModelIndex PipelineModel::indexOfFilter(AbstractFilter* filter, const QModelIndex &parent)
{
  for (int i = 0; i < rowCount(parent); i++)
  {
    QModelIndex childIndex = index(i, PipelineItem::Contents, parent);
    if (this->filter(childIndex).get() == filter)
    {
      return childIndex;
    }
  }

  return QModelIndex();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool PipelineModel::hasActivePipeline()
{
  return m_ActivePipelineIndex.isValid();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QModelIndex PipelineModel::getActivePipeline() const
{
  return m_ActivePipelineIndex;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineModel::setActivePipeline(const QModelIndex &index)
{
  PipelineItem* oldActiveItem = getItem(m_ActivePipelineIndex);
  if (oldActiveItem)
  {
    oldActiveItem->setActivePipeline(false);

    emit dataChanged(m_ActivePipelineIndex, m_ActivePipelineIndex);
  }

  PipelineItem* newActiveItem = getItem(index);
  if (newActiveItem)
  {
    newActiveItem->setActivePipeline(true);

    emit dataChanged(index, index);
  }
  
  m_ActivePipelineIndex = index;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineModel::clearActivePipeline()
{
  setActivePipeline(QModelIndex());
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
Qt::ItemFlags PipelineModel::flags(const QModelIndex& index) const
{
  if (!index.isValid() || index.model() != this)
  {
    return Qt::ItemIsDropEnabled;
  }

  Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);

  Qt::ItemFlags itemFlags = Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled | defaultFlags;

  return itemFlags;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QStringList PipelineModel::mimeTypes() const
{
  QStringList types;
  types << SIMPLView::DragAndDrop::FilterPipelineItem;
  types << SIMPLView::DragAndDrop::FilterListItem;
  types << SIMPLView::DragAndDrop::BookmarkItem;
  types << SIMPLView::DragAndDrop::Url;
  return types;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool PipelineModel::canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const
{
  Q_UNUSED(row);
  Q_UNUSED(parent);

  if (action == Qt::IgnoreAction)
  {
    return false;
  }

  if (!data->hasFormat(SIMPLView::DragAndDrop::FilterPipelineItem)
      && !data->hasFormat(SIMPLView::DragAndDrop::FilterListItem)
      && !data->hasFormat(SIMPLView::DragAndDrop::BookmarkItem)
      && !data->hasFormat(SIMPLView::DragAndDrop::Url))
  {
    return false;
  }

  if (column > 0)
  {
    return false;
  }

  return true;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PipelineItem* PipelineModel::getItem(const QModelIndex& index) const
{
  if(index.isValid())
  {
    PipelineItem* item = static_cast<PipelineItem*>(index.internalPointer());
    if(item)
    {
      return item;
    }
  }
  return m_RootItem;
}

//// -----------------------------------------------------------------------------
////
//// -----------------------------------------------------------------------------
//QVariant PipelineModel::headerData(int section, Qt::Orientation orientation, int role) const
//{
//  if(orientation == Qt::Horizontal && role == Qt::DisplayRole)
//  {
//    return m_RootItem->data(section);
//  }

//  return QVariant();
//}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QModelIndex PipelineModel::index(int row, int column, const QModelIndex& parent) const
{
  if(parent.isValid() && parent.column() != 0)
  {
    return QModelIndex();
  }

  PipelineItem* parentItem = getItem(parent);

  PipelineItem* childItem = parentItem->child(row);
  if(childItem)
  {
    return createIndex(row, column, childItem);
  }
  else
  {
    return QModelIndex();
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool PipelineModel::insertRows(int position, int rows, const QModelIndex& parent)
{
  PipelineItem* parentItem = getItem(parent);
  bool success;

  beginInsertRows(parent, position, position + rows - 1);
  success = parentItem->insertChildren(position, rows, m_RootItem->columnCount());
  endInsertRows();

  return success;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool PipelineModel::removeRows(int position, int rows, const QModelIndex& parent)
{
  if(position < 0)
  {
    return false;
  }

  PipelineItem* parentItem = getItem(parent);
  bool success = true;

  beginRemoveRows(parent, position, position + rows - 1);
  success = parentItem->removeChildren(position, rows);
  endRemoveRows();

  return success;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool PipelineModel::moveRows(const QModelIndex& sourceParent, int sourceRow, int count, const QModelIndex& destinationParent, int destinationChild)
{
  beginMoveRows(sourceParent, sourceRow, sourceRow + count - 1, destinationParent, destinationChild);

  PipelineItem* srcParentItem = getItem(sourceParent);
  PipelineItem* destParentItem = getItem(destinationParent);

  for(int i = sourceRow; i < sourceRow + count; i++)
  {
    QModelIndex srcIndex = index(i, PipelineItem::Contents, sourceParent);
    PipelineItem* srcItem = getItem(srcIndex);

    destParentItem->insertChild(destinationChild, srcItem);
    srcItem->setParent(destParentItem);
    srcParentItem->removeChild(i);
  }

  endMoveRows();

  return true;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QModelIndex PipelineModel::parent(const QModelIndex& index) const
{
  if(!index.isValid())
  {
    return QModelIndex();
  }

  PipelineItem* childItem = getItem(index);
  PipelineItem* parentItem = childItem->parent();

  if(parentItem == m_RootItem)
  {
    return QModelIndex();
  }

  return createIndex(parentItem->childNumber(), 0, parentItem);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int PipelineModel::rowCount(const QModelIndex& parent) const
{
  PipelineItem* parentItem = getItem(parent);

  return parentItem->childCount();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool PipelineModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
  PipelineItem* item = getItem(index);

  if (role == PipelineModel::Roles::WidgetStateRole)
  {
    bool ok = false;
    int intValue = value.toInt(&ok);
    if (ok == false)
    {
      return false;
    }

    PipelineItem::WidgetState value = static_cast<PipelineItem::WidgetState>(intValue);
    item->setWidgetState(value);
  }
  else if (role == PipelineModel::Roles::ErrorStateRole)
  {
    bool ok = false;
    int intValue = value.toInt(&ok);
    if (ok == false)
    {
      return false;
    }

    PipelineItem::ErrorState value = static_cast<PipelineItem::ErrorState>(intValue);
    item->setErrorState(value);
  }
  else if (role == PipelineModel::Roles::PipelineStateRole)
  {
    bool ok = false;
    int intValue = value.toInt(&ok);
    if (ok == false)
    {
      return false;
    }

    PipelineItem::PipelineState value = static_cast<PipelineItem::PipelineState>(intValue);
    item->setPipelineState(value);
  }
  else if (role == PipelineModel::Roles::ItemTypeRole)
  {
    bool ok = false;
    int intValue = value.toInt(&ok);
    if (ok == false)
    {
      return false;
    }

    PipelineItem::ItemType value = static_cast<PipelineItem::ItemType>(intValue);
    item->setItemType(value);
  }
  else if (role == PipelineModel::Roles::PipelinePathRole)
  {
    item->setPipelineFilePath(value.toString());
  }
  else if (role == PipelineModel::Roles::PipelineModifiedRole)
  {
    item->setPipelineModified(value.toBool());
    emit pipelineModified(item->getTempPipeline(), this->getPipelineRootIndexFromPipeline(item->getTempPipeline()), value.toBool());
  }
  else if (role == PipelineModel::Roles::ExpandedRole)
  {
    int expanded = value.toBool();
    item->setExpanded(expanded);
  }
  else if(role == Qt::DecorationRole)
  {
    item->setIcon(value.value<QIcon>());
  }
  else if(role == Qt::ToolTipRole)
  {
    item->setItemTooltip(value.toString());
  }
  else if (role == Qt::DisplayRole)
  {
    item->setData(index.column(), value);
  }
  else
  {
    return false;
  }

  emit dataChanged(index, index);

  return true;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
Qt::DropActions PipelineModel::supportedDropActions() const
{
  return Qt::CopyAction | Qt::MoveAction;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int PipelineModel::getMaxFilterCount() const
{
  int numFilters = rowCount();
  int maxFilterCount = 0;
  if(numFilters < 10)
  {
    numFilters = 11;
    maxFilterCount = 99;
  }

  if(numFilters > 9)
  {
    int mag = 0;
    int max = numFilters;
    maxFilterCount = 1;
    while(max > 0)
    {
      mag++;
      max = max / 10;
      maxFilterCount *= 10;
    }
    maxFilterCount -= 1;
  }

  return maxFilterCount;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineModel::listenFilterInProgress(AbstractFilter* filter)
{
  QModelIndex index = indexOfFilter(filter);

  // Do not set state to Executing if the filter is disabled
  PipelineItem::WidgetState wState = static_cast<PipelineItem::WidgetState>(data(index, PipelineModel::WidgetStateRole).toInt());
  if(wState != PipelineItem::WidgetState::Disabled)
  {
    setData(index, static_cast<int>(PipelineItem::WidgetState::Executing), PipelineModel::WidgetStateRole);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineModel::listenFilterCompleted(AbstractFilter* filter)
{
  QModelIndex index = indexOfFilter(filter);

  // Do not set state to Completed if the filter is disabled
  PipelineItem::WidgetState wState = static_cast<PipelineItem::WidgetState>(data(index, PipelineModel::WidgetStateRole).toInt());
  if(wState != PipelineItem::WidgetState::Disabled)
  {
    setData(index, static_cast<int>(PipelineItem::WidgetState::Completed), PipelineModel::WidgetStateRole);
  }
  if(filter->getWarningCondition() < 0)
  {
    setData(index, static_cast<int>(PipelineItem::ErrorState::Warning), PipelineModel::ErrorStateRole);
  }
  if(filter->getErrorCondition() < 0)
  {
    setData(index, static_cast<int>(PipelineItem::ErrorState::Error), PipelineModel::ErrorStateRole);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FilterInputWidget* PipelineModel::filterInputWidget(const QModelIndex &index)
{
  PipelineItem* item = getItem(index);
  return item->getFilterInputWidget();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineModel::setFilterInputWidget(const QModelIndex &index, FilterInputWidget* fiw)
{
  PipelineItem* item = getItem(index);
  item->setFilterInputWidget(fiw);

  emit dataChanged(index, index);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PipelineItem* PipelineModel::getRootItem()
{
  return m_RootItem;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool PipelineModel::isEmpty()
{
  if(rowCount(QModelIndex()) <= 0)
  {
    return true;
  }
  return false;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool PipelineModel::isPipelineRootItem(const QModelIndex &index) const
{
  return static_cast<PipelineItem::ItemType>(data(index, Roles::ItemTypeRole).toInt()) == PipelineItem::ItemType::PipelineRoot;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool PipelineModel::isFilterItem(const QModelIndex &index) const
{
  PipelineItem::ItemType itemType = static_cast<PipelineItem::ItemType>(data(index, Roles::ItemTypeRole).toInt());
  return itemType == PipelineItem::ItemType::Filter;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool PipelineModel::isDropIndicatorItem(const QModelIndex &index) const
{
  PipelineItem::ItemType itemType = static_cast<PipelineItem::ItemType>(data(index, Roles::ItemTypeRole).toInt());
  return itemType == PipelineItem::ItemType::DropIndicator;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QColor PipelineModel::getForegroundColor(const QModelIndex &index) const
{
  if (index.isValid() == false)
  {
    return QColor();
  }

  PipelineItem* item = getItem(index);

  PipelineItem::WidgetState wState = item->getWidgetState();
  PipelineItem::PipelineState pState = item->getPipelineState();
  PipelineItem::ErrorState eState = item->getErrorState();

  QColor fgColor;

  switch(wState)
  {
  case PipelineItem::WidgetState::Ready:
    fgColor = QColor();
    break;
  case PipelineItem::WidgetState::Executing:
    fgColor = QColor(6, 140, 190);
    break;
  case PipelineItem::WidgetState::Completed:
    fgColor = QColor(6, 118, 6);
    break;
  case PipelineItem::WidgetState::Disabled:
    fgColor = QColor(96, 96, 96);
    break;
  }

  // Do not change the background color if the widget is disabled.
  if(wState != PipelineItem::WidgetState::Disabled)
  {
    switch(pState)
    {
    case PipelineItem::PipelineState::Running:
      fgColor = QColor(190, 190, 190);
      break;
    case PipelineItem::PipelineState::Stopped:
      fgColor = QColor(0, 0, 0);
      break;
    case PipelineItem::PipelineState::Paused:
      fgColor = QColor(0, 0, 0);
      break;
    }
  }

  switch(eState)
  {
  case PipelineItem::ErrorState::Ok:

    break;
  case PipelineItem::ErrorState::Error:
    fgColor = QColor(179, 2, 5);
    break;
  case PipelineItem::ErrorState::Warning:
    fgColor = QColor(215, 197, 1);
    break;
  }

  return fgColor;
}
