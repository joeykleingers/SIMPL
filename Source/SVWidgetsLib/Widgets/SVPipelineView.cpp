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

#include "SVPipelineView.h"

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
#include "SVWidgetsLib/Widgets/DataStructureWidget.h"
#include "SVWidgetsLib/Widgets/FilterInputWidget.h"
#include "SVWidgetsLib/Widgets/PipelineFilterMimeData.h"
#include "SVWidgetsLib/Widgets/PipelineItemDelegate.h"
#include "SVWidgetsLib/Widgets/PipelineModel.h"
#include "SVWidgetsLib/Widgets/PipelineViewController.h"
#include "SVWidgetsLib/Widgets/ProgressDialog.h"
#include "SVWidgetsLib/Widgets/AddFilterToPipelineCommand.h"
#include "SVWidgetsLib/Widgets/RemoveFilterFromPipelineCommand.h"
#include "SVWidgetsLib/Widgets/DataStructureWidget.h"
#include "SVWidgetsLib/Widgets/ProgressDialog.h"
#include "SVWidgetsLib/Widgets/SVStyle.h"
#include "SVWidgetsLib/QtSupport/QtSRecentFileList.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
SVPipelineView::SVPipelineView(QWidget* parent)
: QListView(parent)
, PipelineView()
, m_PipelineIsRunning(false)
{
  setupGui();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
SVPipelineView::~SVPipelineView()
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineView::setupGui()
{
  setContextMenuPolicy(Qt::CustomContextMenu);
  setFocusPolicy(Qt::StrongFocus);
  setDropIndicatorShown(false);

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

  connectSignalsSlots();

  // Run this once, so that the Paste button availability is updated for what is currently on the system clipboard
  updatePasteAvailability();

  QClipboard* clipboard = QApplication::clipboard();
  connect(clipboard, &QClipboard::dataChanged, this, &SVPipelineView::updatePasteAvailability);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineView::connectSignalsSlots()
{ 
  connect(this, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(requestContextMenu(const QPoint&)));

  connect(this, &SVPipelineView::deleteKeyPressed, this, &SVPipelineView::listenDeleteKeyTriggered);

  connect(getPipelineViewController(), &PipelineViewController::pipelineDataChanged, [=] (const QModelIndex &pipelineIndex) {
    emit pipelineDataChanged();

    QModelIndexList selectedIndexes = selectionModel()->selectedRows();
    qSort(selectedIndexes);

    if(selectedIndexes.size() == 1)
    {
      QModelIndex selectedIndex = selectedIndexes[0];
      PipelineModel* model = getPipelineModel();

      AbstractFilter::Pointer filter = model->filter(selectedIndex);
      emit currentFilterUpdated(filter);
    }
    else
    {
      emit currentFilterUpdated(AbstractFilter::NullPointer());
    }

    preflightPipeline();
  });

  connect(getPipelineViewController(), &PipelineViewController::preflightFinished, this, &SVPipelineView::preflightFinished);

  connect(getPipelineViewController(), &PipelineViewController::pipelineStarted, [=](const QModelIndex &pipelineRootIndex) {
    setPipelineIsRunning(true);
    setAcceptDrops(false);
    setDragEnabled(false);
    m_ActionClearPipeline->setDisabled(true);
  });

  connect(getPipelineViewController(), &PipelineViewController::pipelineFilePathUpdated, [=] (const QString &name) { m_CurrentPipelineFilePath = name; });

  connect(selectionModel(), &QItemSelectionModel::selectionChanged, [=](const QItemSelection& selected, const QItemSelection& deselected) {
    m_ActionCut->setEnabled(selected.size() > 0);
    m_ActionCopy->setEnabled(selected.size() > 0);
  });

  connect(m_ActionCut, &QAction::triggered, this, &SVPipelineView::listenCutTriggered);
  connect(m_ActionCopy, &QAction::triggered, this, &SVPipelineView::listenCopyTriggered);
  connect(m_ActionPaste, &QAction::triggered, this, &SVPipelineView::listenPasteTriggered);

  connect(m_ActionClearPipeline, &QAction::triggered, this, &SVPipelineView::listenClearPipelineTriggered);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineView::addPipelineMessageObserver(QObject* pipelineMessageObserver)
{
  if (getPipelineViewController())
  {
    getPipelineViewController()->addPipelineMessageObserver(pipelineMessageObserver);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineView::addPipeline(FilterPipeline::Pointer pipeline, int insertIndex)
{
  if (getPipelineViewController())
  {
    getPipelineViewController()->addPipeline(pipeline, insertIndex);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineView::addFilterFromClassName(const QString& filterClassName, int insertIndex)
{
  if (getPipelineViewController())
  {
    getPipelineViewController()->addFilterFromClassName(filterClassName, insertIndex, m_PipelineRootIndex);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineView::addFilter(AbstractFilter::Pointer filter, int insertIndex)
{
  if (getPipelineViewController())
  {
    getPipelineViewController()->addFilter(filter, insertIndex, m_PipelineRootIndex);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineView::addFilters(std::vector<AbstractFilter::Pointer> filters, int insertIndex)
{
  if (getPipelineViewController())
  {
    getPipelineViewController()->addFilters(filters, insertIndex, m_PipelineRootIndex);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineView::removePipeline(FilterPipeline::Pointer pipeline)
{
  if (getPipelineViewController())
  {
    getPipelineViewController()->removePipeline(pipeline);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineView::removeFilter(AbstractFilter::Pointer filter)
{
  if (getPipelineViewController())
  {
    getPipelineViewController()->removeFilter(filter, m_PipelineRootIndex);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineView::removeFilters(std::vector<AbstractFilter::Pointer> filters)
{
  if (getPipelineViewController())
  {
    getPipelineViewController()->removeFilters(filters, m_PipelineRootIndex);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineView::cutFilter(AbstractFilter::Pointer filter)
{
  if (getPipelineViewController())
  {
    getPipelineViewController()->cutFilter(filter, m_PipelineRootIndex);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineView::cutFilters(std::vector<AbstractFilter::Pointer> filters)
{
  if (getPipelineViewController())
  {
    getPipelineViewController()->cutFilters(filters, m_PipelineRootIndex);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineView::pasteFilters(int insertIndex)
{
  if (getPipelineViewController())
  {
    getPipelineViewController()->pasteFilters(insertIndex, m_PipelineRootIndex);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineView::preflightPipeline()
{
  if (getPipelineViewController())
  {
    getPipelineViewController()->preflightPipeline(m_PipelineRootIndex);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineView::executePipeline()
{
  if (getPipelineViewController())
  {
    getPipelineViewController()->executePipeline(m_PipelineRootIndex);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineView::cancelPipeline()
{
  if (getPipelineViewController())
  {
    getPipelineViewController()->cancelPipeline(m_PipelineRootIndex);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineView::clearPipeline()
{
  if (getPipelineViewController())
  {
    getPipelineViewController()->clearPipeline(m_PipelineRootIndex);
    emit currentFilterUpdated(AbstractFilter::NullPointer());
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int SVPipelineView::writePipeline(const QModelIndex &pipelineRootIndex, const QString& outputPath)
{
  if (getPipelineViewController())
  {
    return getPipelineViewController()->writePipeline(pipelineRootIndex, outputPath);
  }

  return -1;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineView::listenCutTriggered()
{
  copySelectedFilters();

  std::vector<AbstractFilter::Pointer> filters = getSelectedFilters();
  cutFilters(filters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineView::listenCopyTriggered()
{
  copySelectedFilters();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
std::vector<AbstractFilter::Pointer> SVPipelineView::getSelectedFilters()
{
  QModelIndexList selectedIndexes = selectionModel()->selectedRows();
  qSort(selectedIndexes);

  std::vector<AbstractFilter::Pointer> filters;
  PipelineModel* model = getPipelineModel();
  for(int i = 0; i < selectedIndexes.size(); i++)
  {
    AbstractFilter::Pointer filter = model->filter(selectedIndexes[i]);
    filters.push_back(filter);
  }

  return filters;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineView::listenPasteTriggered()
{
  pasteFilters();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------s
void SVPipelineView::updatePasteAvailability()
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
void SVPipelineView::listenDeleteKeyTriggered()
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
void SVPipelineView::listenClearPipelineTriggered()
{
  clearPipeline();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int SVPipelineView::filterCount()
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
QPixmap SVPipelineView::getDraggingPixmap(QModelIndexList indexes)
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
  int dragPixmapHeight = indexPixmap.size().height() * indexes.size() + (spacing() * (indexes.size() - 1));

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
    offset = offset + indexPixmap.size().height() + spacing();
  }
  p.end();

  return dragPixmap;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineView::mouseMoveEvent(QMouseEvent* event)
{
  if((event->buttons() & Qt::LeftButton) && (event->pos() - m_DragStartPosition).manhattanLength() >= 2 && dragEnabled() == true)
  {
    beginDrag(event);
  }
  else
  {
    QListView::mouseMoveEvent(event);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineView::beginDrag(QMouseEvent* event)
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

    if (getPipelineViewController())
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
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineView::dragMoveEvent(QDragMoveEvent* event)
{
  PipelineModel* model = getPipelineModel();

  QString dropIndicatorText;
  const QMimeData* mimedata = event->mimeData();
  const PipelineFilterMimeData* filterData = dynamic_cast<const PipelineFilterMimeData*>(mimedata);
  if(filterData != nullptr)
  {
    // This drag has filter data, so set the appropriate drop indicator text
    std::vector<PipelineFilterMimeData::FilterDragMetadata> dragData = filterData->getFilterDragData();
    if(dragData.size() == 1)
    {
      AbstractFilter::Pointer filter = dragData[0].first;
      dropIndicatorText = filter->getHumanLabel();
    }
    else
    {
      dropIndicatorText = QObject::tr("Place %1 Filters Here").arg(dragData.size());
    }
  }
  else if(mimedata->hasUrls())
  {
    // This drag has URL data, so set the appropriate drop indicator text
    QString data = mimedata->text();
    QUrl url(data);
    QString filePath = url.toLocalFile();

    QFileInfo fi(filePath);
    dropIndicatorText = QObject::tr("Place '%1' Here").arg(fi.baseName());
  }
  else if(mimedata->hasFormat(SIMPLView::DragAndDrop::BookmarkItem))
  {
    // This drag has Bookmark data, so set the appropriate drop indicator text
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

    QFileInfo fi(filePath);
    if(fi.isDir() == true)
    {
      event->ignore();
      return;
    }

    dropIndicatorText = QObject::tr("Place '%1' Here").arg(fi.baseName());
  }
  else if(mimedata->hasFormat(SIMPLView::DragAndDrop::FilterListItem))
  {
    // This drag has Filter List data, so set the appropriate drop indicator text
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
    dropIndicatorText = filter->getHumanLabel();
  }
  else
  {
    // We don't know what type of data this drag is, so ignore the event
    event->ignore();
    return;
  }

  QPoint mousePos = event->pos();

  QModelIndex index = this->indexAt(mousePos);

  int itemHeight = sizeHintForRow(0);

  QModelIndex lastIndex = model->index(model->rowCount() - 1, PipelineItem::Contents);
  QRect lastIndexRect = visualRect(lastIndex);

  PipelineItem::ItemType itemType = static_cast<PipelineItem::ItemType>(model->data(index, PipelineModel::ItemTypeRole).toInt());

  if(index.isValid() == false)
  {
    int dropIndicatorRow;
    if(mousePos.y() > lastIndexRect.y())
    {
      // The drag is occurring in an empty space at the end of the view
      dropIndicatorRow = filterCount();
    }
    else
    {
      // The drag is occurring in an empty space between filters
      dropIndicatorRow = findPreviousRow(mousePos);
    }

    if(m_DropIndicatorIndex.isValid() && dropIndicatorRow != m_DropIndicatorIndex.row())
    {
      removeDropIndicator();
      addDropIndicator(dropIndicatorText, dropIndicatorRow);
    }
    else if(m_DropIndicatorIndex.isValid() == false)
    {
      addDropIndicator(dropIndicatorText, dropIndicatorRow);
    }
  }
  else if(itemType == PipelineItem::ItemType::Filter)
  {
    // The drag is occurring on top of a filter item
    QRect indexRect = visualRect(index);

    int dropIndicatorRow;
    if(mousePos.y() <= indexRect.y() + itemHeight / 2)
    {
      // The drag is in the upper half of the item
      dropIndicatorRow = index.row();
    }
    else
    {
      // The drag is in the lower half of the item
      dropIndicatorRow = index.row() + 1;
    }

    removeDropIndicator();
    addDropIndicator(dropIndicatorText, dropIndicatorRow);
  }

  QListView::dragMoveEvent(event);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineView::dragEnterEvent(QDragEnterEvent* event)
{
  event->accept();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineView::dragLeaveEvent(QDragLeaveEvent* event)
{
  removeDropIndicator();

  event->accept();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int SVPipelineView::findNextRow(const QPoint& pos)
{
  if(filterCount() == 0)
  {
    return 0;
  }

  int stepHeight = sizeHintForRow(0);
  if(spacing() < stepHeight)
  {
    stepHeight = spacing();
  }

  QPoint currentPos = pos;

  while(indexAt(currentPos).isValid() == false && currentPos.y() <= viewport()->size().height())
  {
    currentPos.setY(currentPos.y() + stepHeight);
  }

  QModelIndex index = indexAt(currentPos);
  int nextRow;
  if(index.isValid())
  {
    nextRow = index.row();
  }
  else
  {
    nextRow = filterCount();
  }

  return nextRow;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int SVPipelineView::findPreviousRow(const QPoint& pos)
{
  if(filterCount() == 0)
  {
    return 0;
  }

  int stepHeight = sizeHintForRow(0);
  if(spacing() < stepHeight)
  {
    stepHeight = spacing();
  }

  QPoint currentPos = pos;

  while(indexAt(currentPos).isValid() == false && currentPos.y() >= 0)
  {
    currentPos.setY(currentPos.y() - stepHeight);
  }

  QModelIndex index = indexAt(currentPos);
  if(index.isValid())
  {
    return index.row();
  }
  else
  {
    return 0;
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineView::addDropIndicator(const QString& text, int insertIndex)
{
  PipelineModel* model = getPipelineModel();

  model->insertRow(insertIndex);
  QModelIndex dropIndicatorIndex = model->index(insertIndex, PipelineItem::Contents);
  model->setData(dropIndicatorIndex, static_cast<int>(PipelineItem::ItemType::DropIndicator), PipelineModel::ItemTypeRole);
  model->setDropIndicatorText(dropIndicatorIndex, text);

  QRect rect = visualRect(dropIndicatorIndex);
//  model->setData(dropIndicatorIndex, rect.height(), PipelineModel::Roles::HeightRole);

  m_DropIndicatorIndex = dropIndicatorIndex;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineView::removeDropIndicator()
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
void SVPipelineView::dropEvent(QDropEvent* event)
{
  PipelineModel* model = getPipelineModel();

  int dropRow = m_DropIndicatorIndex.row();

  removeDropIndicator();

  const QMimeData* mimedata = event->mimeData();
  const PipelineFilterMimeData* filterData = dynamic_cast<const PipelineFilterMimeData*>(mimedata);
  if(filterData != nullptr)
  {
    // This is filter data from an SVPipelineView instance
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

      m_MoveCommand->setText(text);

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
void SVPipelineView::setFiltersEnabled(QModelIndexList indexes, bool enabled)
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
void SVPipelineView::setSelectedFiltersEnabled(bool enabled)
{
  QModelIndexList indexes = selectionModel()->selectedRows();
  qSort(indexes);
  setFiltersEnabled(indexes, enabled);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineView::keyPressEvent(QKeyEvent* event)
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

  QListView::keyPressEvent(event);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int SVPipelineView::openPipeline(const QString& filePath, int insertIndex)
{
  if (getPipelineViewController())
  {
    return getPipelineViewController()->openPipeline(filePath, insertIndex, m_PipelineRootIndex);
  }

  return -1;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineView::mousePressEvent(QMouseEvent* event)
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

  QListView::mousePressEvent(event);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineView::requestContextMenu(const QPoint& pos)
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
void SVPipelineView::requestFilterItemContextMenu(const QPoint& pos, const QModelIndex& index)
{
  if (getPipelineViewController())
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

    menu.addSeparator();

    menu.addAction(m_ActionClearPipeline);

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
void SVPipelineView::requestPipelineItemContextMenu(const QPoint& pos)
{
  if (getPipelineViewController())
  {
    QMenu menu;

    menu.addAction(m_ActionPaste);

    requestSinglePipelineContextMenu(menu);

    menu.exec(pos);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineView::requestSinglePipelineContextMenu(QMenu& menu)
{
  if (getPipelineViewController())
  {
    menu.addSeparator();

    menu.addAction(m_ActionClearPipeline);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineView::requestDefaultContextMenu(const QPoint& pos)
{
  if (getPipelineViewController())
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
void SVPipelineView::setModel(QAbstractItemModel* model)
{
  PipelineModel* oldModel = dynamic_cast<PipelineModel*>(this->model());
  if(oldModel)
  {
    PipelineViewController* pipelineViewController = getPipelineViewController();

    disconnect(getPipelineViewController(), &PipelineViewController::pipelineFinished, 0, 0);

    disconnect(oldModel, &PipelineModel::rowsInserted, 0, 0);

    disconnect(oldModel, &PipelineModel::rowsRemoved, 0, 0);

    disconnect(oldModel, &PipelineModel::rowsMoved, 0, 0);

    delete oldModel;
    pipelineViewController->setPipelineModel(nullptr);
  }

  QListView::setModel(model);

  PipelineModel* pipelineModel = dynamic_cast<PipelineModel*>(model);

  if(pipelineModel != nullptr)
  {
    pipelineModel->insertRow(0);
    m_PipelineRootIndex = pipelineModel->index(0, PipelineItem::Contents);
    pipelineModel->setData(m_PipelineRootIndex, static_cast<int>(PipelineItem::ItemType::PipelineRoot), PipelineModel::Roles::ItemTypeRole);
    setRootIndex(m_PipelineRootIndex);

    PipelineViewController* pipelineViewController = getPipelineViewController();
    pipelineViewController->setPipelineModel(pipelineModel);

    connect(getPipelineViewController(), &PipelineViewController::pipelineFinished, [=](const QModelIndex &pipelineRootIndex) {
      setPipelineIsRunning(false);
      setAcceptDrops(true);
      setDragEnabled(true);
      m_ActionClearPipeline->setEnabled(pipelineModel->rowCount() > 0);
      emit pipelineFinished();
    });

    connect(pipelineModel, &PipelineModel::rowsInserted, [=] { m_ActionClearPipeline->setEnabled(true); });

    connect(pipelineModel, &PipelineModel::rowsRemoved, [=] { m_ActionClearPipeline->setEnabled(pipelineModel->rowCount() > 0); });

    connect(pipelineModel, &PipelineModel::rowsMoved, [=] { m_ActionClearPipeline->setEnabled(pipelineModel->rowCount() > 0); });

    m_ActionClearPipeline->setEnabled(pipelineModel->rowCount() > 0);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool SVPipelineView::isPipelineCurrentlyRunning()
{
  return m_PipelineIsRunning;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PipelineModel* SVPipelineView::getPipelineModel()
{
  return static_cast<PipelineModel*>(model());
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QPixmap SVPipelineView::getDisableBtnPixmap(bool highlighted)
{
  if(m_DisableBtnPixmap.isNull())
  {
    m_DisableBtnPixmap = QPixmap(":/SIMPL/icons/images/ban.png");
    m_DisableHighlightedPixmap = m_DisableBtnPixmap;
  }

  QColor highlightedTextColor = palette().color(QPalette::HighlightedText);
  if(highlighted && m_DisableBtnColor != highlightedTextColor)
  {
    m_DisableBtnColor = highlightedTextColor;
    m_DisableHighlightedPixmap = setPixmapColor(m_DisableHighlightedPixmap, m_DisableBtnColor);
  }

  if(highlighted)
  {
    return m_DisableHighlightedPixmap;
  }
  else
  {
    return m_DisableBtnPixmap;
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QPixmap SVPipelineView::getHighDPIDisableBtnPixmap(bool highlighted)
{
  if(m_DisableBtnPixmap2x.isNull())
  {
    m_DisableBtnPixmap2x = QPixmap(":/SIMPL/icons/images/ban@2x.png");
    m_DisableBtnHighlightedPixmap2x = m_DisableBtnPixmap2x;
  }

  QColor highlightedTextColor = palette().color(QPalette::HighlightedText);
  if(highlighted && m_DisableBtn2xColor != highlightedTextColor)
  {
    m_DisableBtn2xColor = highlightedTextColor;
    m_DisableBtnHighlightedPixmap2x = setPixmapColor(m_DisableBtnHighlightedPixmap2x, m_DisableBtn2xColor);
  }

  if(highlighted)
  {
    return m_DisableBtnHighlightedPixmap2x;
  }
  else
  {
    return m_DisableBtnPixmap2x;
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QPixmap SVPipelineView::getDisableBtnActivatedPixmap(bool highlighted)
{
  Q_UNUSED(highlighted)

  if(m_DisableBtnActivatedPixmap.isNull())
  {
    m_DisableBtnActivatedPixmap = QPixmap(":/SIMPL/icons/images/ban_red.png");
  }

  return m_DisableBtnActivatedPixmap;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QPixmap SVPipelineView::getHighDPIDisableBtnActivatedPixmap(bool highlighted)
{
  Q_UNUSED(highlighted)

  if(m_DisableBtnActivatedPixmap2x.isNull())
  {
    m_DisableBtnActivatedPixmap2x = QPixmap(":/SIMPL/icons/images/ban_red@2x.png");
  }

  return m_DisableBtnActivatedPixmap2x;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QPixmap SVPipelineView::getDisableBtnHoveredPixmap(bool highlighted)
{
  if(m_DisableBtnHoveredPixmap.isNull())
  {
    m_DisableBtnHoveredPixmap = QPixmap(":/SIMPL/icons/images/ban_hover.png");
    m_DisableBtnHoveredHighlightedPixmap = m_DisableBtnHoveredPixmap;
  }

  QColor highlightedTextColor = palette().color(QPalette::HighlightedText);
  if(m_DisableBtnHoveredColor != highlightedTextColor.darker(115))
  {
    m_DisableBtnHoveredColor = highlightedTextColor.darker(115);
    m_DisableBtnHoveredHighlightedPixmap = setPixmapColor(m_DisableBtnHoveredHighlightedPixmap, m_DisableBtnHoveredColor);
  }

  if(highlighted)
  {
    return m_DisableBtnHoveredHighlightedPixmap;
  }
  else
  {
    return m_DisableBtnHoveredPixmap;
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QPixmap SVPipelineView::getHighDPIDisableBtnHoveredPixmap(bool highlighted)
{
  if(m_DisableBtnHoveredPixmap2x.isNull())
  {
    m_DisableBtnHoveredPixmap2x = QPixmap(":/SIMPL/icons/images/ban_hover@2x.png");
    m_DisableBtnHoveredHighlightedPixmap2x = m_DisableBtnHoveredPixmap2x;
  }

  QColor highlightedTextColor = palette().color(QPalette::HighlightedText);
  if(m_DisableBtnHovered2xColor != highlightedTextColor.darker(115))
  {
    m_DisableBtnHovered2xColor = highlightedTextColor.darker(115);
    m_DisableBtnHoveredHighlightedPixmap2x = setPixmapColor(m_DisableBtnHoveredHighlightedPixmap2x, m_DisableBtnHovered2xColor);
  }

  if(highlighted)
  {
    return m_DisableBtnHoveredHighlightedPixmap2x;
  }
  else
  {
    return m_DisableBtnHoveredPixmap2x;
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QPixmap SVPipelineView::getDeleteBtnPixmap(bool highlighted)
{
  if(m_DeleteBtnPixmap.isNull())
  {
    m_DeleteBtnPixmap = QPixmap(":/SIMPL/icons/images/trash.png");
    m_DeleteBtnHighlightedPixmap = m_DeleteBtnPixmap;
  }

  QColor highlightedTextColor = palette().color(QPalette::HighlightedText);
  if(m_DeleteBtnColor != highlightedTextColor)
  {
    m_DeleteBtnColor = highlightedTextColor;
    m_DeleteBtnHighlightedPixmap = setPixmapColor(m_DeleteBtnHighlightedPixmap, m_DeleteBtnColor);
  }

  if(highlighted)
  {
    return m_DeleteBtnHighlightedPixmap;
  }
  else
  {
    return m_DeleteBtnPixmap;
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QPixmap SVPipelineView::getHighDPIDeleteBtnPixmap(bool highlighted)
{
  if(m_DeleteBtnPixmap2x.isNull())
  {
    m_DeleteBtnPixmap2x = QPixmap(":/SIMPL/icons/images/trash@2x.png");
    m_DeleteBtnHighlightedPixmap2x = m_DeleteBtnPixmap2x;
  }

  QColor highlightedTextColor = palette().color(QPalette::HighlightedText);
  if(m_DeleteBtn2xColor != highlightedTextColor)
  {
    m_DeleteBtn2xColor = highlightedTextColor;
    m_DeleteBtnHighlightedPixmap2x = setPixmapColor(m_DeleteBtnHighlightedPixmap2x, m_DeleteBtn2xColor);
  }

  if(highlighted)
  {
    return m_DeleteBtnHighlightedPixmap2x;
  }
  else
  {
    return m_DeleteBtnPixmap2x;
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QPixmap SVPipelineView::getDeleteBtnHoveredPixmap(bool highlighted)
{
  if(m_DeleteBtnHoveredPixmap.isNull())
  {
    m_DeleteBtnHoveredPixmap = QPixmap(":/SIMPL/icons/images/trash_hover.png");
    m_DeleteBtnHoveredHighlightedPixmap = m_DeleteBtnHoveredPixmap;
  }

  QColor highlightedTextColor = palette().color(QPalette::HighlightedText);
  if(m_DeleteBtnHoveredColor != highlightedTextColor.darker(115))
  {
    m_DeleteBtnHoveredColor = highlightedTextColor.darker(115);
    m_DeleteBtnHoveredHighlightedPixmap = setPixmapColor(m_DeleteBtnHoveredHighlightedPixmap, m_DeleteBtnHoveredColor);
  }

  if(highlighted)
  {
    return m_DeleteBtnHoveredHighlightedPixmap;
  }
  else
  {
    return m_DeleteBtnHoveredPixmap;
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QPixmap SVPipelineView::getHighDPIDeleteBtnHoveredPixmap(bool highlighted)
{
  if(m_DeleteBtnHoveredPixmap2x.isNull())
  {
    m_DeleteBtnHoveredPixmap2x = QPixmap(":/SIMPL/icons/images/trash_hover@2x.png");
    m_DeleteBtnHoveredHighlightedPixmap2x = m_DeleteBtnHoveredPixmap2x;
  }

  QColor highlightedTextColor = palette().color(QPalette::HighlightedText);
  if(m_DeleteBtnHovered2xColor != highlightedTextColor.darker(115))
  {
    m_DeleteBtnHovered2xColor = highlightedTextColor.darker(115);
    m_DeleteBtnHoveredHighlightedPixmap2x = setPixmapColor(m_DeleteBtnHoveredHighlightedPixmap2x, m_DeleteBtnHovered2xColor);
  }

  if(highlighted)
  {
    return m_DeleteBtnHoveredHighlightedPixmap2x;
  }
  else
  {
    return m_DeleteBtnHoveredPixmap2x;
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QPixmap SVPipelineView::setPixmapColor(QPixmap pixmap, QColor pixmapColor)
{
  QImage image = pixmap.toImage();
  for(int y = 0; y < image.height(); y++)
  {
    for(int x = 0; x < image.width(); x++)
    {
      QColor color = pixmapColor;

      int alpha = image.pixelColor(x, y).alpha();

      color.setAlpha(alpha);

      if (color.isValid())
      {
        image.setPixelColor(x, y, color);
      }
    }
  }

  return QPixmap::fromImage(image);
}
