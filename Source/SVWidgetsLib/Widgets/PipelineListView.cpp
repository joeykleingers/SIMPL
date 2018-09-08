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

#include "PipelineListView.h"

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
#include "SVWidgetsLib/Widgets/PipelineListViewDelegate.h"
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
PipelineListView::PipelineListView(QWidget* parent)
: QListView(parent)
, PipelineView()
, m_PipelineIsRunning(false)
{
  setupGui();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PipelineListView::~PipelineListView()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineListView::setupGui()
{
//  PipelineListViewDelegate* delegate = new PipelineListViewDelegate(this);
//  setItemDelegate(delegate);

  // Create the model
  PipelineModel* model = new PipelineModel(1, this);
  setModel(model);

  getPipelineViewController()->setAbstractPipelineView(this);

  connectSignalsSlots();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineListView::connectSignalsSlots()
{
  connect(getPipelineViewController(), &PipelineViewController::pipelineStarted, [=] { setPipelineViewState(PipelineViewState::Running); });
  connect(getPipelineViewController(), &PipelineViewController::pipelineCanceling, [=] { setPipelineViewState(PipelineViewState::Cancelling); });
  connect(getPipelineViewController(), &PipelineViewController::pipelineFinished, [=] { setPipelineViewState(PipelineViewState::Idle); });
  connect(getPipelineViewController(), &PipelineViewController::pipelineCanceled, [=] { setPipelineViewState(PipelineViewState::Idle); });
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineListView::setPipelineViewState(PipelineView::PipelineViewState state)
{
  setPipelineState(state);

  if (state == PipelineViewState::Idle)
  {
    setPipelineIsRunning(false);
    setAcceptDrops(true);
    setDragEnabled(true);
  }
  else if (state == PipelineViewState::Running)
  {
    setPipelineIsRunning(true);
    setAcceptDrops(false);
    setDragEnabled(false);
  }
  else
  {

  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineListView::addPipeline(FilterPipeline::Pointer pipeline, int insertIndex)
{
  if(getPipelineViewController())
  {
    getPipelineViewController()->addPipeline(pipeline, insertIndex);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineListView::removePipeline(FilterPipeline::Pointer pipeline)
{
  if(getPipelineViewController())
  {
    getPipelineViewController()->removePipeline(pipeline);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QModelIndexList PipelineListView::getSelectedRows()
{
  return selectionModel()->selectedRows();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QPixmap PipelineListView::getDraggingPixmap(QModelIndexList indexes)
{
  if(indexes.size() <= 0)
  {
    return QPixmap();
  }

  PipelineListViewDelegate* delegate = dynamic_cast<PipelineListViewDelegate*>(itemDelegate());
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
void PipelineListView::mouseMoveEvent(QMouseEvent* event)
{
  if((event->buttons() & Qt::LeftButton) && (event->pos() - m_DragStartPosition).manhattanLength() >= QApplication::startDragDistance() + 1 && dragEnabled() == true)
  {
    beginDrag(event);
  }

  QListView::mouseMoveEvent(event);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineListView::beginDrag(QMouseEvent* event)
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

  if(modifiers.testFlag(Qt::AltModifier) == false)
  {
    m_MoveCommand = new QUndoCommand();

    FilterPipeline::Pointer pipeline = model->tempPipeline(m_PipelineRootIndex);
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

    if(getPipelineViewController())
    {
      getPipelineViewController()->addUndoCommand(m_MoveCommand);
    }
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
int PipelineListView::findNextRow(const QPoint& pos)
{
  if(filterCount(m_PipelineRootIndex) == 0)
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
    nextRow = filterCount(m_PipelineRootIndex);
  }

  return nextRow;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int PipelineListView::findPreviousRow(const QPoint& pos)
{
  if(filterCount(m_PipelineRootIndex) == 0)
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
void PipelineListView::dropEvent(QDropEvent* event)
{
  PipelineModel* model = getPipelineModel();

  DropIndicatorPosition dropIndicatorPos = dropIndicatorPosition();
  QModelIndex nearestIndex = indexAt(event->pos());

  int dropRow = filterCount(m_PipelineRootIndex);
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
    // This is filter data from an PipelineListView instance
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

      QModelIndex leftIndex = model->index(dropRow, PipelineItem::Contents, m_PipelineRootIndex);
      QModelIndex rightIndex = model->index(dropRow + filters.size() - 1, PipelineItem::Contents, m_PipelineRootIndex);
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

    int err = openPipeline(filePath, m_PipelineRootIndex, dropRow);

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

    int err = openPipeline(filePath, m_PipelineRootIndex, dropRow);

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
int PipelineListView::openPipeline(const QString& filePath, QModelIndex pipelineRootIndex, int insertIndex)
{
  int err = -1;
  if(getPipelineViewController())
  {
    QModelIndex originalPipelineRootIndex = pipelineRootIndex;
    err = getPipelineViewController()->openPipeline(filePath, pipelineRootIndex, insertIndex);

    if (err >= 0 && originalPipelineRootIndex.isValid() == false)
    {
      PipelineModel* model = getPipelineModel();
      if (model->rowCount() > 0)
      {
        QModelIndex index = model->index(0, PipelineItem::PipelineItemData::Contents, pipelineRootIndex);
        selectionModel()->select(index, QItemSelectionModel::SelectCurrent);
      }
    }
  }

  return err;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineListView::mousePressEvent(QMouseEvent* event)
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
void PipelineListView::requestContextMenu(const QPoint& pos)
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
    QMenu menu;
    if(itemType == PipelineItem::ItemType::Filter)
    {
      getPipelineViewController()->getFilterItemContextMenu(menu, index);
      menu.exec(pos);
    }
    else if(itemType == PipelineItem::ItemType::PipelineRoot)
    {
      getPipelineViewController()->getPipelineItemContextMenu(menu, index);
      menu.exec(pos);
    }
    else
    {
      getPipelineViewController()->getDefaultContextMenu(menu);
      menu.exec(pos);
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineListView::setModel(QAbstractItemModel* model)
{
  PipelineModel* oldModel = dynamic_cast<PipelineModel*>(this->model());
  if(oldModel)
  {
    PipelineViewController* pipelineViewController = getPipelineViewController();

    delete oldModel;
    pipelineViewController->setPipelineModel(nullptr);
  }

  QListView::setModel(model);

  PipelineModel* pipelineModel = dynamic_cast<PipelineModel*>(model);

  if(pipelineModel != nullptr)
  {
    PipelineViewController* pipelineViewController = getPipelineViewController();
    pipelineViewController->setPipelineModel(pipelineModel);

    connect(pipelineModel, &PipelineModel::pipelineAdded, [=](FilterPipeline::Pointer pipeline, const QModelIndex &pipelineRootIndex) {
      Q_UNUSED(pipeline)
      setRootIndex(pipelineRootIndex);
    });
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool PipelineListView::isPipelineCurrentlyRunning()
{
  return (getPipelineState() == PipelineViewState::Running);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PipelineModel* PipelineListView::getPipelineModel()
{
  return static_cast<PipelineModel*>(model());
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QPixmap PipelineListView::getDisableBtnPixmap(bool highlighted)
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
QPixmap PipelineListView::getHighDPIDisableBtnPixmap(bool highlighted)
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
QPixmap PipelineListView::getDisableBtnActivatedPixmap(bool highlighted)
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
QPixmap PipelineListView::getHighDPIDisableBtnActivatedPixmap(bool highlighted)
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
QPixmap PipelineListView::getDisableBtnHoveredPixmap(bool highlighted)
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
QPixmap PipelineListView::getHighDPIDisableBtnHoveredPixmap(bool highlighted)
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
QPixmap PipelineListView::getDeleteBtnPixmap(bool highlighted)
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
QPixmap PipelineListView::getHighDPIDeleteBtnPixmap(bool highlighted)
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
QPixmap PipelineListView::getDeleteBtnHoveredPixmap(bool highlighted)
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
QPixmap PipelineListView::getHighDPIDeleteBtnHoveredPixmap(bool highlighted)
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
QPixmap PipelineListView::setPixmapColor(QPixmap pixmap, QColor pixmapColor)
{
  QImage image = pixmap.toImage();
  for(int y = 0; y < image.height(); y++)
  {
    for(int x = 0; x < image.width(); x++)
    {
      QColor color = pixmapColor;

      int alpha = image.pixelColor(x, y).alpha();

      color.setAlpha(alpha);

      if(color.isValid())
      {
        image.setPixelColor(x, y, color);
      }
    }
  }

  return QPixmap::fromImage(image);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PipelineListViewDelegate* PipelineListView::getViewDelegate()
{
  return dynamic_cast<PipelineListViewDelegate*>(itemDelegate());
}
