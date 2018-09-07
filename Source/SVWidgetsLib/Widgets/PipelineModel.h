/* ============================================================================
* Copyright (c) 2017 BlueQuartz Software, LLC
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
* Neither the name of BlueQuartz Software nor the names of its
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
*
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#pragma once

#include <QtCore/QAbstractItemModel>
#include <QtCore/QModelIndex>
#include <QtCore/QVariant>

#include <QtWidgets/QAction>
#include <QtWidgets/QTextEdit>

#include "SIMPLib/Common/SIMPLibSetGetMacros.h"
#include "SIMPLib/Filtering/AbstractFilter.h"
#include "SIMPLib/Filtering/FilterPipeline.h"

#include "SVWidgetsLib/Widgets/PipelineItem.h"

#include "SVWidgetsLib/SVWidgetsLib.h"

class QtSSettings;

class SVWidgetsLib_EXPORT PipelineModel : public QAbstractItemModel
{
    Q_OBJECT

  public:
    SIMPL_TYPE_MACRO(PipelineModel)

    PipelineModel(QObject* parent = nullptr);
    PipelineModel(size_t maxPipelineCount, QObject* parent = nullptr);

    ~PipelineModel() override;

    enum Roles
    {
      WidgetStateRole = Qt::UserRole + 1,
      ErrorStateRole,
      PipelineStateRole,
      ItemTypeRole,
      PipelinePathRole,
      PipelineModifiedRole
    };

    SIMPL_SET_PROPERTY(bool, UseModelDisplayText)

    SIMPL_GET_PROPERTY(size_t, MaxPipelineCount)

    /**
     * @brief savePipeline
     * @param pipelineRootIndex
     */
    bool savePipeline(const QModelIndex &pipelineRootIndex, const QString &pipelineName);

    /**
     * @brief isPipelineRootItem
     * @param index
     * @return
     */
    bool isPipelineRootItem(const QModelIndex &index) const;

    /**
     * @brief isFilterItem
     * @param index
     * @return
     */
    bool isFilterItem(const QModelIndex &index) const;

    QVariant data(const QModelIndex& index, int role) const override;
//    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const Q_DECL_OVERRIDE;

    FilterPipeline::Pointer tempPipeline(const QModelIndex &index) const;
    FilterPipeline::Pointer savedPipeline(const QModelIndex &index) const;

    bool addPipeline(FilterPipeline::Pointer pipeline);
    bool setPipeline(const QModelIndex &index, FilterPipeline::Pointer pipeline);

    QString pipelineFilePath(const QModelIndex &index);
    void setPipelineFilePath(const QModelIndex& index, const QString &filePath);

    AbstractFilter::Pointer filter(const QModelIndex &index) const;

    QModelIndex indexOfFilter(AbstractFilter *filter, const QModelIndex &parent = QModelIndex());

    FilterInputWidget* filterInputWidget(const QModelIndex &index);
    void setFilterInputWidget(const QModelIndex &index, FilterInputWidget* fiw);

    PipelineOutputTextEdit* pipelineOutputTextEdit(const QModelIndex &pipelineRootIndex);

    PipelineMessageObserver* pipelineMessageObserver(const QModelIndex &pipelineRootIndex);
    QVector<PipelineMessage> pipelineMessages(const QModelIndex &pipelineRootIndex);
    void clearPipelineMessages(const QModelIndex &pipelineRootIndex);

    bool isEmpty();

    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex& index) const override;

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;

    bool insertRows(int position, int rows, const QModelIndex& parent = QModelIndex()) override;
    bool removeRows(int position, int rows, const QModelIndex& parent = QModelIndex()) override;

    bool moveRows(const QModelIndex & sourceParent, int sourceRow, int count, const QModelIndex & destinationParent, int destinationChild) override;

    Qt::ItemFlags flags(const QModelIndex& index) const override;

    Qt::DropActions supportedDropActions() const override;

    QStringList mimeTypes() const override;
    
    bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const override;

    bool setData(const QModelIndex& index, const QVariant& value, int role) override;

    PipelineItem* getRootItem();

    int getMaxFilterCount() const;

    QList<QObject*> getPipelineMessageObservers();

    QModelIndex getPipelineRootIndexFromPipeline(FilterPipeline::Pointer pipeline);

  protected:
    /**
     * @brief setPipelineOutputTextEdit
     * @param pipelineRootIndex
     * @param pipelineOutputTE
     */
    void setPipelineOutputTextEdit(const QModelIndex &pipelineRootIndex, PipelineOutputTextEdit* pipelineOutputTE);

    /**
     * @brief setPipelineMessageObserver
     * @param pipelineRootIndex
     * @param messageObserver
     */
    void setPipelineMessageObserver(const QModelIndex &pipelineRootIndex, PipelineMessageObserver* messageObserver);

  signals:
    void pipelineAdded(FilterPipeline::Pointer pipeline, const QModelIndex &pipelineRootIndex);
    void pipelineRemoved(FilterPipeline::Pointer pipeline);
    void pipelineModified(FilterPipeline::Pointer pipeline, const QModelIndex &pipelineRootIndex, bool modified);
    void pipelineSaved(FilterPipeline::Pointer pipeline, const QModelIndex &pipelineRootIndex);

    void filtersAdded(std::vector<AbstractFilter::Pointer> filters, std::vector<size_t> indices, const QModelIndex &pipelineRootIndex);
    void filtersRemoved(std::vector<size_t> indices, const QModelIndex &pipelineRootIndex);

    void statusMessage(const QString& message);
    void stdOutMessage(const QString& message);

    void clearIssuesTriggered();

    void preflightTriggered(const QModelIndex &pipelineRootIndex);

    void filterParametersChanged(AbstractFilter::Pointer filter);

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
    PipelineItem*                       m_RootItem = nullptr;
    bool                                m_UseModelDisplayText = true;
    size_t                              m_MaxPipelineCount = std::numeric_limits<size_t>::max();

    PipelineItem* getItem(const QModelIndex& index) const;

    QColor getForegroundColor(const QModelIndex &index) const;

    void insertFilter(AbstractFilter::Pointer filter, int index, const QModelIndex &pipelineRootIndex);

    void addFilterData(AbstractFilter::Pointer filter, const QModelIndex &filterIndex);

    PipelineModel(const PipelineModel&);    // Copy Constructor Not Implemented
    void operator=(const PipelineModel&);  // Operator '=' Not Implemented
};

