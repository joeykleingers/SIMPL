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

#include "PipelineView.h"

#include <QtGui/QClipboard>

#include <QtWidgets/QApplication>
#include <QtWidgets/QMenu>

#include "SIMPLib/Common/DocRequestManager.h"
#include "SIMPLib/FilterParameters/JsonFilterParametersReader.h"
#include "SIMPLib/FilterParameters/JsonFilterParametersWriter.h"

#include "SVWidgetsLib/Core/SVWidgetsLibConstants.h"
#include "SVWidgetsLib/Widgets/PipelineModel.h"
#include "SVWidgetsLib/Widgets/PipelineViewController.h"
#include "SVWidgetsLib/Widgets/IssuesWidget.h"
#include "SVWidgetsLib/Widgets/StandardOutputWidget.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PipelineView::PipelineView() :
  m_PipelineViewController(new PipelineViewController())
{
  setupGui();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PipelineView::~PipelineView()
{
  delete m_ActionEnableFilter;
  delete m_ActionCut;
  delete m_ActionCopy;
  delete m_ActionPaste;
  delete m_ActionClearPipeline;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineView::setupGui()
{
  // Delete action if it exists
  if(m_ActionEnableFilter)
  {
    delete m_ActionEnableFilter;
  }

  m_ActionEnableFilter = new QAction("Enable");
  m_ActionEnableFilter->setCheckable(true);
  m_ActionEnableFilter->setChecked(true);
  m_ActionEnableFilter->setEnabled(false);

  m_ActionCut = new QAction("Cut");
  m_ActionCopy = new QAction("Copy");
  m_ActionPaste = new QAction("Paste");
  m_ActionClearPipeline = new QAction("Clear Pipeline");

  m_ActionCut->setShortcut(QKeySequence::Cut);
  m_ActionCopy->setShortcut(QKeySequence::Copy);
  m_ActionPaste->setShortcut(QKeySequence::Paste);

  m_ActionCut->setDisabled(true);
  m_ActionCopy->setDisabled(true);

  m_ActionClearPipeline->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_Backspace));

  // Run this once, so that the Paste button availability is updated for what is currently on the system clipboard
  updatePasteAvailability();

  QClipboard* clipboard = QApplication::clipboard();
  QObject::connect(clipboard, &QClipboard::dataChanged, [=] { updatePasteAvailability(); });
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineView::connectSignalsSlots()
{
  QObject::connect(m_ActionClearPipeline, &QAction::triggered, [=] { clearPipeline(); });

  QObject::connect(m_ActionCut, &QAction::triggered, [=] {
    copySelectedFilters();

    std::vector<AbstractFilter::Pointer> filters = getSelectedFilters();
    cutFilters(filters);
  });

  QObject::connect(m_ActionCopy, &QAction::triggered, [=] { copySelectedFilters(); });
  QObject::connect(m_ActionPaste, &QAction::triggered, [=] { pasteFilters(); });
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------s
void PipelineView::updatePasteAvailability()
{
  QClipboard* clipboard = QApplication::clipboard();
  QString text = clipboard->text();

  JsonFilterParametersReader::Pointer jsonReader = JsonFilterParametersReader::New();
  FilterPipeline::Pointer pipeline = jsonReader->readPipelineFromString(text);

  if(text.isEmpty() || FilterPipeline::NullPointer() == pipeline)
  {
    m_ActionPaste->setDisabled(true);
  }
  else
  {
    m_ActionPaste->setEnabled(true);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineView::addPipelineMessageObserver(QObject* pipelineMessageObserver)
{
  if(m_PipelineViewController)
  {
    m_PipelineViewController->addPipelineMessageObserver(pipelineMessageObserver);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int PipelineView::writePipeline(const QModelIndex& pipelineRootIndex, const QString& outputPath)
{
  if(m_PipelineViewController)
  {
    return m_PipelineViewController->writePipeline(pipelineRootIndex, outputPath);
  }

  return -1;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineView::copySelectedFilters()
{
  FilterPipeline::Pointer pipeline = FilterPipeline::New();
  std::vector<AbstractFilter::Pointer> filters = getSelectedFilters();
  for(int i = 0; i < filters.size(); i++)
  {
    pipeline->pushBack(filters[i]);
  }

  JsonFilterParametersWriter::Pointer jsonWriter = JsonFilterParametersWriter::New();
  QString jsonString = jsonWriter->writePipelineToString(pipeline, "Pipeline");

  QClipboard* clipboard = QApplication::clipboard();
  clipboard->setText(jsonString);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineView::requestFilterItemContextMenu(const QPoint& pos, const QModelIndex& index)
{
  if(m_PipelineViewController)
  {
    PipelineModel* model = getPipelineModel();
    QModelIndexList selectedIndexes = getSelectedRows();
    qSort(selectedIndexes);

    QMenu menu;

    menu.addAction(getActionCut());
    menu.addAction(getActionCopy());
    menu.addSeparator();

    QAction* actionPasteAbove = new QAction("Paste Above");
    QAction* actionPasteBelow = new QAction("Paste Below");

    QObject::connect(actionPasteAbove, &QAction::triggered, [=] { pasteFilters(index.row()); });

    QObject::connect(actionPasteBelow, &QAction::triggered, [=] { pasteFilters(index.row() + 1); });

    menu.addAction(actionPasteAbove);
    menu.addAction(actionPasteBelow);
    menu.addSeparator();

    int count = selectedIndexes.size();
    bool widgetEnabled = true;

    for(int i = 0; i < count && widgetEnabled; i++)
    {
      AbstractFilter::Pointer filter = model->filter(selectedIndexes[i]);
      if(filter != nullptr)
      {
        widgetEnabled = filter->getEnabled();
      }
    }

    if(selectedIndexes.contains(index) == false)
    {
      // Only toggle the target filter widget if it is not in the selected objects
      QModelIndexList toggledIndices = QModelIndexList();
      toggledIndices.push_back(index);

      AbstractFilter::Pointer filter = model->filter(index);
      if(filter != nullptr)
      {
        widgetEnabled = filter->getEnabled();
      }

      QObject::disconnect(m_ActionEnableFilter, &QAction::toggled, 0, 0);
      QObject::connect(m_ActionEnableFilter, &QAction::toggled, [=] { m_PipelineViewController->setFiltersEnabled(toggledIndices, m_ActionEnableFilter->isChecked()); });
    }
    else
    {
      QObject::disconnect(m_ActionEnableFilter, &QAction::toggled, 0, 0);
      QObject::connect(m_ActionEnableFilter, &QAction::toggled, [=] { m_PipelineViewController->setFiltersEnabled(selectedIndexes, m_ActionEnableFilter->isChecked()); });
    }

    m_ActionEnableFilter->setChecked(widgetEnabled);
    m_ActionEnableFilter->setEnabled(true);
//    m_ActionEnableFilter->setDisabled(getPipelineIsRunning());
    menu.addAction(m_ActionEnableFilter);

    menu.addSeparator();

    QAction* removeAction;
    QList<QKeySequence> shortcutList;
    shortcutList.push_back(QKeySequence(Qt::Key_Backspace));
    shortcutList.push_back(QKeySequence(Qt::Key_Delete));

    if(selectedIndexes.contains(index) == false || selectedIndexes.size() == 1)
    {
      removeAction = new QAction("Delete Filter", &menu);
      QObject::connect(removeAction, &QAction::triggered, [=] {
        AbstractFilter::Pointer filter = model->filter(index);
        removeFilter(filter);
      });
    }
    else
    {
      removeAction = new QAction(QObject::tr("Delete %1 Filters").arg(selectedIndexes.size()), &menu);
      QObject::connect(removeAction, &QAction::triggered, [=] {
        QList<QPersistentModelIndex> persistentList;
        for(int i = 0; i < selectedIndexes.size(); i++)
        {
          persistentList.push_back(selectedIndexes[i]);
        }

        std::vector<AbstractFilter::Pointer> filters;
        for(int i = 0; i < persistentList.size(); i++)
        {
          AbstractFilter::Pointer filter = model->filter(persistentList[i]);
          filters.push_back(filter);
        }

        removeFilters(filters);
      });
    }
    removeAction->setShortcuts(shortcutList);
//    if(getPipelineIsRunning() == true)
//    {
//      removeAction->setDisabled(true);
//    }

    menu.addAction(removeAction);

    menu.addAction(m_ActionClearPipeline);

    menu.addSeparator();

    // Error Handling Menu
    requestErrorHandlingContextMenu(menu);
    menu.addSeparator();

    QAction* actionLaunchHelp = new QAction("Filter Help");
    QObject::connect(actionLaunchHelp, &QAction::triggered, [=] {
      AbstractFilter::Pointer filter = model->filter(index);
      if(filter != nullptr)
      {
        // Launch the help for this filter
        QString className = filter->getNameOfClass();

        DocRequestManager* docRequester = DocRequestManager::Instance();
        docRequester->requestFilterDocs(className);
      }
    });

    menu.addAction(actionLaunchHelp);

    menu.exec(pos);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineView::requestPipelineItemContextMenu(const QPoint& pos)
{
  QMenu menu;

  menu.addAction(m_ActionPaste);

  requestSinglePipelineContextMenu(menu);

  requestErrorHandlingContextMenu(menu);

  menu.exec(pos);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineView::requestSinglePipelineContextMenu(QMenu& menu)
{
  menu.addSeparator();

  menu.addAction(m_ActionClearPipeline);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineView::requestDefaultContextMenu(const QPoint& pos)
{
  QMenu menu;
  menu.addAction(m_ActionPaste);
  menu.addSeparator();
  menu.addAction(m_ActionClearPipeline);

  menu.exec(pos);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineView::requestErrorHandlingContextMenu(QMenu& menu)
{
  menu.addSeparator();

  QMenu* errorMenu = menu.addMenu("Error Handling");

  QMenu* combinedMenu = errorMenu->addMenu("All");
  QAction* showCombinedErrorAction = combinedMenu->addAction("Show on Error");
  QAction* ignoreCombinedErrorAction = combinedMenu->addAction("Ignore on Error");

  QMenu* errorTableMenu = errorMenu->addMenu("Issues Table");
  QAction* showTableErrorAction = errorTableMenu->addAction("Show on Error");
  QAction* ignoreTableErrorAction = errorTableMenu->addAction("Ignore on Error");

  QMenu* stdOutMenu = errorMenu->addMenu("Standard Output");
  QAction* showStdOutErrorAction = stdOutMenu->addAction("Show on Error");
  QAction* ignoreStdOutErrorAction = stdOutMenu->addAction("Ignore on Error");

  menu.addSeparator();

  showTableErrorAction->setCheckable(true);
  ignoreTableErrorAction->setCheckable(true);
  showStdOutErrorAction->setCheckable(true);
  ignoreStdOutErrorAction->setCheckable(true);
  showCombinedErrorAction->setCheckable(true);
  ignoreCombinedErrorAction->setCheckable(true);

  // Set Checked based on user preferences
  SIMPLView::DockWidgetSettings::HideDockSetting issuesTableSetting = IssuesWidget::GetHideDockSetting();
  SIMPLView::DockWidgetSettings::HideDockSetting stdOutSetting = StandardOutputWidget::GetHideDockSetting();

  bool showTableError = (issuesTableSetting != SIMPLView::DockWidgetSettings::HideDockSetting::Ignore);
  bool showStdOutput = (stdOutSetting != SIMPLView::DockWidgetSettings::HideDockSetting::Ignore);
  bool showCombinedError = showTableError && showStdOutput;
  bool ignoreCombinedError = !showTableError && !showStdOutput;

  showTableErrorAction->setChecked(showTableError);
  ignoreTableErrorAction->setChecked(!showTableError);
  showStdOutErrorAction->setChecked(showStdOutput);
  ignoreStdOutErrorAction->setChecked(!showStdOutput);
  showCombinedErrorAction->setChecked(showCombinedError);
  ignoreCombinedErrorAction->setChecked(ignoreCombinedError);

  // Connect actions
  // Issues Widget
  QObject::connect(showTableErrorAction, &QAction::triggered, [=]() {
    IssuesWidget::SetHideDockSetting(SIMPLView::DockWidgetSettings::HideDockSetting::OnError);
    preflightPipeline();
  });
  QObject::connect(ignoreTableErrorAction, &QAction::triggered, [=]() {
    IssuesWidget::SetHideDockSetting(SIMPLView::DockWidgetSettings::HideDockSetting::Ignore);
    preflightPipeline();
  });

  // Standard Output
  QObject::connect(showStdOutErrorAction, &QAction::triggered, [=]() {
    StandardOutputWidget::SetHideDockSetting(SIMPLView::DockWidgetSettings::HideDockSetting::OnError);
    preflightPipeline();
  });
  QObject::connect(ignoreStdOutErrorAction, &QAction::triggered, [=]() {
    StandardOutputWidget::SetHideDockSetting(SIMPLView::DockWidgetSettings::HideDockSetting::Ignore);
    preflightPipeline();
  });

  // Combined
  QObject::connect(showCombinedErrorAction, &QAction::triggered, [=]() {
    IssuesWidget::SetHideDockSetting(SIMPLView::DockWidgetSettings::HideDockSetting::OnError);
    StandardOutputWidget::SetHideDockSetting(SIMPLView::DockWidgetSettings::HideDockSetting::OnError);
    preflightPipeline();
  });
  QObject::connect(ignoreCombinedErrorAction, &QAction::triggered, [=]() {
    IssuesWidget::SetHideDockSetting(SIMPLView::DockWidgetSettings::HideDockSetting::Ignore);
    StandardOutputWidget::SetHideDockSetting(SIMPLView::DockWidgetSettings::HideDockSetting::Ignore);
    preflightPipeline();
  });
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QAction* PipelineView::getActionUndo()
{
  if (m_PipelineViewController)
  {
    return m_PipelineViewController->getActionUndo();
  }
  return nullptr;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QAction* PipelineView::getActionRedo()
{
  if (m_PipelineViewController)
  {
    return m_PipelineViewController->getActionRedo();
  }
  return nullptr;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
std::vector<AbstractFilter::Pointer> PipelineView::getSelectedFilters()
{
  QModelIndexList selectedIndexes = getSelectedRows();
  qSort(selectedIndexes);

  std::vector<AbstractFilter::Pointer> filters;
  PipelineModel* model = getPipelineModel();
  for(int i = 0; i < selectedIndexes.size(); i++)
  {
    AbstractFilter::Pointer filter = model->filter(selectedIndexes[i]);
    filters.push_back(filter);
  }

  return filters;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PipelineModel* PipelineView::getPipelineModel()
{
  if (m_PipelineViewController)
  {
    return m_PipelineViewController->getPipelineModel();
  }
  return nullptr;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineView::setSelectedFiltersEnabled(bool enabled)
{
  if (m_PipelineViewController)
  {
    QModelIndexList indexes = getSelectedRows();
    qSort(indexes);
    m_PipelineViewController->setFiltersEnabled(indexes, enabled);
  }
}
