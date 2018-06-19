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

#include <vector>
#include <stack>

#include <QtCore/QSharedPointer>

#include <QtGui/QPainter>

#include <QtWidgets/QLabel>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QTableWidget>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QListView>
#include <QtWidgets/QUndoCommand>

#include "SIMPLib/Common/PipelineMessage.h"
#include "SIMPLib/CoreFilters/DataContainerReader.h"
#include "SIMPLib/FilterParameters/H5FilterParametersReader.h"
#include "SIMPLib/FilterParameters/H5FilterParametersWriter.h"

#include "SVWidgetsLib/SVWidgetsLib.h"
#include "SVWidgetsLib/Widgets/PipelineView.h"
#include "SVWidgetsLib/QtSupport/QtSFileDragMessageBox.h"

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

    SIMPL_GET_PROPERTY(QAction*, ActionEnableFilter)
    SIMPL_GET_PROPERTY(QAction*, ActionCut)
    SIMPL_GET_PROPERTY(QAction*, ActionCopy)
    SIMPL_GET_PROPERTY(QAction*, ActionPaste)

    SVPipelineView(QWidget* parent = 0);
    virtual ~SVPipelineView();

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
     * @brief Adds a filter with the specified filterClassName to the current model
     * @param filterClassName
     */
    void addFilterFromClassName(const QString &filterClassName, int insertIndex = -1);

    /**
     * @brief Adds a filter to the current model at insertIndex.  If insertIndex is < 0,
     * the filter gets appended to the end of the model
     * @param filter
     */
    void addFilter(AbstractFilter::Pointer filter, int insertIndex = -1);

    /**
     * @brief Adds multiple filters to the current model.  If insertIndex is < 0,
     * the filters get appended to the end of the model
     * @param filters
     */
    void addFilters(std::vector<AbstractFilter::Pointer> filters, int insertIndex = -1);

    /**
     * @brief Removes filter from the current model
     * @param filter
     */
    void removeFilter(AbstractFilter::Pointer filter);

    /**
     * @brief Removes multiple filters from the current model
     * @param filters
     */
    void removeFilters(std::vector<AbstractFilter::Pointer> filters);

    /**
     * @brief Cuts filter from the current model
     * @param filter
     */
    void cutFilter(AbstractFilter::Pointer filter);

    /**
     * @brief Cuts multiple filters from the current model
     * @param filters
     */
    void cutFilters(std::vector<AbstractFilter::Pointer> filters);

    /**
     * @brief Copies the currently selected filters from the current model into the system clipboard
     */
    void copySelectedFilters();

    /**
     * @brief Pastes multiple filters from the system clipboard to the current model
     * @param insertIndex
     */
    void pasteFilters(int insertIndex = -1);

    /**
     * @brief preflightPipeline
     * @param pipelineRootIndex
     */
    void preflightPipeline(const QModelIndex &pipelineRootIndex);

    /**
     * @brief executePipeline
     * @param pipelineRootIndex
     */
    void executePipeline(const QModelIndex &pipelineRootIndex);

    /**
     * @brief cancelPipeline
     * @param pipelineIndex
     */
    void cancelPipeline();

    /**
     * @brief clearPipeline
     * @param pipelineRootIndex
     */
    void clearPipeline(const QModelIndex &pipelineRootIndex);

  signals:
    void clearDataStructureWidgetTriggered();

    void addPlaceHolderFilter(QPoint p);
    void removePlaceHolderFilter();

    void preflightFinished(FilterPipeline::Pointer pipeline, int err);
    void pipelineFinished();

    void filePathOpened(const QString &filePath);

    void filterInputWidgetNeedsCleared();

    void filterInputWidgetEdited();

    void filterEnabledStateChanged();

    void deleteKeyPressed();

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
    void keyPressEvent(QKeyEvent *event) override;

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

  private slots:
    /**
     * @brief listenFilterInProgress
     * @param filter
     */
    void listenFilterInProgress(AbstractFilter *filter);

    /**
     * @brief listenFilterCompleted
     */
    void listenFilterCompleted(AbstractFilter *filter);

  private:
    PipelineViewController* m_PipelineViewController = nullptr;

    QUndoCommand* m_MoveCommand = nullptr;

    QAction* m_ActionEnableFilter = nullptr;
    QAction* m_ActionCut = nullptr;
    QAction* m_ActionCopy = nullptr;
    QAction* m_ActionPaste = nullptr;
    QAction* m_ActionClearPipeline = nullptr;

    bool m_PipelineRunning = false;

    QString m_CurrentPipelineFilePath;

    QPoint m_DragStartPosition;
    QModelIndex m_DropIndicatorIndex;

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
     * @brief Gets the currently selected filters
     * @return
     */
    std::vector<AbstractFilter::Pointer> getSelectedFilters();

    /**
     * @brief requestFilterContextMenu
     * @param pos
     * @param index
     */
    void requestFilterItemContextMenu(const QPoint &pos, const QModelIndex &index);

    /**
     * @brief requestPipelineContextMenu
     * @param pos
     */
    void requestPipelineItemContextMenu(const QPoint &pos);

    /**
     * @brief requestSinglePipelineContextMenu
     * @param menu
     */
    void requestSinglePipelineContextMenu(QMenu &menu);

    /**
     * @brief requestDefaultContextMenu
     * @param pos
     */
    void requestDefaultContextMenu(const QPoint &pos);

    /**
     * @brief addDropIndicator
     * @param text
     * @param insertIndex
     */
    void addDropIndicator(const QString &text, int insertIndex);

    /**
     * @brief removeDropIndicator
     */
    void removeDropIndicator();

    /**
     * @brief findNextRow
     * @param pos
     * @return
     */
    int findNextRow(const QPoint &pos);

    /**
     * @brief findPreviousRow
     * @param pos
     * @return
     */
    int findPreviousRow(const QPoint &pos);

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
    void operator=(const SVPipelineView&) = delete;       // Move assignment Not Implemented
};

