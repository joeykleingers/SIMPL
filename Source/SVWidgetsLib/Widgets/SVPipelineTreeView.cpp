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

#include <iostream>

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QMimeData>
#include <QtCore/QSharedPointer>
#include <QtCore/QSignalMapper>
#include <QtCore/QTemporaryFile>
#include <QtCore/QThread>
#include <QtCore/QUrl>

#include <QtGui/QClipboard>
#include <QtGui/QDrag>
#include <QtGui/QDragEnterEvent>
#include <QtGui/QDragLeaveEvent>
#include <QtGui/QDragMoveEvent>
#include <QtGui/QDropEvent>
#include <QtGui/QMouseEvent>
#include <QtGui/QPixmap>

#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QProgressDialog>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QVBoxLayout>

#include "SIMPLib/Common/DocRequestManager.h"
#include "SIMPLib/Common/PipelineMessage.h"
#include "SIMPLib/Common/SIMPLibSetGetMacros.h"
#include "SIMPLib/FilterParameters/JsonFilterParametersReader.h"
#include "SIMPLib/FilterParameters/JsonFilterParametersWriter.h"
#include "SIMPLib/Filtering/FilterFactory.hpp"
#include "SIMPLib/Filtering/FilterManager.h"
#include "SIMPLib/SIMPLib.h"

#include "SVWidgetsLib/QtSupport/QtSDroppableScrollArea.h"

#include "SVWidgetsLib/Animations/PipelineItemHeightAnimation.h"
#include "SVWidgetsLib/Animations/PipelineItemSlideAnimation.h"
#include "SVWidgetsLib/Core/FilterWidgetManager.h"
#include "SVWidgetsLib/Core/SVWidgetsLibConstants.h"
#include "SVWidgetsLib/FilterParameterWidgets/FilterParameterWidgetsDialogs.h"
#include "SVWidgetsLib/QtSupport/QtSRecentFileList.h"
#include "SVWidgetsLib/QtSupport/QtSStyles.h"
#include "SVWidgetsLib/Widgets/AddFilterToPipelineCommand.h"
#include "SVWidgetsLib/Widgets/DataStructureWidget.h"
#include "SVWidgetsLib/Widgets/FilterInputWidget.h"
#include "SVWidgetsLib/Widgets/PipelineFilterMimeData.h"
#include "SVWidgetsLib/Widgets/SVPipelineTreeViewDelegate.h"
#include "SVWidgetsLib/Widgets/PipelineModel.h"
#include "SVWidgetsLib/Widgets/PipelineViewController.h"
#include "SVWidgetsLib/Widgets/ProgressDialog.h"
#include "SVWidgetsLib/Widgets/AddFilterToPipelineCommand.h"
#include "SVWidgetsLib/Widgets/RemoveFilterFromPipelineCommand.h"
#include "SVWidgetsLib/Widgets/IssuesWidget.h"
#include "SVWidgetsLib/Widgets/StandardOutputWidget.h"
#include "SVWidgetsLib/Widgets/SVStyle.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
SVPipelineTreeView::SVPipelineTreeView(QWidget* parent)
: QTreeView(parent)
, PipelineView()
, m_PipelineIsRunning(false)
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

  SVPipelineTreeViewDelegate* delegate = new SVPipelineTreeViewDelegate(this);
  setItemDelegate(delegate);

  // Create the model
  PipelineModel* model = new PipelineModel(this);
  model->setUseModelDisplayText(false);
  setModel(model);

  connectSignalsSlots();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineTreeView::connectSignalsSlots()
{
  connect(this, &SVPipelineTreeView::customContextMenuRequested, this, &SVPipelineTreeView::requestContextMenu);

  connect(selectionModel(), &QItemSelectionModel::selectionChanged, [=](const QItemSelection& selected, const QItemSelection& deselected) {
    Q_UNUSED(deselected)
    getPipelineViewController()->setCutCopyEnabled(selected.size() > 0);
  });
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineTreeView::addPipelineMessageObserver(QObject* pipelineMessageObserver)
{
  if(getPipelineViewController())
  {
    getPipelineViewController()->addPipelineMessageObserver(pipelineMessageObserver);
  }
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
int SVPipelineTreeView::filterCount()
{
  PipelineModel* model = getPipelineModel();
  int count = model->rowCount();
  if(m_DropIndicatorIndex.isValid())
  {
    count--;
  }

  return count;
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

  QRect firstSelectionRect = visualRect(selectedIndexes[0]);

  if(modifiers.testFlag(Qt::AltModifier) == false)
  {
    m_MoveCommand = new QUndoCommand();

    FilterPipeline::Pointer pipeline = model->tempPipeline(getPipelineModel()->getActivePipeline());
    RemoveFilterFromPipelineCommand* cmd = new RemoveFilterFromPipelineCommand(filters, pipeline, getPipelineModel(), m_MoveCommand);
    if(filters.size() == 1)
    {
      cmd->setText(QObject::tr("\"%1 '%2' from pipeline '%3'\"").arg("Remove").arg(filters[0]->getHumanLabel()).arg(pipeline->getName()));
    }
    else
    {
      cmd->setText(QObject::tr("\"%1 %2 filters from pipeline '%3'\"").arg("Remove").arg(filters.size()).arg(pipeline->getName()));
    }
    m_MoveCommand->setText(cmd->text());

    int dropIndicatorRow = currentIndex().row();

    QString dropIndicatorText;
    if(selectedIndexes.size() == 1)
    {
      AbstractFilter::Pointer filter = model->filter(selectedIndexes[0]);
      dropIndicatorText = filter->getHumanLabel();
    }
    else
    {
      dropIndicatorText = QObject::tr("Place %1 Filters Here").arg(selectedIndexes.size());
    }

    if(getPipelineViewController())
    {
      getPipelineViewController()->addUndoCommand(m_MoveCommand);
    }

    addDropIndicator(dropIndicatorText, dropIndicatorRow);
  }

  QDrag* drag = new QDrag(this);
  drag->setMimeData(mimeData);
  drag->setPixmap(dragPixmap);
  QPoint dragPos(event->pos().x() - firstSelectionRect.x(), event->pos().y() - firstSelectionRect.y());
  drag->setHotSpot(dragPos);

  if(modifiers.testFlag(Qt::AltModifier))
  {
    drag->exec(Qt::CopyAction);
  }
  else
  {
    drag->exec(Qt::MoveAction);
  }

  removeDropIndicator();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineTreeView::dragMoveEvent(QDragMoveEvent* event)
{
//  PipelineModel* model = getPipelineModel();

//  QString dropIndicatorText;
//  const QMimeData* mimedata = event->mimeData();
//  const PipelineFilterMimeData* filterData = dynamic_cast<const PipelineFilterMimeData*>(mimedata);
//  if(filterData != nullptr)
//  {
//    // This drag has filter data, so set the appropriate drop indicator text
//    std::vector<PipelineFilterMimeData::FilterDragMetadata> dragData = filterData->getFilterDragData();
//    if(dragData.size() == 1)
//    {
//      AbstractFilter::Pointer filter = dragData[0].first;
//      dropIndicatorText = filter->getHumanLabel();
//    }
//    else
//    {
//      dropIndicatorText = QObject::tr("Place %1 Filters Here").arg(dragData.size());
//    }
//  }
//  else if(mimedata->hasUrls())
//  {
//    // This drag has URL data, so set the appropriate drop indicator text
//    QString data = mimedata->text();
//    QUrl url(data);
//    QString filePath = url.toLocalFile();

//    QFileInfo fi(filePath);
//    dropIndicatorText = QObject::tr("Place '%1' Here").arg(fi.baseName());
//  }
//  else if(mimedata->hasFormat(SIMPLView::DragAndDrop::BookmarkItem))
//  {
//    // This drag has Bookmark data, so set the appropriate drop indicator text
//    QByteArray jsonArray = mimedata->data(SIMPLView::DragAndDrop::BookmarkItem);
//    QJsonDocument doc = QJsonDocument::fromJson(jsonArray);
//    QJsonObject obj = doc.object();

//    if(obj.size() > 1)
//    {
//      event->ignore();
//      return;
//    }

//    QJsonObject::iterator iter = obj.begin();
//    QString filePath = iter.value().toString();

//    QFileInfo fi(filePath);
//    if(fi.isDir() == true)
//    {
//      event->ignore();
//      return;
//    }

//    dropIndicatorText = QObject::tr("Place '%1' Here").arg(fi.baseName());
//  }
//  else if(mimedata->hasFormat(SIMPLView::DragAndDrop::FilterListItem))
//  {
//    // This drag has Filter List data, so set the appropriate drop indicator text
//    QByteArray jsonArray = mimedata->data(SIMPLView::DragAndDrop::FilterListItem);
//    QJsonDocument doc = QJsonDocument::fromJson(jsonArray);
//    QJsonObject obj = doc.object();
//    QJsonObject::iterator iter = obj.begin();
//    QString filterClassName = iter.value().toString();

//    FilterManager* fm = FilterManager::Instance();
//    if(nullptr == fm)
//    {
//      event->ignore();
//      return;
//    }

//    IFilterFactory::Pointer wf = fm->getFactoryFromClassName(filterClassName);
//    if(nullptr == wf)
//    {
//      event->ignore();
//      return;
//    }

//    AbstractFilter::Pointer filter = wf->create();
//    dropIndicatorText = filter->getHumanLabel();
//  }
//  else
//  {
//    // We don't know what type of data this drag is, so ignore the event
//    event->ignore();
//    return;
//  }

//  QPoint mousePos = event->pos();

//  QModelIndex index = this->indexAt(mousePos);

//  int itemHeight = sizeHintForRow(0);

//  QModelIndex lastIndex = model->index(model->rowCount() - 1, PipelineItem::Contents);
//  QRect lastIndexRect = visualRect(lastIndex);

//  PipelineItem::ItemType itemType = static_cast<PipelineItem::ItemType>(model->data(index, PipelineModel::ItemTypeRole).toInt());

//  if(index.isValid() == false)
//  {
//    int dropIndicatorRow;
//    if(mousePos.y() > lastIndexRect.y())
//    {
//      // The drag is occurring in an empty space at the end of the view
//      dropIndicatorRow = filterCount();
//    }
//    else
//    {
//      // The drag is occurring in an empty space between filters
//      dropIndicatorRow = findPreviousRow(mousePos);
//    }

//    if(m_DropIndicatorIndex.isValid() && dropIndicatorRow != m_DropIndicatorIndex.row())
//    {
//      removeDropIndicator();
//      addDropIndicator(dropIndicatorText, dropIndicatorRow);
//    }
//    else if(m_DropIndicatorIndex.isValid() == false)
//    {
//      addDropIndicator(dropIndicatorText, dropIndicatorRow);
//    }
//  }
//  else if(itemType == PipelineItem::ItemType::Filter)
//  {
//    // The drag is occurring on top of a filter item
//    QRect indexRect = visualRect(index);

//    int dropIndicatorRow;
//    if(mousePos.y() <= indexRect.y() + itemHeight / 2)
//    {
//      // The drag is in the upper half of the item
//      dropIndicatorRow = index.row();
//    }
//    else
//    {
//      // The drag is in the lower half of the item
//      dropIndicatorRow = index.row() + 1;
//    }

//    removeDropIndicator();
//    addDropIndicator(dropIndicatorText, dropIndicatorRow);
//  }

  QTreeView::dragMoveEvent(event);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineTreeView::dragEnterEvent(QDragEnterEvent* event)
{
  event->accept();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineTreeView::dragLeaveEvent(QDragLeaveEvent* event)
{
  removeDropIndicator();

  event->accept();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineTreeView::addDropIndicator(const QString& text, int insertIndex)
{
  PipelineModel* model = getPipelineModel();

  model->insertRow(insertIndex);
  QModelIndex dropIndicatorIndex = model->index(insertIndex, PipelineItem::Contents);
  model->setData(dropIndicatorIndex, static_cast<int>(PipelineItem::ItemType::DropIndicator), PipelineModel::ItemTypeRole);
  model->setDropIndicatorText(dropIndicatorIndex, text);

  //  QRect rect = visualRect(dropIndicatorIndex);
  //  model->setData(dropIndicatorIndex, rect.height(), PipelineModel::Roles::HeightRole);

  m_DropIndicatorIndex = dropIndicatorIndex;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineTreeView::removeDropIndicator()
{
  if(m_DropIndicatorIndex.isValid())
  {
    PipelineModel* model = getPipelineModel();
    model->removeRow(m_DropIndicatorIndex.row());
    m_DropIndicatorIndex = QModelIndex();
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineTreeView::dropEvent(QDropEvent* event)
{
  PipelineModel* model = getPipelineModel();

  int dropRow = m_DropIndicatorIndex.row();

  removeDropIndicator();

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

    Qt::KeyboardModifiers modifiers = QApplication::queryKeyboardModifiers();
    if(event->source() == this && modifiers.testFlag(Qt::AltModifier) == false)
    {
      FilterPipeline::Pointer pipeline = model->tempPipeline(getPipelineModel()->getActivePipeline());

      // This is an internal move, so we need to create an Add command and add it as a child to the overall move command.
      AddFilterToPipelineCommand* cmd = new AddFilterToPipelineCommand(filters, pipeline, dropRow, model, m_MoveCommand);
      if(filters.size() == 1)
      {
        cmd->setText(QObject::tr("\"%1 '%2' to pipeline '%3'\"").arg("Move").arg(filters[0]->getHumanLabel()).arg(pipeline->getName()));
      }
      else
      {
        cmd->setText(QObject::tr("\"%1 %2 filters to pipeline '%3'\"").arg("Move").arg(filters.size()).arg(pipeline->getName()));
      }

      // Set the text of the drag command
      QString text = cmd->text();

      if(m_MoveCommand)
      {
        m_MoveCommand->setText(text);
      }

      // The overall drag command already has a child command that removed the filters initially, and
      // has already been placed on the undo stack and executed.  This new child command needs to be executed
      // so that it matches up with the state of its parent command.
      cmd->redo();

      clearSelection();

      QModelIndex leftIndex = model->index(dropRow, PipelineItem::Contents);
      QModelIndex rightIndex = model->index(dropRow + filters.size() - 1, PipelineItem::Contents);
      QItemSelection selection(leftIndex, rightIndex);

      selectionModel()->select(selection, QItemSelectionModel::Select);
    }
    else
    {
      addFilters(filters, dropRow);
    }

    event->accept();
  }
  else if(mimedata->hasUrls())
  {
    QString data = mimedata->text();
    QUrl url(data);
    QString filePath = url.toLocalFile();

    int err = openPipeline(filePath, dropRow);

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

    int err = openPipeline(filePath, dropRow);

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
    addFilter(filter, dropRow);

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
void SVPipelineTreeView::keyPressEvent(QKeyEvent* event)
{
  if(event->key() == Qt::Key_Backspace || event->key() == Qt::Key_Delete)
  {
    bool isRunning = getPipelineIsRunning();
    if(isRunning == false)
    {
      QModelIndexList selectedIndexes = selectionModel()->selectedRows();
      if(selectedIndexes.size() <= 0)
      {
        return;
      }

      qSort(selectedIndexes);

      PipelineModel* model = getPipelineModel();

      std::vector<AbstractFilter::Pointer> filters;
      for(int i = 0; i < selectedIndexes.size(); i++)
      {
        AbstractFilter::Pointer filter = model->filter(selectedIndexes[i]);
        filters.push_back(filter);
      }

      removeFilters(filters);
    }
  }
  else if(event->key() == Qt::Key_A && qApp->queryKeyboardModifiers() == Qt::ControlModifier)
  {
    selectAll();
  }

  QTreeView::keyPressEvent(event);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int SVPipelineTreeView::openPipeline(const QString& filePath, int insertIndex)
{
  if(getPipelineViewController())
  {
    QModelIndex pipelineRootIndex;
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

      emit filterInputWidgetNeedsCleared();
    }
  }

  QTreeView::mousePressEvent(event);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineTreeView::requestContextMenu(const QPoint& pos)
{
  activateWindow();

  QModelIndex index = indexAt(pos);
  PipelineModel* model = getPipelineModel();
  QPoint mapped;

  PipelineItem::ItemType itemType = static_cast<PipelineItem::ItemType>(model->data(index, PipelineModel::ItemTypeRole).toInt());
  if(itemType == PipelineItem::ItemType::Filter)
  {
    mapped = viewport()->mapToGlobal(pos);
  }
  else if(itemType == PipelineItem::ItemType::PipelineRoot)
  {
    mapped = viewport()->mapToGlobal(pos);
  }
  else
  {
    mapped = mapToGlobal(pos);
  }

  if(getPipelineViewController())
  {
    if(itemType == PipelineItem::ItemType::Filter)
    {
      QMenu* menu = getPipelineViewController()->getFilterItemContextMenu(index);
      menu->exec(mapped);
    }
    else if(itemType == PipelineItem::ItemType::PipelineRoot)
    {
      QMenu* menu = getPipelineViewController()->getPipelineItemContextMenu(index);
      menu->exec(mapped);
    }
    else
    {
      QMenu* menu = getPipelineViewController()->getDefaultContextMenu();
      menu->exec(mapped);
    }
  }
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

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool SVPipelineTreeView::isPipelineCurrentlyRunning()
{
  return m_PipelineIsRunning;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PipelineModel* SVPipelineTreeView::getPipelineModel()
{
  return static_cast<PipelineModel*>(model());
}
