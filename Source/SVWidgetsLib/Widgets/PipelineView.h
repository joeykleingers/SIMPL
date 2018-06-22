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

/**
 * @brief The PipelineView class
 */
class SVWidgetsLib_EXPORT PipelineView
{
  public:
    virtual ~PipelineView();

    SIMPL_GET_PROPERTY(PipelineViewController*, PipelineViewController)

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

  public slots:
    /**
     * @brief Adds a filter with the specified filterClassName to the current model
     * @param filterClassName
     */
    virtual void addFilterFromClassName(const QString &filterClassName, int insertIndex = -1) = 0;

    /**
     * @brief Adds a filter to the current model at insertIndex.  If insertIndex is < 0,
     * the filter gets appended to the end of the model
     * @param filter
     */
    virtual void addFilter(AbstractFilter::Pointer filter, int insertIndex = -1) = 0;

    /**
     * @brief Adds multiple filters to the current model.  If insertIndex is < 0,
     * the filters get appended to the end of the model
     * @param filters
     */
    virtual void addFilters(std::vector<AbstractFilter::Pointer> filters, int insertIndex = -1) = 0;

    /**
     * @brief Removes filter from the current model
     * @param filter
     */
    virtual void removeFilter(AbstractFilter::Pointer filter) = 0;

    /**
     * @brief Removes multiple filters from the current model
     * @param filters
     */
    virtual void removeFilters(std::vector<AbstractFilter::Pointer> filters) = 0;

    /**
     * @brief Cuts filter from the current model
     * @param filter
     */
    virtual void cutFilter(AbstractFilter::Pointer filter) = 0;

    /**
     * @brief Cuts multiple filters from the current model
     * @param filters
     */
    virtual void cutFilters(std::vector<AbstractFilter::Pointer> filters) = 0;

    /**
     * @brief Pastes multiple filters from the system clipboard to the current model
     * @param insertIndex
     */
    virtual void pasteFilters(int insertIndex = -1) = 0;

    /**
     * @brief preflightPipeline
     * @param pipelineRootIndex
     */
    virtual void preflightPipeline() = 0;

    /**
     * @brief executePipeline
     * @param pipelineRootIndex
     */
    virtual void executePipeline() = 0;

    /**
     * @brief cancelPipeline
     * @param pipelineIndex
     */
    virtual void cancelPipeline() = 0;

    /**
     * @brief clearPipeline
     * @param pipelineRootIndex
     */
    virtual void clearPipeline() = 0;

    /**
     * @brief Copies the currently selected filters from the current model into the system clipboard
     */
    void copySelectedFilters();

  protected:
    PipelineView();

    virtual std::vector<AbstractFilter::Pointer> getSelectedFilters() = 0;

    SIMPL_SET_PROPERTY(PipelineViewController*, PipelineViewController)

  private:
    PipelineViewController* m_PipelineViewController = nullptr;

    PipelineView(const PipelineView&) = delete;   // Copy Constructor Not Implemented
    void operator=(const PipelineView&) = delete; // Move assignment Not Implemented
};

