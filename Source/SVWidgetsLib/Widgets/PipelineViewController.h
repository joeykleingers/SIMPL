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

#include <QtCore/QSharedPointer>
#include <QtCore/QModelIndex>

#include <QtWidgets/QAction>

#include "SIMPLib/Filtering/FilterPipeline.h"

#include "SVWidgetsLib/SVWidgetsLib.h"

class QUndoCommand;
class QUndoStack;
class PipelineModel;

/*
 *
 */
class SVWidgetsLib_EXPORT PipelineViewController : public QObject
{
    Q_OBJECT

  public:
    PipelineViewController(PipelineModel* model, QObject* parent = nullptr);
    virtual ~PipelineViewController();

    SIMPL_GET_PROPERTY(QAction*, ActionUndo)
    SIMPL_GET_PROPERTY(QAction*, ActionRedo)

    /**
     * @brief addPipelineMessageObserver
     * @param pipelineMessageObserver
     */
    void addPipelineMessageObserver(QObject* pipelineMessageObserver);

    /**
     * @brief addUndoCommand
     * @param cmd
     */
    void addUndoCommand(QUndoCommand* cmd);

    /**
     * @brief undo
     */
    void undo();

    /**
     * @brief redo
     */
    void redo();

    /**
     * @brief openPipeline
     * @param filePath
     * @return
     */
    int openPipeline(const QString& filePath, int insertIndex = -1, const QModelIndex &pipelineRootIndex = QModelIndex());

    /**
     * @brief readPipelineFromFile
     * @param filePath
     * @return FilterPipeline::Pointer
     */
    FilterPipeline::Pointer readPipelineFromFile(const QString& filePath);

    /**
     * @brief getFilterPipeline
     * @param pipelineRootIndex
     * @return
     */
    FilterPipeline::Pointer getFilterPipeline(const QModelIndex &pipelineRootIndex);

    /**
     * @brief writePipeline
     * @param outputPath
     * @return
     */
    int writePipeline(const QModelIndex &pipelineRootIndex, const QString &outputPath);

  public slots:
    /**
     * @brief Adds a filter with the specified filterClassName to the current model
     * @param filterClassName
     */
    void addFilterFromClassName(const QString &filterClassName, int insertIndex = -1, const QModelIndex &pipelineRootIndex = QModelIndex());

    /**
     * @brief Adds a filter to the current model at insertIndex.  If insertIndex is < 0,
     * the filter gets appended to the end of the model
     * @param filter
     */
    void addFilter(AbstractFilter::Pointer filter, int insertIndex = -1, const QModelIndex &pipelineRootIndex = QModelIndex());

    /**
     * @brief Adds multiple filters to the current model.  If insertIndex is < 0,
     * the filters get appended to the end of the model
     * @param filters
     */
    void addFilters(std::vector<AbstractFilter::Pointer> filters, int insertIndex = -1, const QModelIndex &pipelineRootIndex = QModelIndex());

    /**
     * @brief Removes filter from the current model
     * @param filter
     */
    void removeFilter(AbstractFilter::Pointer filter, const QModelIndex &pipelineRootIndex = QModelIndex());

    /**
     * @brief Removes multiple filters from the current model
     * @param filters
     */
    void removeFilters(std::vector<AbstractFilter::Pointer> filters, const QModelIndex &pipelineRootIndex = QModelIndex());

    /**
     * @brief Cuts filter from the current model
     * @param filter
     */
    void cutFilter(AbstractFilter::Pointer filter, const QModelIndex &pipelineRootIndex = QModelIndex());

    /**
     * @brief Cuts multiple filters from the current model
     * @param filters
     */
    void cutFilters(std::vector<AbstractFilter::Pointer> filters, const QModelIndex &pipelineRootIndex = QModelIndex());

    /**
     * @brief Pastes multiple filters from the system clipboard to the current model
     * @param insertIndex
     */
    void pasteFilters(int insertIndex = -1, const QModelIndex &pipelineRootIndex = QModelIndex());

    /**
     * @brief Should be block this class from either emitting a preflight signal or otherwise running a preflight.
     * @param b
     */
    void blockPreflightSignals(bool b);

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
     * @brief clearPipeline
     * @param pipelineRootIndex
     */
    void clearPipeline(const QModelIndex &pipelineRootIndex);

    /**
     * @brief Update the indices of all FilterInputWidgets
     * @param pipelineRootIndex
     */
    void updateFilterInputWidgetIndices(const QModelIndex &pipelineRootIndex);

    /**
     * @brief cancelPipeline
     * @param pipelineIndex
     */
    void cancelPipeline(const QModelIndex &pipelineRootIndex);

    /**
     * @brief Copies the currently selected filters from the current model into the system clipboard
     */
    void copySelectedFilters();

  protected:
    PipelineViewController();

    /**
     * @brief initialize
     */
    void initialize();

  protected slots:
    void listenCutTriggered();
    void listenCopyTriggered();

  signals:
    void clearIssuesTriggered();
    void displayIssuesTriggered();

    void writeSIMPLViewSettingsTriggered();

    void preflightFinished(FilterPipeline::Pointer pipeline, int err);

    void pipelineStarted(const QModelIndex &pipelineRootIndex);
    void pipelineFilePathUpdated(const QString &name);
    void pipelineDataChanged(const QModelIndex &pipelineRootIndex);
    void pipelineFinished(const QModelIndex &pipelineRootIndex);

    void statusMessage(const QString& message);
    void stdOutMessage(const QString& message);

  private slots:
    /**
     * @brief processPipelineMessage
     * @param msg
     */
    void processPipelineMessage(const PipelineMessage& msg);

    /**
     * @brief finishPipeline
     * @param pipeline
     * @param pipelineRootIndex
     */
    void pipelineExecutionFinished(FilterPipeline::Pointer pipeline, const QModelIndex &pipelineRootIndex);

  private:
    PipelineModel* m_PipelineModel = nullptr;
    QList<QObject*> m_PipelineMessageObservers;

    std::vector<AbstractFilter::Pointer> m_SelectedFilters;

    QSharedPointer<QUndoStack> m_UndoStack;
    QAction* m_ActionUndo = nullptr;
    QAction* m_ActionRedo = nullptr;

    bool m_BlockPreflight = false;
    std::stack<bool> m_BlockPreflightStack;

    QVector<DataContainerArray::Pointer> m_PreflightDataContainerArrays;

    /**
     * @brief setupUndoStack
     */
    void setupUndoStack();

    /**
     * @brief setupControllerActions
     */
    void setupControllerActions();

    /**
     * @brief setPipelineToReadyState
     */
    void setPipelineToReadyState(const QModelIndex &pipelineRootIndex);

    /**
     * @brief setPipelineToRunningState
     */
    void setPipelineToRunningState(const QModelIndex &pipelineRootIndex);

    /**
     * @brief setPipelineToStoppedState
     */
    void setPipelineToStoppedState(const QModelIndex &pipelineRootIndex);

    PipelineViewController(const PipelineViewController&) = delete;   // Copy Constructor Not Implemented
    void operator=(const PipelineViewController&) = delete; // Move assignment Not Implemented
};

