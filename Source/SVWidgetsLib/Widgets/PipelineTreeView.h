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


#ifndef _PipelineTreeView_h_
#define _PipelineTreeView_h_

#include <QApplication>

#include <QtCore/QJsonObject>

#include <QtWidgets/QMenu>
#include <QtGui/QMouseEvent>
#include <QtWidgets/QTreeWidget>
#include <QtWidgets/QTreeWidgetItem>

#include "SIMPLib/Filtering/FilterPipeline.h"

#include "SVWidgetsLib/SVWidgetsLib.h"

#include "SVWidgetsLib/Widgets/PipelineTreeModel.h"

class PipelineTreeController;
class PipelineBuilderWidget;
class QAction;
class QTreeWidgetItem;

class SVWidgetsLib_EXPORT PipelineTreeView : public QTreeView
{
    Q_OBJECT

  public:
    enum ItemType
    {
      Pipeline_Item_Type = 1,
      Filter_Item_Type = 2,
      Unknown_Item_Type = 3
    };

    SIMPL_INSTANCE_PROPERTY(bool, PipelineIsRunning)

    /**
    * @brief PipelineTreeView
    * @param parent
    */
    PipelineTreeView(QWidget* parent = 0);

    /**
    * @brief ~PipelineTreeView()
    */
    ~PipelineTreeView();

    /**
    * @brief fromJsonObject
    * @param modelObject
    */
    static PipelineTreeModel* FromJsonObject(QJsonObject modelObject);

    /**
    * @brief toJsonObject
    */
    QJsonObject toJsonObject();

    /**
    * @brief setModel
    * @param model
    */
    void setModel(QAbstractItemModel* model) Q_DECL_OVERRIDE;

    /**
    * @brief filterOutDescendants
    * @param indexList
    */
    QModelIndexList filterOutDescendants(QModelIndexList indexList);

  public slots:
    void collapseIndex(const QModelIndex& index);
    void expandIndex(const QModelIndex& index);

  protected:
    void currentChanged(const QModelIndex& current, const QModelIndex& previous) Q_DECL_OVERRIDE;

    void setFiltersEnabled(QModelIndexList indices, bool enabled);
    void setSelectedFiltersEnabled(bool enabled);

    void updateActionEnableFilter();

    /**
    * @brief Adds the actions in the actionList parameter to the right-click menu
    */
    void addActionList(QList<QAction*> actionList);

  signals:
    void itemWasDropped(QModelIndex parent, QString& title, QIcon icon, QString path, int index, bool allowEditing, bool editState, bool isExpanding);
    void currentIndexChanged(const QModelIndex& current, const QModelIndex& previous);
    void folderChangedState(const QModelIndex& index, bool expand);
    void contextMenuRequested(const QPoint& pos);
    void filterEnabledStateChanged();

    void needsPreflight(const QModelIndex &pipelineIndex);
    void activePipelineChanged(const QModelIndex &pipelineIdx);

  private slots:
    void requestContextMenu(const QPoint &pos);

  private:
    QPoint                                        m_StartPos;
    QMenu                                         m_Menu;
    QAction*                                      m_ActionEnableFilter;
    QList<QPersistentModelIndex>                  m_IndexesBeingDragged;
    QPersistentModelIndex                         m_ActiveIndexBeingDragged;
    QModelIndex                                   m_TopLevelItemPlaceholder;

    void expandChildren(const QModelIndex& parent, PipelineTreeModel* model);
    QJsonObject wrapModel(QModelIndex index);
    static void UnwrapModel(QString objectName, QJsonObject object, PipelineTreeModel* model, QModelIndex parentIndex);

    /**
     * @brief requestSinglePipelineContextMenu
     * @param menu
     * @param pipelineIdx
     */
    void requestSinglePipelineContextMenu(QMenu &menu, const QModelIndex &pipelineIdx);

    /**
     * @brief requestMultiplePipelineContextMenu
     * @param menu
     * @param pipelineIndices
     */
    void requestMultiplePipelineContextMenu(QMenu &menu, QModelIndexList pipelineIndices);

    /**
     * @brief findNewActivePipeline
     * @param oldActivePipeline
     * @return
     */
    QModelIndex findNewActivePipeline(const QModelIndex &oldActivePipeline);

    /**
     * @brief requestFilterContextMenu
     * @param pos
     * @param index
     */
    void requestFilterContextMenu(const QPoint &pos, const QModelIndex &index);

    /**
     * @brief requestPipelineContextMenu
     * @param pos
     * @param index
     */
    void requestPipelineContextMenu(const QPoint &pos, const QModelIndex &index);

    /**
     * @brief requestDefaultContextMenu
     * @param pos
     */
    void requestDefaultContextMenu(const QPoint &pos);
};

#endif /* _PipelineTreeView_H_ */
