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

#include "SVPipelineTreeView.h"

#include <QtCore/QJsonDocument>

#include <QtGui/QMouseEvent>
#include <QtGui/QDrag>

#include <QtWidgets/QApplication>

#include "SIMPLib/Filtering/FilterManager.h"

#include "SVWidgetsLib/Core/SVWidgetsLibConstants.h"
#include "SVWidgetsLib/Widgets/SVPipelineTreeViewDelegate.h"
#include "SVWidgetsLib/Widgets/SVPipelineTreeViewSelectionModel.h"
#include "SVWidgetsLib/Widgets/PipelineModel.h"
#include "SVWidgetsLib/Widgets/PipelineFilterMimeData.h"
#include "SVWidgetsLib/Widgets/AddPipelineToModelCommand.h"
#include "SVWidgetsLib/Widgets/AddFilterToPipelineCommand.h"
#include "SVWidgetsLib/Widgets/RemoveFilterFromPipelineCommand.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
SVPipelineTreeView::SVPipelineTreeView(QWidget* parent)
: QTreeView(parent)
, PipelineView()
{
  setupGui();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
SVPipelineTreeView::~SVPipelineTreeView()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineTreeView::setupGui()
{
  setContextMenuPolicy(Qt::CustomContextMenu);
  setFocusPolicy(Qt::StrongFocus);
  setAcceptDrops(true);

  SVPipelineTreeViewDelegate* delegate = new SVPipelineTreeViewDelegate(this);
  setItemDelegate(delegate);

  // Create the model
  PipelineModel* model = new PipelineModel(this);
  model->setUseModelDisplayText(false);
  setModel(model);

  SVPipelineTreeViewSelectionModel* selectionModel = new SVPipelineTreeViewSelectionModel(model);
  setSelectionModel(selectionModel);

  connectSignalsSlots();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineTreeView::connectSignalsSlots()
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QModelIndexList SVPipelineTreeView::getSelectedRows()
{
  return selectionModel()->selectedRows();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QPixmap SVPipelineTreeView::getDraggingPixmap(QModelIndexList indexes)
{
  if(indexes.size() <= 0)
  {
    return QPixmap();
  }

//  SVPipelineTreeViewDelegate* delegate = dynamic_cast<SVPipelineTreeViewDelegate*>(itemDelegate());
//  if(delegate == nullptr)
//  {
//    return QPixmap();
//  }

//  QPixmap indexPixmap = delegate->createPixmap(indexes[0]);

//  int dragPixmapWidth = indexPixmap.size().width();
//  int dragPixmapHeight = indexPixmap.size().height() * indexes.size() + ((indexes.size() - 1));

//  QPixmap dragPixmap(dragPixmapWidth, dragPixmapHeight);
//  dragPixmap.fill(Qt::transparent);

//  QPainter p;
//  p.begin(&dragPixmap);
//  p.setOpacity(0.70);
//  int offset = 0;
//  for(int i = 0; i < indexes.size(); i++)
//  {
//    QPixmap currentPixmap = delegate->createPixmap(indexes[i]);
//    p.drawPixmap(0, offset, currentPixmap);
//    offset = offset + indexPixmap.size().height();
//  }
//  p.end();

  return QPixmap();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineTreeView::mouseMoveEvent(QMouseEvent* event)
{
  if((event->buttons() & Qt::LeftButton) && (event->pos() - m_DragStartPosition).manhattanLength() >= QApplication::startDragDistance() + 1 && dragEnabled() == true)
  {
    beginDrag(event);
  }
  else
  {
    QTreeView::mouseMoveEvent(event);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineTreeView::beginDrag(QMouseEvent* event)
{
  QModelIndexList selectedIndexes = selectionModel()->selectedRows();
  if(selectedIndexes.size() <= 0)
  {
    return;
  }

  qSort(selectedIndexes);

  QPixmap dragPixmap = getDraggingPixmap(selectedIndexes);

  std::vector<PipelineFilterMimeData::FilterDragMetadata> filtersDragData;
  std::vector<AbstractFilter::Pointer> filters;
  PipelineModel* model = getPipelineModel();
  Qt::KeyboardModifiers modifiers = QApplication::queryKeyboardModifiers();

  for(int i = 0; i < selectedIndexes.size(); i++)
  {
    QModelIndex selectedIndex = selectedIndexes[i];

    AbstractFilter::Pointer filter = model->filter(selectedIndex);

    if(modifiers.testFlag(Qt::AltModifier) == true)
    {
      filter = filter->newFilterInstance(true);
    }

    PipelineFilterMimeData::FilterDragMetadata filterDragData;
    filterDragData.first = filter;
    filterDragData.second = selectedIndex.row();
    filtersDragData.push_back(filterDragData);

    filters.push_back(filter);
  }

  PipelineFilterMimeData* mimeData = new PipelineFilterMimeData();
  mimeData->setFilterDragData(filtersDragData);
  mimeData->setData(SIMPLView::DragAndDrop::FilterPipelineItem, QByteArray());
  mimeData->setSourceController(getPipelineViewController());

  QRect firstSelectionRect = visualRect(selectedIndexes[0]);
  QPoint dragPos(event->pos().x() - firstSelectionRect.x(), event->pos().y() - firstSelectionRect.y());

  if(modifiers.testFlag(Qt::AltModifier) == false)
  {
    FilterPipeline::Pointer pipeline = model->tempPipeline(getActivePipeline());
    RemoveFilterFromPipelineCommand* cmd = new RemoveFilterFromPipelineCommand(filters, pipeline, getPipelineModel());
    if(filters.size() == 1)
    {
      cmd->setText(QObject::tr("\"%1 '%2' from pipeline '%3'\"").arg("Remove").arg(filters[0]->getHumanLabel()).arg(pipeline->getName()));
    }
    else
    {
      cmd->setText(QObject::tr("\"%1 %2 filters from pipeline '%3'\"").arg("Remove").arg(filters.size()).arg(pipeline->getName()));
    }
    cmd->setText(cmd->text());

    mimeData->setSourceUndoCommand(cmd);
  }

  QDrag* drag = new QDrag(this);
  drag->setMimeData(mimeData);
//  drag->setPixmap(dragPixmap);
  drag->setHotSpot(dragPos);

  if(modifiers.testFlag(Qt::AltModifier))
  {
    drag->exec(Qt::CopyAction);
  }
  else
  {
    drag->exec(Qt::MoveAction);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineTreeView::dragMoveEvent(QDragMoveEvent* event)
{
  QTreeView::dragMoveEvent(event);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineTreeView::dropEvent(QDropEvent* event)
{
  PipelineModel* model = getPipelineModel();

  DropIndicatorPosition dropIndicatorPos = dropIndicatorPosition();

  QModelIndex pipelineRootIndex;

  QModelIndex nearestIndex = indexAt(event->pos());
  if (nearestIndex.isValid() == false)
  {
    pipelineRootIndex = QModelIndex();
  }
  else
  {
    PipelineItem::ItemType itemType = static_cast<PipelineItem::ItemType>(model->data(nearestIndex, PipelineModel::Roles::ItemTypeRole).toInt());
    if (itemType == PipelineItem::ItemType::PipelineRoot)
    {
      if (dropIndicatorPos == QAbstractItemView::AboveItem || dropIndicatorPos == QAbstractItemView::BelowItem)
      {
        pipelineRootIndex = QModelIndex();
      }
      else
      {
        pipelineRootIndex = nearestIndex;
      }
    }
    else
    {
      pipelineRootIndex = nearestIndex.parent();
    }
  }

  int dropRow = filterCount(pipelineRootIndex);
  if (dropIndicatorPos == QAbstractItemView::AboveItem)
  {
    dropRow = nearestIndex.row();
  }
  else if (dropIndicatorPos == QAbstractItemView::BelowItem)
  {
    dropRow = nearestIndex.row() + 1;
  }

  const QMimeData* mimedata = event->mimeData();
  const PipelineFilterMimeData* filterData = dynamic_cast<const PipelineFilterMimeData*>(mimedata);
  if(filterData != nullptr)
  {
    // This is filter data from an SVPipelineTreeView instance
    std::vector<PipelineFilterMimeData::FilterDragMetadata> dragData = filterData->getFilterDragData();

    std::vector<AbstractFilter::Pointer> filters;
    for(size_t i = 0; i < dragData.size(); i++)
    {
      filters.push_back(dragData[i].first);
    }

    if (filters.empty())
    {
      return;
    }

    FilterPipeline::Pointer pipeline = model->tempPipeline(pipelineRootIndex);
    FilterPipeline::FilterContainerType dropContainer = pipeline->getFilterContainer();

    if (getPipelineViewController() != nullptr)
    {
      Qt::KeyboardModifiers modifiers = QApplication::queryKeyboardModifiers();
      if(dropContainer.contains(filters[0]) && modifiers.testFlag(Qt::AltModifier) == false)
      {
        if (filterData->getSourceUndoCommand() != nullptr)
        {
          std::vector<QUndoCommand*> cmds;
          cmds.push_back(filterData->getSourceUndoCommand());

          if (pipelineRootIndex.isValid())
          {
            dropRow = dropRow - filters.size();  // The filters we are moving haven't been removed from their original positions yet, so this is the new drop row after removing the filters
            AddFilterToPipelineCommand* cmd = new AddFilterToPipelineCommand(filters, pipeline, dropRow, model);
            cmds.push_back(cmd);
          }
          else
          {
            FilterPipeline::Pointer pipeline = FilterPipeline::New();
            foreach(AbstractFilter::Pointer f, filters)
            {
              pipeline->pushBack(f);
            }

            AddPipelineToModelCommand* cmd = new AddPipelineToModelCommand(pipeline, dropRow, model);
            cmds.push_back(cmd);
          }

          QString cmdText;
          if(filters.size() == 1)
          {
            cmdText = QObject::tr("\"%1 '%2'\"").arg("Move").arg(filters[0]->getHumanLabel());
          }
          else
          {
            cmdText = QObject::tr("\"%1 %2 filters\"").arg("Move").arg(filters.size());
          }
          getPipelineViewController()->addUndoCommandMacro(cmds, cmdText);
        }
      }
      else
      {
        AddFilterToPipelineCommand* cmd = new AddFilterToPipelineCommand(filters, pipeline, dropRow, model);
        if(filters.size() == 1)
        {
          cmd->setText(QObject::tr("\"%1 '%2' to '%3'\"").arg("Add").arg(filters[0]->getHumanLabel()).arg(pipeline->getName()));
        }
        else
        {
          cmd->setText(QObject::tr("\"%1 %2 filters to '%3'\"").arg("Add").arg(filters.size()).arg(pipeline->getName()));
        }
        getPipelineViewController()->addUndoCommand(cmd);
      }

      clearSelection();

      QModelIndex leftIndex = model->index(dropRow, PipelineItem::Contents);
      QModelIndex rightIndex = model->index(dropRow + filters.size() - 1, PipelineItem::Contents);
      QItemSelection selection(leftIndex, rightIndex);

      selectionModel()->select(selection, QItemSelectionModel::Select);

      event->accept();
    }
    else
    {
      event->ignore();
    }
  }
  else if(mimedata->hasUrls())
  {
    QString data = mimedata->text();
    QUrl url(data);
    QString filePath = url.toLocalFile();

    int err = openPipeline(filePath, pipelineRootIndex, dropRow);

    if(err >= 0)
    {
      event->accept();
    }
    else
    {
      event->ignore();
    }
  }
  else if(mimedata->hasFormat(SIMPLView::DragAndDrop::BookmarkItem))
  {
    QByteArray jsonArray = mimedata->data(SIMPLView::DragAndDrop::BookmarkItem);
    QJsonDocument doc = QJsonDocument::fromJson(jsonArray);
    QJsonObject obj = doc.object();

    if(obj.size() > 1)
    {
      event->ignore();
      return;
    }

    QJsonObject::iterator iter = obj.begin();
    QString filePath = iter.value().toString();

    int err = openPipeline(filePath, pipelineRootIndex, dropRow);

    if(err >= 0)
    {
      event->accept();
    }
    else
    {
      event->ignore();
    }
  }
  else if(mimedata->hasFormat(SIMPLView::DragAndDrop::FilterListItem))
  {
    QByteArray jsonArray = mimedata->data(SIMPLView::DragAndDrop::FilterListItem);
    QJsonDocument doc = QJsonDocument::fromJson(jsonArray);
    QJsonObject obj = doc.object();
    QJsonObject::iterator iter = obj.begin();
    QString filterClassName = iter.value().toString();

    FilterManager* fm = FilterManager::Instance();
    if(nullptr == fm)
    {
      event->ignore();
      return;
    }

    IFilterFactory::Pointer wf = fm->getFactoryFromClassName(filterClassName);
    if(nullptr == wf)
    {
      event->ignore();
      return;
    }

    AbstractFilter::Pointer filter = wf->create();

    if (pipelineRootIndex.isValid())
    {
      addFilter(filter, pipelineRootIndex, dropRow);
    }
    else
    {
      FilterPipeline::Pointer pipeline = FilterPipeline::New();
      pipeline->pushBack(filter);
      addPipeline(pipeline, dropRow);
    }

    event->accept();
  }
  else
  {
    event->ignore();
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int SVPipelineTreeView::openPipeline(const QString& filePath, QModelIndex pipelineRootIndex, int insertIndex)
{
  if(getPipelineViewController())
  {
    int err = getPipelineViewController()->openPipeline(filePath, pipelineRootIndex, insertIndex);
    expand(pipelineRootIndex);
    return err;
  }

  return -1;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineTreeView::mousePressEvent(QMouseEvent* event)
{
  if(event->button() == Qt::LeftButton)
  {
    m_DragStartPosition = event->pos();

    if(indexAt(event->pos()).isValid() == false)
    {
      clearSelection();
    }
  }

  QTreeView::mousePressEvent(event);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineTreeView::setModel(QAbstractItemModel* model)
{
  PipelineModel* oldModel = dynamic_cast<PipelineModel*>(this->model());
  if(oldModel)
  {
    PipelineViewController* pipelineViewController = getPipelineViewController();

    delete oldModel;
    pipelineViewController->setPipelineModel(nullptr);
  }

  QTreeView::setModel(model);

  PipelineModel* pipelineModel = dynamic_cast<PipelineModel*>(model);

  if(pipelineModel != nullptr)
  {
    PipelineViewController* pipelineViewController = getPipelineViewController();
    pipelineViewController->setPipelineModel(pipelineModel);

    connect(pipelineModel, &PipelineModel::pipelineAdded, [=](FilterPipeline::Pointer pipeline, const QModelIndex &pipelineRootIndex) {
      Q_UNUSED(pipeline)
      expand(pipelineRootIndex);
    });
  }
}
