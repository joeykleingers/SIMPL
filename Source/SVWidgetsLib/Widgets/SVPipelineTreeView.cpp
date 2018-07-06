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
#include "SVWidgetsLib/Widgets/PipelineItemDelegate.h"
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

  // Delete action if it exists
  if(m_ActionEnableFilter)
  {
    delete m_ActionEnableFilter;
  }

  m_ActionEnableFilter = new QAction("Enable", this);
  m_ActionEnableFilter->setCheckable(true);
  m_ActionEnableFilter->setChecked(true);
  m_ActionEnableFilter->setEnabled(false);

  m_ActionCut = new QAction("Cut", this);
  m_ActionCopy = new QAction("Copy", this);
  m_ActionPaste = new QAction("Paste", this);
  m_ActionClearPipeline = new QAction("Clear Pipeline", this);

  m_ActionCut->setShortcut(QKeySequence::Cut);
  m_ActionCopy->setShortcut(QKeySequence::Copy);
  m_ActionPaste->setShortcut(QKeySequence::Paste);

  m_ActionCut->setDisabled(true);
  m_ActionCopy->setDisabled(true);

  m_ActionClearPipeline->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Backspace));

  // Create the model
  PipelineModel* model = new PipelineModel(this);
  setModel(model);

  connectSignalsSlots();

  // Run this once, so that the Paste button availability is updated for what is currently on the system clipboard
  updatePasteAvailability();

  QClipboard* clipboard = QApplication::clipboard();
  connect(clipboard, &QClipboard::dataChanged, this, &SVPipelineTreeView::updatePasteAvailability);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineTreeView::connectSignalsSlots()
{
  connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(requestContextMenu(const QPoint&)));

  connect(this, &SVPipelineTreeView::deleteKeyPressed, this, &SVPipelineTreeView::listenDeleteKeyTriggered);

  connect(getPipelineViewController(), &PipelineViewController::pipelineFinished, [=](const QModelIndex& pipelineRootIndex) {
    setPipelineIsRunning(false);
    setAcceptDrops(true);
    setDragEnabled(true);

    PipelineModel* pipelineModel = getPipelineModel();
    if (pipelineModel)
    {
      m_ActionClearPipeline->setEnabled(pipelineModel->rowCount() > 0);
    }
    else
    {
      m_ActionClearPipeline->setDisabled(true);
    }
    emit pipelineFinished();
  });

  connect(getPipelineViewController(), &PipelineViewController::pipelineStarted, [=](const QModelIndex& pipelineRootIndex) {
    setPipelineIsRunning(true);
    setAcceptDrops(false);
    setDragEnabled(false);
    m_ActionClearPipeline->setDisabled(true);
  });

  connect(getPipelineViewController(), &PipelineViewController::pipelineFilePathUpdated, [=](const QString& name) { m_CurrentPipelineFilePath = name; });

  connect(selectionModel(), &QItemSelectionModel::selectionChanged, [=](const QItemSelection& selected, const QItemSelection& deselected) {
    m_ActionCut->setEnabled(selected.size() > 0);
    m_ActionCopy->setEnabled(selected.size() > 0);
  });

  connect(m_ActionCut, &QAction::triggered, this, &SVPipelineTreeView::listenCutTriggered);
  connect(m_ActionCopy, &QAction::triggered, this, &SVPipelineTreeView::listenCopyTriggered);
  connect(m_ActionPaste, &QAction::triggered, this, &SVPipelineTreeView::listenPasteTriggered);

  connect(m_ActionClearPipeline, &QAction::triggered, this, &SVPipelineTreeView::listenClearPipelineTriggered);
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
void SVPipelineTreeView::addPipeline(FilterPipeline::Pointer pipeline, int insertIndex)
{
  if(getPipelineViewController())
  {
    getPipelineViewController()->addPipeline(pipeline, insertIndex);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineTreeView::addFilterFromClassName(const QString& filterClassName, int insertIndex)
{
  if(getPipelineViewController())
  {
    getPipelineViewController()->addFilterFromClassName(filterClassName, insertIndex, m_PipelineRootIndex);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineTreeView::addFilter(AbstractFilter::Pointer filter, int insertIndex)
{
  if(getPipelineViewController())
  {
    getPipelineViewController()->addFilter(filter, insertIndex, m_PipelineRootIndex);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineTreeView::addFilters(std::vector<AbstractFilter::Pointer> filters, int insertIndex)
{
  if(getPipelineViewController())
  {
    getPipelineViewController()->addFilters(filters, insertIndex, m_PipelineRootIndex);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineTreeView::removePipeline(FilterPipeline::Pointer pipeline)
{
  if(getPipelineViewController())
  {
    getPipelineViewController()->removePipeline(pipeline);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineTreeView::removeFilter(AbstractFilter::Pointer filter)
{
  if(getPipelineViewController())
  {
    getPipelineViewController()->removeFilter(filter, m_PipelineRootIndex);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineTreeView::removeFilters(std::vector<AbstractFilter::Pointer> filters)
{
  if(getPipelineViewController())
  {
    getPipelineViewController()->removeFilters(filters, m_PipelineRootIndex);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineTreeView::cutFilter(AbstractFilter::Pointer filter)
{
  if(getPipelineViewController())
  {
    getPipelineViewController()->cutFilter(filter, m_PipelineRootIndex);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineTreeView::cutFilters(std::vector<AbstractFilter::Pointer> filters)
{
  if(getPipelineViewController())
  {
    getPipelineViewController()->cutFilters(filters, m_PipelineRootIndex);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineTreeView::pasteFilters(int insertIndex)
{
  if(getPipelineViewController())
  {
    getPipelineViewController()->pasteFilters(insertIndex, m_PipelineRootIndex);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineTreeView::preflightPipeline()
{
  if(getPipelineViewController())
  {
    getPipelineViewController()->preflightPipeline(m_PipelineRootIndex);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineTreeView::executePipeline()
{
  if(getPipelineViewController())
  {
    getPipelineViewController()->executePipeline(m_PipelineRootIndex);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineTreeView::cancelPipeline()
{
  if(getPipelineViewController())
  {
    getPipelineViewController()->cancelPipeline(m_PipelineRootIndex);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineTreeView::clearPipeline()
{
  if(getPipelineViewController())
  {
    getPipelineViewController()->clearPipeline(m_PipelineRootIndex);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineTreeView::listenCutTriggered()
{
  copySelectedFilters();

  std::vector<AbstractFilter::Pointer> filters = getSelectedFilters();
  cutFilters(filters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineTreeView::listenCopyTriggered()
{
  copySelectedFilters();
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
void SVPipelineTreeView::listenPasteTriggered()
{
  pasteFilters();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------s
void SVPipelineTreeView::updatePasteAvailability()
{
  QClipboard* clipboard = QApplication::clipboard();
  QString text = clipboard->text();

  JsonFilterParametersReader::Pointer jsonReader = JsonFilterParametersReader::New();
  FilterPipeline::Pointer pipeline = jsonReader->readPipelineFromString(text);

  if(text.isEmpty() || FilterPipeline::NullPointer() == pipeline)
  {
    m_ActionPaste->setDisabled(true);
  }
  else
  {
    m_ActionPaste->setEnabled(true);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineTreeView::listenDeleteKeyTriggered()
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

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineTreeView::listenClearPipelineTriggered()
{
  clearPipeline();
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

  PipelineItemDelegate* delegate = dynamic_cast<PipelineItemDelegate*>(itemDelegate());
  if(delegate == nullptr)
  {
    return QPixmap();
  }

  QPixmap indexPixmap = delegate->createPixmap(indexes[0]);

  int dragPixmapWidth = indexPixmap.size().width();
  int dragPixmapHeight = indexPixmap.size().height() * indexes.size() + ((indexes.size() - 1));

  QPixmap dragPixmap(dragPixmapWidth, dragPixmapHeight);
  dragPixmap.fill(Qt::transparent);

  QPainter p;
  p.begin(&dragPixmap);
  p.setOpacity(0.70);
  int offset = 0;
  for(int i = 0; i < indexes.size(); i++)
  {
    QPixmap currentPixmap = delegate->createPixmap(indexes[i]);
    p.drawPixmap(0, offset, currentPixmap);
    offset = offset + indexPixmap.size().height();
  }
  p.end();

  return dragPixmap;
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

    FilterPipeline::Pointer pipeline = model->tempPipeline(m_PipelineRootIndex);
    RemoveFilterFromPipelineCommand* cmd = new RemoveFilterFromPipelineCommand(filters, pipeline, "Remove", m_MoveCommand);
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
      FilterPipeline::Pointer pipeline = model->tempPipeline(m_PipelineRootIndex);

      // This is an internal move, so we need to create an Add command and add it as a child to the overall move command.
      AddFilterToPipelineCommand* cmd = new AddFilterToPipelineCommand(filters, pipeline, dropRow, "Move", m_MoveCommand);

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
void SVPipelineTreeView::setFiltersEnabled(QModelIndexList indexes, bool enabled)
{
  int count = indexes.size();
  PipelineModel* model = getPipelineModel();
  for(int i = 0; i < count; i++)
  {
    QModelIndex index = indexes[i];
    AbstractFilter::Pointer filter = model->filter(index);
    if(enabled == true)
    {
      filter->setEnabled(true);
      model->setData(index, static_cast<int>(PipelineItem::WidgetState::Ready), PipelineModel::WidgetStateRole);
    }
    else
    {
      filter->setEnabled(false);
      model->setData(index, static_cast<int>(PipelineItem::WidgetState::Disabled), PipelineModel::WidgetStateRole);
    }
  }

  preflightPipeline();
  emit filterEnabledStateChanged();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineTreeView::setSelectedFiltersEnabled(bool enabled)
{
  QModelIndexList indexes = selectionModel()->selectedRows();
  qSort(indexes);
  setFiltersEnabled(indexes, enabled);
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
      emit deleteKeyPressed();
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

  if(itemType == PipelineItem::ItemType::Filter)
  {
    requestFilterItemContextMenu(mapped, index);
  }
  else if(itemType == PipelineItem::ItemType::PipelineRoot)
  {
    requestPipelineItemContextMenu(mapped);
  }
  else
  {
    requestDefaultContextMenu(mapped);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineTreeView::requestFilterItemContextMenu(const QPoint& pos, const QModelIndex& index)
{
  if(getPipelineViewController())
  {
    PipelineModel* model = getPipelineModel();
    QModelIndexList selectedIndexes = selectionModel()->selectedRows();
    qSort(selectedIndexes);

    QMenu menu;

    menu.addAction(m_ActionCut);
    menu.addAction(m_ActionCopy);
    menu.addSeparator();

    QAction* actionPasteAbove = new QAction("Paste Above", this);
    QAction* actionPasteBelow = new QAction("Paste Below", this);

    connect(actionPasteAbove, &QAction::triggered, this, [=] { pasteFilters(index.row()); });

    connect(actionPasteBelow, &QAction::triggered, this, [=] { pasteFilters(index.row() + 1); });

    menu.addAction(actionPasteAbove);
    menu.addAction(actionPasteBelow);
    menu.addSeparator();

    int count = selectedIndexes.size();
    bool widgetEnabled = true;

    for(int i = 0; i < count && widgetEnabled; i++)
    {
      AbstractFilter::Pointer filter = model->filter(selectedIndexes[i]);
      if(filter != nullptr)
      {
        widgetEnabled = filter->getEnabled();
      }
    }

    if(selectedIndexes.contains(index) == false)
    {
      // Only toggle the target filter widget if it is not in the selected objects
      QModelIndexList toggledIndices = QModelIndexList();
      toggledIndices.push_back(index);

      AbstractFilter::Pointer filter = model->filter(index);
      if(filter != nullptr)
      {
        widgetEnabled = filter->getEnabled();
      }

      disconnect(m_ActionEnableFilter, &QAction::toggled, 0, 0);
      connect(m_ActionEnableFilter, &QAction::toggled, [=] { setFiltersEnabled(toggledIndices, m_ActionEnableFilter->isChecked()); });
    }
    else
    {
      disconnect(m_ActionEnableFilter, &QAction::toggled, 0, 0);
      connect(m_ActionEnableFilter, &QAction::toggled, [=] { setFiltersEnabled(selectedIndexes, m_ActionEnableFilter->isChecked()); });
    }

    m_ActionEnableFilter->setChecked(widgetEnabled);
    m_ActionEnableFilter->setEnabled(true);
    m_ActionEnableFilter->setDisabled(getPipelineIsRunning());
    menu.addAction(m_ActionEnableFilter);

    menu.addSeparator();

    QAction* removeAction;
    QList<QKeySequence> shortcutList;
    shortcutList.push_back(QKeySequence(Qt::Key_Backspace));
    shortcutList.push_back(QKeySequence(Qt::Key_Delete));

    if(selectedIndexes.contains(index) == false || selectedIndexes.size() == 1)
    {
      removeAction = new QAction("Delete Filter", &menu);
      connect(removeAction, &QAction::triggered, [=] {
        AbstractFilter::Pointer filter = model->filter(index);
        removeFilter(filter);
      });
    }
    else
    {
      removeAction = new QAction(tr("Delete %1 Filters").arg(selectedIndexes.size()), &menu);
      connect(removeAction, &QAction::triggered, [=] {
        QList<QPersistentModelIndex> persistentList;
        for(int i = 0; i < selectedIndexes.size(); i++)
        {
          persistentList.push_back(selectedIndexes[i]);
        }

        std::vector<AbstractFilter::Pointer> filters;
        for(int i = 0; i < persistentList.size(); i++)
        {
          AbstractFilter::Pointer filter = model->filter(persistentList[i]);
          filters.push_back(filter);
        }

        removeFilters(filters);
      });
    }
    removeAction->setShortcuts(shortcutList);
    if(getPipelineIsRunning() == true)
    {
      removeAction->setDisabled(true);
    }

    menu.addAction(removeAction);

    menu.addAction(m_ActionClearPipeline);

    menu.addSeparator();

    // Error Handling Menu
    requestErrorHandlingContextMenu(menu);
    menu.addSeparator();

    QAction* actionLaunchHelp = new QAction("Filter Help", this);
    connect(actionLaunchHelp, &QAction::triggered, [=] {
      AbstractFilter::Pointer filter = model->filter(index);
      if(filter != nullptr)
      {
        // Launch the help for this filter
        QString className = filter->getNameOfClass();

        DocRequestManager* docRequester = DocRequestManager::Instance();
        docRequester->requestFilterDocs(className);
      }
    });

    menu.addAction(actionLaunchHelp);

    menu.exec(pos);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineTreeView::requestPipelineItemContextMenu(const QPoint& pos)
{
  if(getPipelineViewController())
  {
    QMenu menu;

    menu.addAction(m_ActionPaste);

    requestSinglePipelineContextMenu(menu);

    requestErrorHandlingContextMenu(menu);

    menu.exec(pos);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineTreeView::requestSinglePipelineContextMenu(QMenu& menu)
{
  if(getPipelineViewController())
  {
    menu.addSeparator();

    menu.addAction(m_ActionClearPipeline);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineTreeView::requestErrorHandlingContextMenu(QMenu& menu)
{
  menu.addSeparator();

  QMenu* errorMenu = menu.addMenu("Error Handling");

  QMenu* combinedMenu = errorMenu->addMenu("All");
  QAction* showCombinedErrorAction = combinedMenu->addAction("Show on Error");
  QAction* ignoreCombinedErrorAction = combinedMenu->addAction("Ignore on Error");

  QMenu* errorTableMenu = errorMenu->addMenu("Issues Table");
  QAction* showTableErrorAction = errorTableMenu->addAction("Show on Error");
  QAction* ignoreTableErrorAction = errorTableMenu->addAction("Ignore on Error");

  QMenu* stdOutMenu = errorMenu->addMenu("Standard Output");
  QAction* showStdOutErrorAction = stdOutMenu->addAction("Show on Error");
  QAction* ignoreStdOutErrorAction = stdOutMenu->addAction("Ignore on Error");

  menu.addSeparator();

  showTableErrorAction->setCheckable(true);
  ignoreTableErrorAction->setCheckable(true);
  showStdOutErrorAction->setCheckable(true);
  ignoreStdOutErrorAction->setCheckable(true);
  showCombinedErrorAction->setCheckable(true);
  ignoreCombinedErrorAction->setCheckable(true);

  // Set Checked based on user preferences
  SIMPLView::DockWidgetSettings::HideDockSetting issuesTableSetting = IssuesWidget::GetHideDockSetting();
  SIMPLView::DockWidgetSettings::HideDockSetting stdOutSetting = StandardOutputWidget::GetHideDockSetting();

  bool showTableError = (issuesTableSetting != SIMPLView::DockWidgetSettings::HideDockSetting::Ignore);
  bool showStdOutput = (stdOutSetting != SIMPLView::DockWidgetSettings::HideDockSetting::Ignore);
  bool showCombinedError = showTableError && showStdOutput;
  bool ignoreCombinedError = !showTableError && !showStdOutput;

  showTableErrorAction->setChecked(showTableError);
  ignoreTableErrorAction->setChecked(!showTableError);
  showStdOutErrorAction->setChecked(showStdOutput);
  ignoreStdOutErrorAction->setChecked(!showStdOutput);
  showCombinedErrorAction->setChecked(showCombinedError);
  ignoreCombinedErrorAction->setChecked(ignoreCombinedError);

  // Connect actions
  // Issues Widget
  connect(showTableErrorAction, &QAction::triggered, [=]() {
    IssuesWidget::SetHideDockSetting(SIMPLView::DockWidgetSettings::HideDockSetting::OnError);
    preflightPipeline();
  });
  connect(ignoreTableErrorAction, &QAction::triggered, [=]() {
    IssuesWidget::SetHideDockSetting(SIMPLView::DockWidgetSettings::HideDockSetting::Ignore);
    preflightPipeline();
  });

  // Standard Output
  connect(showStdOutErrorAction, &QAction::triggered, [=]() {
    StandardOutputWidget::SetHideDockSetting(SIMPLView::DockWidgetSettings::HideDockSetting::OnError);
    preflightPipeline();
  });
  connect(ignoreStdOutErrorAction, &QAction::triggered, [=]() {
    StandardOutputWidget::SetHideDockSetting(SIMPLView::DockWidgetSettings::HideDockSetting::Ignore);
    preflightPipeline();
  });

  // Combined
  connect(showCombinedErrorAction, &QAction::triggered, [=]() {
    IssuesWidget::SetHideDockSetting(SIMPLView::DockWidgetSettings::HideDockSetting::OnError);
    StandardOutputWidget::SetHideDockSetting(SIMPLView::DockWidgetSettings::HideDockSetting::OnError);
    preflightPipeline();
  });
  connect(ignoreCombinedErrorAction, &QAction::triggered, [=]() {
    IssuesWidget::SetHideDockSetting(SIMPLView::DockWidgetSettings::HideDockSetting::Ignore);
    StandardOutputWidget::SetHideDockSetting(SIMPLView::DockWidgetSettings::HideDockSetting::Ignore);
    preflightPipeline();
  });
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineTreeView::requestDefaultContextMenu(const QPoint& pos)
{
  if(getPipelineViewController())
  {
    QMenu menu;
    menu.addAction(m_ActionPaste);
    menu.addSeparator();
    menu.addAction(m_ActionClearPipeline);

    menu.exec(pos);
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

    disconnect(oldModel, &PipelineModel::rowsInserted, 0, 0);

    disconnect(oldModel, &PipelineModel::rowsRemoved, 0, 0);

    disconnect(oldModel, &PipelineModel::rowsMoved, 0, 0);

    delete oldModel;
    pipelineViewController->setPipelineModel(nullptr);
  }

  QTreeView::setModel(model);

  PipelineModel* pipelineModel = dynamic_cast<PipelineModel*>(model);

  if(pipelineModel != nullptr)
  {
    PipelineViewController* pipelineViewController = getPipelineViewController();
    pipelineViewController->setPipelineModel(pipelineModel);

    connect(pipelineModel, &PipelineModel::rowsInserted, [=] { m_ActionClearPipeline->setEnabled(true); });

    connect(pipelineModel, &PipelineModel::rowsRemoved, [=] { m_ActionClearPipeline->setEnabled(pipelineModel->rowCount() > 0); });

    connect(pipelineModel, &PipelineModel::rowsMoved, [=] { m_ActionClearPipeline->setEnabled(pipelineModel->rowCount() > 0); });

    m_ActionClearPipeline->setEnabled(pipelineModel->rowCount() > 0);
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
