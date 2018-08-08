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

/*
 *
 */
class SVWidgetsLib_EXPORT SVPipelineView : public QListView, public PipelineView
{
  Q_OBJECT

public:
  SIMPL_INSTANCE_PROPERTY(bool, PipelineIsRunning)

  enum class PipelineViewState : int
  {
    Idle = 0,
    Running,
    Cancelling
  };

  using IndexedFilterObject = std::pair<int, PipelineFilterObject*>;

  SIMPL_INSTANCE_PROPERTY(PipelineViewState, PipelineState)

  SVPipelineView(QWidget* parent = nullptr);
  virtual ~SVPipelineView();

  /**
   * @brief filterCount
   * @return
   */
  int filterCount();

  /**
   * @brief openPipeline
   * @param filePath
   * @param insertIndex
   * @return
   */
  int openPipeline(const QString& filePath, int insertIndex = -1) override;

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
   * @brief Adds a filter with the specified filterClassName to the current model
   * @param filterClassName
   */
  void addFilterFromClassName(const QString& filterClassName, int insertIndex = -1) override;

  /**
   * @brief Adds a filter to the current model at insertIndex.  If insertIndex is < 0,
   * the filter gets appended to the end of the model
   * @param filter
   */
  void addFilter(AbstractFilter::Pointer filter, int insertIndex = -1) override;

  /**
   * @brief Adds multiple filters to the current model.  If insertIndex is < 0,
   * the filters get appended to the end of the model
   * @param filters
   */
  void addFilters(std::vector<AbstractFilter::Pointer> filters, int insertIndex = -1) override;

  /**
   * @brief removePipeline
   * @param pipeline
   */
  void removePipeline(FilterPipeline::Pointer pipeline);

  /**
   * @brief Removes filter from the current model
   * @param filter
   */
  void removeFilter(AbstractFilter::Pointer filter) override;

  /**
   * @brief Removes multiple filters from the current model
   * @param filters
   */
  void removeFilters(std::vector<AbstractFilter::Pointer> filters) override;

  /**
   * @brief Cuts filter from the current model
   * @param filter
   */
  void cutFilter(AbstractFilter::Pointer filter) override;

  /**
   * @brief Cuts multiple filters from the current model
   * @param filters
   */
  void cutFilters(std::vector<AbstractFilter::Pointer> filters) override;

  /**
   * @brief Pastes multiple filters from the system clipboard to the current model
   * @param insertIndex
   */
  void pasteFilters(int insertIndex = -1) override;

  /**
   * @brief preflightPipeline
   * @param pipelineRootIndex
   */
  void preflightPipeline() override;

  /**
   * @brief executePipeline
   * @param pipelineRootIndex
   */
  void executePipeline() override;

  /**
   * @brief cancelPipeline
   * @param pipelineIndex
   */
  void cancelPipeline() override;

  /**
   * @brief clearPipeline
   * @param pipelineRootIndex
   */
  void clearPipeline() override;

signals:
  void pipelineFinished();

  void pipelineDataChanged();

  void filePathOpened(const QString& filePath);

  void filterInputWidgetNeedsCleared();

  void deleteKeyPressed();



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
  void dragEnterEvent(QDragEnterEvent* event) override;
  void dragLeaveEvent(QDragLeaveEvent* event) override;
  void dropEvent(QDropEvent* event) override;
  void keyPressEvent(QKeyEvent* event) override;

protected slots:
  /**
   * @brief requestContextMenu
   * @param pos
   */
  void requestContextMenu(const QPoint& pos);

  /**
   * @brief Slot that executes when the delete key gets pressed
   */
  void listenDeleteKeyTriggered();

private:
  QVector<DataContainerArray::Pointer> m_PreflightDataContainerArrays;

  QUndoCommand* m_MoveCommand = nullptr;
  QPoint m_DragStartPosition;
  QModelIndex m_DropIndicatorIndex;

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

  SVPipelineView(const SVPipelineView&) = delete; // Copy Constructor Not Implemented
  void operator=(const SVPipelineView&) = delete; // Move assignment Not Implemented
};
