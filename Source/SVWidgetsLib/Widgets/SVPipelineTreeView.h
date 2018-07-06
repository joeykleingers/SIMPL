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
#include <QtWidgets/QTreeView>
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
class SVWidgetsLib_EXPORT SVPipelineTreeView : public QTreeView, public PipelineView
{
  Q_OBJECT

public:
  SIMPL_INSTANCE_PROPERTY(bool, PipelineIsRunning)

  SIMPL_GET_PROPERTY(QAction*, ActionEnableFilter)
  SIMPL_GET_PROPERTY(QAction*, ActionCut)
  SIMPL_GET_PROPERTY(QAction*, ActionCopy)
  SIMPL_GET_PROPERTY(QAction*, ActionPaste)
  SIMPL_GET_PROPERTY(QAction*, ActionClearPipeline)

  SVPipelineTreeView(QWidget* parent = nullptr);
  virtual ~SVPipelineTreeView();

  /**
   * @brief addPipelineMessageObserver
   * @param pipelineMessageObserver
   */
  void addPipelineMessageObserver(QObject* pipelineMessageObserver);

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
  int openPipeline(const QString& filePath, int insertIndex = -1);

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

  void filterEnabledStateChanged();

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

  void setFiltersEnabled(QModelIndexList indexes, bool enabled);
  void setSelectedFiltersEnabled(bool enabled);

protected slots:  
  /**
   * @brief updatePasteAvailability
   */
  void updatePasteAvailability();

  /**
   * @brief requestContextMenu
   * @param pos
   */
  void requestContextMenu(const QPoint& pos);

  /**
   * @brief Slot that executes when the delete key gets pressed
   */
  void listenDeleteKeyTriggered();

  void listenCutTriggered();
  void listenCopyTriggered();
  void listenPasteTriggered();
  void listenClearPipelineTriggered();

private:
  QVector<DataContainerArray::Pointer> m_PreflightDataContainerArrays;

  bool m_PipelineRunning = false;

  QUndoCommand* m_MoveCommand = nullptr;
  QPoint m_DragStartPosition;
  QModelIndex m_DropIndicatorIndex;

  QAction* m_ActionEnableFilter = nullptr;
  QAction* m_ActionCut = nullptr;
  QAction* m_ActionCopy = nullptr;
  QAction* m_ActionPaste = nullptr;
  QAction* m_ActionClearPipeline = nullptr;

  QString m_CurrentPipelineFilePath;

  QModelIndex m_PipelineRootIndex;

  /**
   * @brief getSelectedRows
   * @return
   */
  QModelIndexList getSelectedRows() override;

  /**
   * @brief requestFilterContextMenu
   * @param pos
   * @param index
   */
  void requestFilterItemContextMenu(const QPoint& pos, const QModelIndex& index);

  /**
   * @brief requestPipelineContextMenu
   * @param pos
   */
  void requestPipelineItemContextMenu(const QPoint& pos);

  /**
   * @brief requestSinglePipelineContextMenu
   * @param menu
   */
  void requestSinglePipelineContextMenu(QMenu& menu);

  /**
   * @brief requestErrorHandlingContextMenu
   * @param menu
   */
  void requestErrorHandlingContextMenu(QMenu& menu);

  /**
   * @brief requestDefaultContextMenu
   * @param pos
   */
  void requestDefaultContextMenu(const QPoint& pos);

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
   * @brief getDraggingPixmap
   * @param indexes
   * @return
   */
  QPixmap getDraggingPixmap(QModelIndexList indexes);

  SVPipelineTreeView(const SVPipelineTreeView&) = delete; // Copy Constructor Not Implemented
  void operator=(const SVPipelineTreeView&) = delete; // Move assignment Not Implemented
};
