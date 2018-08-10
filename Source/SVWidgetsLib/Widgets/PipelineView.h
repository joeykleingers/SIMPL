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

#include <QtCore/QModelIndex>

#include "SIMPLib/Common/SIMPLibSetGetMacros.h"

#include "SIMPLib/Filtering/AbstractFilter.h"

#include "SVWidgetsLib/SVWidgetsLib.h"

class PipelineViewController;
class QAction;
class QMenu;
class PipelineModel;

/**
 * @brief The PipelineView class
 */
class SVWidgetsLib_EXPORT PipelineView
{
  public:
    virtual ~PipelineView();

    SIMPL_POINTER_PROPERTY(PipelineViewController, PipelineViewController)

    /**
     * @brief addPipelineMessageObserver
     * @param pipelineMessageObserver
     */
    void addPipelineMessageObserver(QObject* pipelineMessageObserver);

    /**
     * @brief openPipeline
     * @param filePath
     * @param insertIndex
     * @return
     */
    virtual int openPipeline(const QString& filePath, int insertIndex = -1) = 0;

    /**
     * @brief writePipeline
     * @param outputPath
     * @return
     */
    int writePipeline(const QModelIndex &pipelineRootIndex, const QString &outputPath);

    /**
     * @brief Copies the currently selected filters from the current model into the system clipboard
     */
    void copySelectedFilters();

    /**
     * @brief getActionUndo
     * @return
     */
    QAction* getActionUndo();

    /**
     * @brief getActionRedo
     * @return
     */
    QAction* getActionRedo();

    /**
     * @brief getPipelineModel
     * @return
     */
    PipelineModel* getPipelineModel();

    /**
     * @brief getSelectedFilters
     * @return
     */
    std::vector<AbstractFilter::Pointer> getSelectedFilters();

    /**
     * @brief getSelectedRows
     * @return
     */
    virtual QModelIndexList getSelectedRows() = 0;

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
     * @brief Pastes multiple filters from the system clipboard to the current model
     * @param insertIndex
     */
    void pasteFilters(int insertIndex = -1);

    /**
     * @brief preflightPipeline
     * @param pipelineRootIndex
     */
    void preflightPipeline();

    /**
     * @brief executePipeline
     * @param pipelineRootIndex
     */
    void executePipeline();

    /**
     * @brief cancelPipeline
     * @param pipelineIndex
     */
    void cancelPipeline();

    /**
     * @brief clearPipeline
     * @param pipelineRootIndex
     */
    void clearPipeline();

  protected:
    PipelineView();

    /**
     * @brief setupGui
     */
    void setupGui();

    /**
     * @brief setSelectedFiltersEnabled
     * @param enabled
     */
    void setSelectedFiltersEnabled();

  private:
    QModelIndex m_ActivePipelineIndex;

    PipelineView(const PipelineView&) = delete;   // Copy Constructor Not Implemented
    void operator=(const PipelineView&) = delete; // Move assignment Not Implemented
};

