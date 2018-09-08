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

#include <stack>
#include <vector>

#include <QtCore/QSharedPointer>

#include <QtGui/QPainter>

#include <QtWidgets/QLabel>
#include <QtWidgets/QListView>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QUndoCommand>
#include <QtWidgets/QVBoxLayout>

#include "SIMPLib/Common/PipelineMessage.h"
#include "SIMPLib/CoreFilters/DataContainerReader.h"
#include "SIMPLib/FilterParameters/H5FilterParametersReader.h"
#include "SIMPLib/FilterParameters/H5FilterParametersWriter.h"

#include "SVWidgetsLib/QtSupport/QtSFileDragMessageBox.h"
#include "SVWidgetsLib/SVWidgetsLib.h"
#include "SVWidgetsLib/Widgets/PipelineView.h"

class QScrollArea;
class QContextMenuEvent;
class QLabel;
class QEvent;
class QMenu;
class QAction;
class PipelineFilterObject;
class DataStructureWidget;
class PipelineModel;
class QSignalMapper;
class PipelineViewController;
class PipelineListViewDelegate;

/*
 *
 */
class SVWidgetsLib_EXPORT PipelineListView : public QListView, public PipelineView
{
  Q_OBJECT

public:
  SIMPL_INSTANCE_PROPERTY(bool, PipelineIsRunning)

  SIMPL_GET_PROPERTY(QModelIndex, PipelineRootIndex)

  using IndexedFilterObject = std::pair<int, PipelineFilterObject*>;

  PipelineListView(QWidget* parent = nullptr);
  ~PipelineListView() override;

  /**
   * @brief openPipeline
   * @param filePath
   * @param insertIndex
   * @return
   */
  int openPipeline(const QString& filePath, QModelIndex pipelineRootIndex, int insertIndex = -1) override;

  /**
   * @brief setModel
   * @param model
   */
  void setModel(QAbstractItemModel* model) override;

  /**
   * @brief getPipelineModel
   * @return
   */
  PipelineModel* getPipelineModel();

  /**
   * @brief isPipelineCurrentlyRunning
   * @return
   */
  bool isPipelineCurrentlyRunning();

  /**
   * @brief setPipelineViewState
   * @param state
   */
  void setPipelineViewState(PipelineViewState state);

  /**
   * @brief Returns the regular disable button pixmap in the current highlighted text color
   * @return
   */
  QPixmap getDisableBtnPixmap(bool highlighted = false);

  /**
   * @brief Returns the high-dpi disable button pixmap in the current highlighted text color
   * @return
   */
  QPixmap getHighDPIDisableBtnPixmap(bool highlighted = false);

  /**
   * @brief Returns the regular activated disable button pixmap in the current highlighted text color
   * @return
   */
  QPixmap getDisableBtnActivatedPixmap(bool highlighted = false);

  /**
   * @brief Returns the high-dpi activated disable button pixmap in the current highlighted text color
   * @return
   */
  QPixmap getHighDPIDisableBtnActivatedPixmap(bool highlighted = false);

  /**
   * @brief Returns the regular hovered disable button pixmap in the current highlighted text color
   * @return
   */
  QPixmap getDisableBtnHoveredPixmap(bool highlighted = false);

  /**
   * @brief Returns the high-dpi hovered disable button pixmap in the current highlighted text color
   * @return
   */
  QPixmap getHighDPIDisableBtnHoveredPixmap(bool highlighted = false);

  /**
   * @brief Returns the regular delete button pixmap in the current highlighted text color
   * @return
   */
  QPixmap getDeleteBtnPixmap(bool highlighted = false);

  /**
   * @brief Returns the high-dpi delete button pixmap in the current highlighted text color
   * @return
   */
  QPixmap getHighDPIDeleteBtnPixmap(bool highlighted = false);

  /**
   * @brief Returns the regular hovered delete button pixmap in the current highlighted text color
   * @return
   */
  QPixmap getDeleteBtnHoveredPixmap(bool highlighted = false);

  /**
   * @brief Returns the high-dpi hovered delete button pixmap in the current highlighted text color
   * @return
   */
  QPixmap getHighDPIDeleteBtnHoveredPixmap(bool highlighted = false);

public slots:
  /**
   * @brief addPipeline
   * @param pipeline
   * @param insertIndex
   */
  void addPipeline(FilterPipeline::Pointer pipeline, int insertIndex = -1);

  /**
   * @brief removePipeline
   * @param pipeline
   */
  void removePipeline(FilterPipeline::Pointer pipeline);

signals:
  void pipelineFinished();

  void pipelineDataChanged();

  void filePathOpened(const QString& filePath);

  void filterInputWidgetNeedsCleared();

  void displayIssuesTriggered();
  void clearIssuesTriggered();

  void writeSIMPLViewSettingsTriggered();

  void addPlaceHolderFilter(QPoint p);
  void removePlaceHolderFilter();

  void filterParametersChanged(AbstractFilter::Pointer filter);

  void pipelineStarted();

  void pipelineHasMessage(const PipelineMessage& msg);
  void pipelineFilePathUpdated(const QString& name);
  void pipelineChanged();

  void statusMessage(const QString& message);
  void stdOutMessage(const QString& message);

protected:
  void setupGui();

  void connectSignalsSlots();

  /**
   * @brief beginDrag
   * @param event
   */
  void beginDrag(QMouseEvent* event);

  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void dragMoveEvent(QDragMoveEvent* event) override;
  void dragLeaveEvent(QDragLeaveEvent* event) override;
  void dropEvent(QDropEvent* event) override;
  void keyPressEvent(QKeyEvent* event) override;
  void paintEvent(QPaintEvent* event) override;

protected slots:
  /**
   * @brief requestContextMenu
   * @param pos
   */
  void requestContextMenu(const QPoint& pos);

private:
  QVector<DataContainerArray::Pointer> m_PreflightDataContainerArrays;

  QUndoCommand* m_MoveCommand = nullptr;
  QPoint m_DragStartPosition;
  int m_DropIndicatorRow;

  QString m_CurrentPipelineFilePath;

  QModelIndex m_PipelineRootIndex;

  QPixmap m_DisableBtnPixmap;
  QPixmap m_DisableHighlightedPixmap;
  QColor m_DisableBtnColor = QColor(Qt::black);

  QPixmap m_DisableBtnPixmap2x;
  QPixmap m_DisableBtnHighlightedPixmap2x;
  QColor m_DisableBtn2xColor = QColor(Qt::black);

  QPixmap m_DisableBtnActivatedPixmap;
  QPixmap m_DisableBtnActivatedPixmap2x;

  QPixmap m_DisableBtnHoveredPixmap;
  QPixmap m_DisableBtnHoveredHighlightedPixmap;
  QColor m_DisableBtnHoveredColor = QColor(Qt::black);

  QPixmap m_DisableBtnHoveredPixmap2x;
  QPixmap m_DisableBtnHoveredHighlightedPixmap2x;
  QColor m_DisableBtnHovered2xColor = QColor(Qt::black);

  QPixmap m_DeleteBtnPixmap;
  QPixmap m_DeleteBtnHighlightedPixmap;
  QColor m_DeleteBtnColor = QColor(Qt::black);

  QPixmap m_DeleteBtnPixmap2x;
  QPixmap m_DeleteBtnHighlightedPixmap2x;
  QColor m_DeleteBtn2xColor = QColor(Qt::black);

  QPixmap m_DeleteBtnHoveredPixmap;
  QPixmap m_DeleteBtnHoveredHighlightedPixmap;
  QColor m_DeleteBtnHoveredColor = QColor(Qt::black);

  QPixmap m_DeleteBtnHoveredPixmap2x;
  QPixmap m_DeleteBtnHoveredHighlightedPixmap2x;
  QColor m_DeleteBtnHovered2xColor = QColor(Qt::black);

  /**
   * @brief getSelectedRows
   * @return
   */
  QModelIndexList getSelectedRows() override;

  /**
   * @brief addDropIndicator
   * @param text
   * @param insertIndex
   */
  void addDropIndicator(const QString& text, int insertIndex);

  /**
   * @brief removeDropIndicator
   */
  void removeDropIndicator();

  /**
   * @brief findNextRow
   * @param pos
   * @return
   */
  int findNextRow(const QPoint& pos);

  /**
   * @brief findPreviousRow
   * @param pos
   * @return
   */
  int findPreviousRow(const QPoint& pos);

  /**
   * @brief getDraggingPixmap
   * @param indexes
   * @return
   */
  QPixmap getDraggingPixmap(QModelIndexList indexes);

  /**
   * @brief setPixmapColor
   * @param pixmap
   * @param pixmapColor
   */
  QPixmap setPixmapColor(QPixmap pixmap, QColor pixmapColor);

  /**
   * @brief getViewDelegate
   * @return
   */
  PipelineListViewDelegate* getViewDelegate();

  PipelineListView(const PipelineListView&) = delete; // Copy Constructor Not Implemented
  void operator=(const PipelineListView&) = delete; // Move assignment Not Implemented
};
