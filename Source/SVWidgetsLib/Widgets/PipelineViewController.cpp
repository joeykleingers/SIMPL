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

#include "PipelineViewController.h"

#include <QtConcurrent>

#include <QtGui/QClipboard>

#include <QtWidgets/QMenu>

#include <QtWidgets/QUndoStack>
#include <QtWidgets/QUndoCommand>
#include <QtWidgets/QWidget>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QFileDialog>

#include "SIMPLib/Common/DocRequestManager.h"
#include "SIMPLib/CoreFilters/DataContainerReader.h"
#include "SIMPLib/Filtering/FilterManager.h"
#include "SIMPLib/FilterParameters/H5FilterParametersReader.h"
#include "SIMPLib/FilterParameters/H5FilterParametersWriter.h"
#include "SIMPLib/FilterParameters/JsonFilterParametersReader.h"
#include "SIMPLib/FilterParameters/JsonFilterParametersWriter.h"

#include "SVWidgetsLib/Core/SVWidgetsLibConstants.h"
#include "SVWidgetsLib/Widgets/PipelineModel.h"
#include "SVWidgetsLib/Widgets/PipelineView.h"
#include "SVWidgetsLib/Widgets/PipelineItem.h"
#include "SVWidgetsLib/Widgets/FilterInputWidget.h"
#include "SVWidgetsLib/Widgets/IssuesWidget.h"
#include "SVWidgetsLib/Widgets/StandardOutputWidget.h"
#include "SVWidgetsLib/Widgets/AddPipelineToModelCommand.h"
#include "SVWidgetsLib/Widgets/AddFilterToPipelineCommand.h"
#include "SVWidgetsLib/Widgets/RemoveFilterFromPipelineCommand.h"
#include "SVWidgetsLib/Widgets/RemovePipelineFromModelCommand.h"
#include "SVWidgetsLib/QtSupport/QtSFileDragMessageBox.h"
#include "SVWidgetsLib/QtSupport/QtSRecentFileList.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PipelineViewController::PipelineViewController(QObject* parent) :
  QObject(parent)
, m_LastOpenedFilePath(QDir::homePath())

{
  initialize();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PipelineViewController::PipelineViewController(PipelineView* pipelineView, QObject* parent) :
  QObject(parent)
, m_PipelineView(pipelineView)
, m_PipelineModel(pipelineView->getPipelineModel())
{
  initialize();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PipelineViewController::~PipelineViewController()
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
void PipelineViewController::initialize()
{
  setupUndoStack();

  setupActions();

  connectSignalsSlots();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewController::setupActions()
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
  QObject::connect(clipboard, &QClipboard::dataChanged, this, &PipelineViewController::updatePasteAvailability);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool PipelineViewController::savePipeline(const QModelIndex &pipelineRootIndex)
{
  PipelineModel* model = getPipelineModel();
  PipelineItem::ItemType itemType = static_cast<PipelineItem::ItemType>(model->data(pipelineRootIndex, PipelineModel::Roles::ItemTypeRole).toInt());
  if (model == nullptr || itemType != PipelineItem::ItemType::PipelineRoot)
  {
    return false;
  }

  QString filePath = model->data(pipelineRootIndex, PipelineModel::Roles::PipelinePathRole).toString();
  if(filePath.isEmpty())
  {
    // When the file hasn't been saved before, the same functionality as a "Save As" occurs...
    return savePipelineAs(pipelineRootIndex);
  }

  // Write the pipeline
  int err = writePipeline(pipelineRootIndex, filePath);

  if(err >= 0)
  {
    QFileInfo fi(filePath);
    model->savePipeline(pipelineRootIndex, fi.baseName());
    model->setData(pipelineRootIndex, filePath, PipelineModel::Roles::PipelinePathRole);
    model->setData(pipelineRootIndex, false, PipelineModel::Roles::PipelineModifiedRole);

    // Cache the last directory
    m_LastOpenedFilePath = filePath;

    // Add file to the recent files list
    QtSRecentFileList* list = QtSRecentFileList::Instance();
    list->addFile(filePath);

    return true;
  }

  return false;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool PipelineViewController::savePipelineAs(const QModelIndex &pipelineRootIndex)
{
  PipelineModel* model = getPipelineModel();
  PipelineItem::ItemType itemType = static_cast<PipelineItem::ItemType>(model->data(pipelineRootIndex, PipelineModel::Roles::ItemTypeRole).toInt());
  if (model == nullptr || itemType != PipelineItem::ItemType::PipelineRoot)
  {
    return false;
  }

  QString proposedFile = m_LastOpenedFilePath + QDir::separator() + "Untitled.json";
  QString filePath = QFileDialog::getSaveFileName(nullptr, tr("Save Pipeline To File"), proposedFile, tr("Json File (*.json);;SIMPLView File (*.dream3d);;All Files (*.*)"));
  if(true == filePath.isEmpty())
  {
    return false;
  }

  filePath = QDir::toNativeSeparators(filePath);

  // If the filePath already exists - delete it so that we get a clean write to the file
  QFileInfo fi(filePath);
  if(fi.suffix().isEmpty())
  {
    filePath.append(".json");
    fi.setFile(filePath);
  }

  // Write the pipeline
  int err = writePipeline(pipelineRootIndex, filePath);

  if(err >= 0)
  {
    model->savePipeline(pipelineRootIndex, fi.baseName());
    model->setData(pipelineRootIndex, filePath, PipelineModel::Roles::PipelinePathRole);
    model->setData(pipelineRootIndex, false, PipelineModel::Roles::PipelineModifiedRole);

    // Cache the last directory
    m_LastOpenedFilePath = filePath;

    // Add file to the recent files list
    QtSRecentFileList* list = QtSRecentFileList::Instance();
    list->addFile(filePath);

    emit pipelineSavedAs(pipelineRootIndex, filePath);

    return true;
  }

  return false;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------s
void PipelineViewController::updatePasteAvailability()
{
  QClipboard* clipboard = QApplication::clipboard();
  QString jsonString = clipboard->text();
  if (jsonString.isEmpty())
  {
    return;
  }

  QJsonParseError parseError;
  QByteArray byteArray = QByteArray::fromStdString(jsonString.toStdString());
  QJsonDocument doc = QJsonDocument::fromJson(byteArray, &parseError);
  if(parseError.error != QJsonParseError::NoError)
  {
    return;
  }
  QJsonArray pipelinesArray = doc.array();

  if (pipelinesArray.size() <= 0)
  {
    m_ActionPaste->setDisabled(true);
    return;
  }

  JsonFilterParametersReader::Pointer jsonReader = JsonFilterParametersReader::New();
  for (int i = 0; i < pipelinesArray.size(); i++)
  {
    QJsonObject obj = pipelinesArray[i].toObject();
    FilterPipeline::Pointer pipeline = jsonReader->readPipelineFromObject(obj);
    if(FilterPipeline::NullPointer() == pipeline)
    {
      m_ActionPaste->setDisabled(true);
      return;
    }
  }

  m_ActionPaste->setEnabled(true);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewController::connectSignalsSlots()
{
  QObject::connect(m_ActionClearPipeline, &QAction::triggered, [=] { m_PipelineView->clearPipeline(); });

//  connect(this, &PipelineViewController::pipelineFilePathUpdated, [=](const QString& name) { m_CurrentPipelineFilePath = name; });

  connect(m_ActionCut, &QAction::triggered, this, &PipelineViewController::listenCutTriggered);
  connect(m_ActionCopy, &QAction::triggered, this, &PipelineViewController::listenCopyTriggered);
  connect(m_ActionPaste, &QAction::triggered, this, &PipelineViewController::listenPasteTriggered);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewController::listenCutTriggered()
{
  copySelection();

  QModelIndexList selectedIndexes = m_PipelineView->getSelectedRows();

  QMultiMap<QModelIndex, AbstractFilter::Pointer> cutFilterMap;
  for (int i = 0; i < selectedIndexes.size(); i++)
  {
    QModelIndex selectedIndex = selectedIndexes[i];
    PipelineItem::ItemType itemType = static_cast<PipelineItem::ItemType>(m_PipelineModel->data(selectedIndex, PipelineModel::Roles::ItemTypeRole).toInt());
    if (itemType == PipelineItem::ItemType::PipelineRoot)
    {
      cutPipeline(selectedIndex);
    }
    else
    {
      AbstractFilter::Pointer filter = m_PipelineModel->filter(selectedIndex);
      cutFilterMap.insert(selectedIndex.parent(), filter);
    }
  }

  QModelIndexList pipelineRootIndexes = cutFilterMap.uniqueKeys();
  if (pipelineRootIndexes.size() > 0)
  {
    m_UndoStack->beginMacro(tr("\"Cut %1 Filters\""));
    for (int i = 0; i < pipelineRootIndexes.size(); i++)
    {
      QModelIndex pipelineRootIndex = pipelineRootIndexes[i];
      QList<AbstractFilter::Pointer> filters = cutFilterMap.values(pipelineRootIndex);
      std::vector<AbstractFilter::Pointer> filterVector;
      for (int j = 0; j < filters.size(); j++)
      {
        filterVector.push_back(filters[j]);
      }
      cutFilters(filterVector, pipelineRootIndex);
    }
    m_UndoStack->endMacro();
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewController::listenCopyTriggered()
{
  m_PipelineView->copySelection();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewController::listenPasteTriggered()
{  
  QJsonArray pipelinesArray = getPipelinesArrayFromClipboard();
  if (pipelinesArray.size() == 1)
  {
    QJsonObject pipelineObj = pipelinesArray[0].toObject();
    bool hasPipelineRootNode = pipelineObj["Pipeline Root Node"].toBool();
    if (hasPipelineRootNode)
    {
      QJsonObject meta = pipelineObj[SIMPL::Settings::PipelineBuilderGroup].toObject();
      m_UndoStack->beginMacro(tr("\"Paste pipeline '%1'\"").arg(meta[SIMPL::Settings::PipelineName].toString()));
    }
    else
    {
      JsonFilterParametersReader::Pointer jsonReader = JsonFilterParametersReader::New();
      FilterPipeline::Pointer pipeline = jsonReader->readPipelineFromObject(pipelineObj);
      FilterPipeline::FilterContainerType container = pipeline->getFilterContainer();
      if (pipeline->size() > 1)
      {
        m_UndoStack->beginMacro(tr("\"Paste '%1'\"").arg(container[0]->getHumanLabel()));
      }
      else
      {
        m_UndoStack->beginMacro(tr("\"Paste %1 filters\"").arg(pipeline->size()));
      }
    }
  }
  else
  {
    m_UndoStack->beginMacro(tr("\"Paste %1 pipelines\"").arg(pipelinesArray.size()));
  }

  JsonFilterParametersReader::Pointer jsonReader = JsonFilterParametersReader::New();
  for (int i = 0; i < pipelinesArray.size(); i++)
  {
    QJsonObject obj = pipelinesArray[i].toObject();
    bool hasPipelineRootNode = obj["Pipeline Root Node"].toBool();
    FilterPipeline::Pointer pipeline = jsonReader->readPipelineFromObject(obj);

    if (hasPipelineRootNode)
    {
      m_PipelineView->pastePipeline(pipeline);
    }
    else
    {
      FilterPipeline::FilterContainerType container = pipeline->getFilterContainer();
      std::vector<AbstractFilter::Pointer> filters;
      for(int i = 0; i < container.size(); i++)
      {
        filters.push_back(container[i]);
      }

      m_PipelineView->pasteFilters(filters);
    }
  }

  m_UndoStack->endMacro();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewController::listenClearPipelineTriggered()
{
  m_PipelineView->clearPipeline();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewController::addPipelineMessageObserver(QObject* pipelineMessageObserver)
{
  m_PipelineMessageObservers.push_back(pipelineMessageObserver);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewController::addFilterFromClassName(const QString& filterClassName, const QModelIndex &pipelineRootIndex, int insertIndex, QString actionText)
{
  if (m_PipelineModel)
  {
    FilterManager* fm = FilterManager::Instance();
    if(fm != nullptr)
    {
      IFilterFactory::Pointer factory = fm->getFactoryFromClassName(filterClassName);
      if(factory.get() != nullptr)
      {
        AbstractFilter::Pointer filter = factory->create();
        addFilter(filter, pipelineRootIndex, insertIndex, actionText);
      }
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewController::addFilter(AbstractFilter::Pointer filter, const QModelIndex &pipelineRootIndex, int insertIndex, QString actionText)
{
  std::vector<AbstractFilter::Pointer> filters;
  filters.push_back(filter);
  addFilters(filters, pipelineRootIndex, insertIndex, actionText);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewController::addFilters(std::vector<AbstractFilter::Pointer> filters, const QModelIndex &pipelineRootIndex, int insertIndex, QString actionText)
{
  if (m_PipelineModel)
  {
    if (actionText.isEmpty())
    {
      actionText = "Add";
    }

    FilterPipeline::Pointer pipeline = m_PipelineModel->tempPipeline(pipelineRootIndex);
    if (pipeline.get() == nullptr)
    {
      pipeline = FilterPipeline::New();
      pipeline->insert(0, filters);
      pipeline->setName("Untitled");
      addPipeline(pipeline, -1, "", actionText);
      QModelIndex index = m_PipelineModel->index(m_PipelineModel->rowCount() - 1, PipelineItem::Contents);
      m_PipelineModel->setData(index, true, PipelineModel::Roles::PipelineModifiedRole);
      return;
    }

    QString lcActionText = actionText.toLower();

    if (filters.size() > 0)
    {
      bool allFiltersValid = true;
      foreach(AbstractFilter::Pointer filter, filters)
      {
        if (filter.get() == nullptr)
        {
          allFiltersValid = false;
        }
      }

      if (allFiltersValid == true)
      {
        AddFilterToPipelineCommand* cmd = new AddFilterToPipelineCommand(filters, pipeline, insertIndex, m_PipelineModel);
        if(filters.size() == 1)
        {
          cmd->setText(QObject::tr("\"%1 '%2' to '%3'\"").arg(actionText).arg(filters[0]->getHumanLabel()).arg(pipeline->getName()));
        }
        else
        {
          cmd->setText(QObject::tr("\"%1 %2 filters to '%3'\"").arg(actionText).arg(filters.size()).arg(pipeline->getName()));
        }
        addUndoCommand(cmd);
      }
      else
      {
        emit errorMessage(tr("Could not %1 filters to pipeline '%2' because some of the filter objects are invalid.\n"
                             "Please contact the DREAM.3D developers for more information.").arg(lcActionText).arg(pipeline->getName()));
      }
    }
    else
    {
      emit errorMessage(tr("Could not %1 filters to pipeline '%2' because the vector of filters is empty.\n"
                           "Please contact the DREAM.3D developers for more information.").arg(lcActionText).arg(pipeline->getName()));
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewController::addPipeline(FilterPipeline::Pointer pipeline, int insertIndex, const QString &pipelineFilePath, QString actionText)
{
  if (m_PipelineModel)
  {
    AddPipelineToModelCommand* cmd = new AddPipelineToModelCommand(pipeline, insertIndex, m_PipelineModel, pipelineFilePath);
    cmd->setText(QObject::tr("\"%1 '%2' Pipeline\"").arg(actionText).arg(pipeline->getName()));
    addUndoCommand(cmd);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewController::removeFilter(AbstractFilter::Pointer filter, const QModelIndex &pipelineRootIndex, QString actionText)
{
  std::vector<AbstractFilter::Pointer> filters;
  filters.push_back(filter);
  removeFilters(filters, pipelineRootIndex, actionText);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewController::removeFilters(std::vector<AbstractFilter::Pointer> filters, const QModelIndex &pipelineRootIndex, QString actionText)
{
  if (m_PipelineModel)
  {
    if (actionText.isEmpty())
    {
      actionText = "Remove";
    }

    FilterPipeline::Pointer pipeline = m_PipelineModel->tempPipeline(pipelineRootIndex);
    QString lcActionText = actionText.toLower();
    if (pipeline.get() != nullptr)
    {
      if (filters.size() > 0)
      {
        bool allFiltersValid = true;
        foreach(AbstractFilter::Pointer filter, filters)
        {
          if (filter.get() == nullptr)
          {
            allFiltersValid = false;
          }
        }

        if (allFiltersValid == true)
        {
          RemoveFilterFromPipelineCommand* cmd = new RemoveFilterFromPipelineCommand(filters, pipeline, m_PipelineModel);
          if(filters.size() == 1)
          {
            cmd->setText(QObject::tr("\"%1 '%2' from '%3'\"").arg(actionText).arg(filters[0]->getHumanLabel()).arg(pipeline->getName()));
          }
          else
          {
            cmd->setText(QObject::tr("\"%1 %2 filters from '%3'\"").arg(actionText).arg(filters.size()).arg(pipeline->getName()));
          }
          addUndoCommand(cmd);
        }
        else
        {
          emit errorMessage(tr("Could not %1 filters from pipeline '%2' because some of the filter objects are invalid.\n"
                               "Please contact the DREAM.3D developers for more information.").arg(lcActionText).arg(pipeline->getName()));
        }
      }
      else
      {
        emit errorMessage(tr("Could not %1 filters from pipeline '%2' because the vector of filters is empty.\n"
                             "Please contact the DREAM.3D developers for more information.").arg(lcActionText).arg(pipeline->getName()));
      }
    }
    else
    {
      emit errorMessage(tr("Could not %1 filters from pipeline '%2' because the pipeline object is invalid.\n"
                           "Please contact the DREAM.3D developers for more information."));
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewController::removePipeline(const QModelIndex &pipelineRootIndex, QString actionText)
{
  if (m_PipelineModel)
  {
    FilterPipeline::Pointer pipeline = m_PipelineModel->tempPipeline(pipelineRootIndex);
    removePipeline(pipeline, actionText);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewController::removePipeline(FilterPipeline::Pointer pipeline, QString actionText)
{
  if (m_PipelineModel)
  {
    RemovePipelineFromModelCommand* cmd = new RemovePipelineFromModelCommand(pipeline, m_PipelineModel);
    cmd->setText(QObject::tr("\"%1 '%2' Pipeline\"").arg(actionText).arg(pipeline->getName()));
    addUndoCommand(cmd);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewController::cutFilter(AbstractFilter::Pointer filter, const QModelIndex &pipelineRootIndex)
{
  std::vector<AbstractFilter::Pointer> filters;
  filters.push_back(filter);
  removeFilters(filters, pipelineRootIndex, "Cut");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewController::cutFilters(std::vector<AbstractFilter::Pointer> filters, const QModelIndex &pipelineRootIndex)
{
  removeFilters(filters, pipelineRootIndex, "Cut");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewController::cutPipeline(FilterPipeline::Pointer pipeline)
{
  removePipeline(pipeline, "Cut");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewController::cutPipeline(const QModelIndex &pipelineRootIndex)
{
  removePipeline(pipelineRootIndex, "Cut");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewController::copySelection()
{
  QJsonArray pipelineArray;
  QModelIndexList selectedIndexes = m_PipelineView->getSelectedRows();

  JsonFilterParametersWriter::Pointer jsonWriter = JsonFilterParametersWriter::New();
  FilterPipeline::Pointer filtersPipeline = FilterPipeline::New();
  for (int i = 0; i < selectedIndexes.size(); i++)
  {
    QModelIndex selectedIndex = selectedIndexes[i];
    PipelineItem::ItemType itemType = static_cast<PipelineItem::ItemType>(m_PipelineModel->data(selectedIndex, PipelineModel::Roles::ItemTypeRole).toInt());
    if (itemType == PipelineItem::ItemType::PipelineRoot)
    {
      FilterPipeline::Pointer pipeline = m_PipelineModel->tempPipeline(selectedIndex);
      QString filePath = m_PipelineModel->data(selectedIndex, PipelineModel::Roles::PipelinePathRole).toString();
      QJsonObject obj = jsonWriter->writePipelineToObject(pipeline, pipeline->getName());
      obj["Pipeline Root Node"] = true;
      pipelineArray.push_back(obj);
    }
    else
    {
      AbstractFilter::Pointer filter = m_PipelineModel->filter(selectedIndex);
      filtersPipeline->pushBack(filter);
    }
  }

  if (filtersPipeline->size() > 0)
  {
    QJsonObject obj = jsonWriter->writePipelineToObject(filtersPipeline, "Untitled");
    obj["Pipeline Root Node"] = false;
    pipelineArray.push_back(obj);
  }

  QJsonDocument doc(pipelineArray);
  QString jsonString = QString::fromStdString(doc.toJson().toStdString());

  QClipboard* clipboard = QApplication::clipboard();
  clipboard->setText(jsonString);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewController::pasteFilters(std::vector<AbstractFilter::Pointer> filters, const QModelIndex &pipelineRootIndex, int insertIndex)
{
  addFilters(filters, pipelineRootIndex, insertIndex, "Paste");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewController::pastePipeline(FilterPipeline::Pointer pipeline, int insertIndex)
{
  addPipeline(pipeline, insertIndex, "", "Paste");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewController::setCutCopyEnabled(bool enabled)
{
  m_ActionCut->setEnabled(enabled);
  m_ActionCopy->setEnabled(enabled);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewController::blockPreflightSignals(bool b)
{
  if(b)
  {
    m_BlockPreflightStack.push(b);
  }
  else
  {
    m_BlockPreflightStack.pop();
  }

  m_BlockPreflight = (m_BlockPreflightStack.size() > 0) ? true : false;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewController::preflightPipeline(const QModelIndex &pipelineRootIndex)
{
  if (m_PipelineModel)
  {
    if(m_BlockPreflight)
    {
      return;
    }

    m_PipelineModel->clearPipelineMessages(pipelineRootIndex);

    // Create a Pipeline Object and fill it with the filters from this View
    FilterPipeline::Pointer pipeline = m_PipelineModel->tempPipeline(pipelineRootIndex);

    for(int i = 0; i < m_PipelineMessageObservers.size(); i++)
    {
      pipeline->addMessageReceiver(m_PipelineMessageObservers[i]);
    }

    pipeline->addMessageReceiver(m_PipelineModel->pipelineOutputTextEdit(pipelineRootIndex));
    pipeline->addMessageReceiver(m_PipelineModel->pipelineMessageObserver(pipelineRootIndex));

    FilterPipeline::FilterContainerType filters = pipeline->getFilterContainer();
    for(int i = 0; i < filters.size(); i++)
    {
      filters.at(i)->setErrorCondition(0);
      filters.at(i)->setCancel(false);

      QModelIndex childIndex = m_PipelineModel->index(i, PipelineItem::Contents, pipelineRootIndex);
      if(childIndex.isValid())
      {
        m_PipelineModel->setData(childIndex, static_cast<int>(PipelineItem::ErrorState::Ok), PipelineModel::ErrorStateRole);
        AbstractFilter::Pointer filter = m_PipelineModel->filter(childIndex);
        if(filter->getEnabled() == true)
        {
          m_PipelineModel->setData(childIndex, static_cast<int>(PipelineItem::WidgetState::Ready), PipelineModel::WidgetStateRole);
        }
      }
    }

    //  QSharedPointer<ProgressDialog> progressDialog(new ProgressDialog());
    //  progressDialog->setWindowTitle("Pipeline Preflighting");
    //  QString msg = QString("Please wait for %1 filters to preflight...").arg(pipeline->getFilterContainer().count());
    //  progressDialog->setLabelText(msg);
    //  progressDialog->show();
    //  progressDialog->raise();
    //  progressDialog->activateWindow();

    // Preflight the pipeline
    int err = pipeline->preflightPipeline();
    if(err < 0)
    {
      // FIXME: Implement error handling.
    }

    int count = pipeline->getFilterContainer().size();
    // Now that the preflight has been executed loop through the filters and check their error condition and set the
    // outline on the filter widget if there were errors or warnings
    for(qint32 i = 0; i < count; ++i)
    {
      QModelIndex childIndex = m_PipelineModel->index(i, PipelineItem::Contents, pipelineRootIndex);
      if(childIndex.isValid())
      {
        AbstractFilter::Pointer filter = m_PipelineModel->filter(childIndex);
        if(filter->getWarningCondition() < 0)
        {
          m_PipelineModel->setData(childIndex, static_cast<int>(PipelineItem::ErrorState::Warning), PipelineModel::ErrorStateRole);
        }
        if(filter->getErrorCondition() < 0)
        {
          m_PipelineModel->setData(childIndex, static_cast<int>(PipelineItem::ErrorState::Error), PipelineModel::ErrorStateRole);
        }
      }
    }

    emit preflightFinished(pipeline, err);
    updateFilterInputWidgetIndices(pipelineRootIndex);

    pipeline->clearMessageReceivers();
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewController::setFiltersEnabled(QModelIndexList indexes)
{
  int count = indexes.size();
  PipelineModel* model = getPipelineModel();
  QModelIndexList pipelineRootIndexes;
  for(int i = 0; i < count; i++)
  {
    QModelIndex index = indexes[i];
    AbstractFilter::Pointer filter = model->filter(index);
    if(m_ActionEnableFilter->isChecked() == true)
    {
      filter->setEnabled(true);
      model->setData(index, static_cast<int>(PipelineItem::WidgetState::Ready), PipelineModel::WidgetStateRole);
    }
    else
    {
      filter->setEnabled(false);
      model->setData(index, static_cast<int>(PipelineItem::WidgetState::Disabled), PipelineModel::WidgetStateRole);
    }

    if (pipelineRootIndexes.contains(index.parent()) == false)
    {
      pipelineRootIndexes.push_back(index.parent());
    }
  }

  for (int i = 0; i < pipelineRootIndexes.size(); i++)
  {
    preflightPipeline(pipelineRootIndexes[i]);
  }

  emit filterEnabledStateChanged();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewController::clearPipeline(const QModelIndex &pipelineRootIndex)
{
  if (m_PipelineModel)
  {
    std::vector<AbstractFilter::Pointer> filters;
    for(int i = 0; i < m_PipelineModel->rowCount(pipelineRootIndex); i++)
    {
      QModelIndex filterIndex = m_PipelineModel->index(i, PipelineItem::Contents, pipelineRootIndex);
      filters.push_back(m_PipelineModel->filter(filterIndex));
    }

    removeFilters(filters, pipelineRootIndex, "Clear");
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewController::updateFilterInputWidgetIndices(const QModelIndex &pipelineRootIndex)
{
  if (m_PipelineModel)
  {
    int rowCount = m_PipelineModel->rowCount(pipelineRootIndex);
    int col = PipelineModel::ItemTypeRole;

    for(int row = 0; row < rowCount; row++)
    {
      QModelIndex index = m_PipelineModel->index(row, col, pipelineRootIndex);
      if(false == index.isValid())
      {
        return;
      }

      // Update the FilterInputWidget based on the pipeline index
      AbstractFilter::Pointer filter = m_PipelineModel->filter(index);
      FilterInputWidget* fip = m_PipelineModel->filterInputWidget(index);
      if(filter && fip)
      {
        fip->setFilterIndex(QString::number(filter->getPipelineIndex() + 1));
      }
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int PipelineViewController::openPipeline(const QString& filePath, QModelIndex &pipelineRootIndex, int insertIndex)
{
  if (m_PipelineModel)
  {
    QFileInfo fi(filePath);
    if(fi.exists() == false)
    {
      QMessageBox::warning(nullptr, QString::fromLatin1("Pipeline Read Error"), QString::fromLatin1("There was an error opening the specified pipeline file. The pipeline file does not exist."));
      return -1;
    }

    QString ext = fi.suffix();
    QString name = fi.fileName();
    QString baseName = fi.baseName();

    if (insertIndex < 0)
    {
      insertIndex = m_PipelineModel->rowCount(pipelineRootIndex);
    }

    if(ext == "dream3d")
    {
      QtSFileDragMessageBox* msgBox = new QtSFileDragMessageBox();
      msgBox->exec();
      msgBox->deleteLater();

      if(msgBox->didPressOkBtn() == true)
      {
        if(msgBox->isExtractPipelineBtnChecked() == false)
        {
          DataContainerReader::Pointer reader = DataContainerReader::New();
          reader->setInputFile(filePath);

          addFilter(reader, pipelineRootIndex, insertIndex);
          return true;
        }
      }
    }

    // Read the pipeline from the file
    FilterPipeline::Pointer pipeline = readPipelineFromFile(filePath);

    // Check that a valid extension was read...
    if(pipeline == FilterPipeline::NullPointer())
    {
      emit statusMessage(tr("The pipeline was not read correctly from file '%1'. '%2' is an unsupported file extension.").arg(name).arg(ext));
      emit stdOutMessage(tr("The pipeline was not read correctly from file '%1'. '%2' is an unsupported file extension.").arg(name).arg(ext));
      return -1;
    }

    // Notify user of successful read
    emit statusMessage(tr("Opened \"%1\" Pipeline").arg(baseName));
    emit stdOutMessage(tr("Opened \"%1\" Pipeline").arg(baseName));

    // Populate the pipeline view
    FilterPipeline::Pointer insertPipeline = m_PipelineModel->tempPipeline(pipelineRootIndex);
    if (insertPipeline.get() == nullptr)
    {
      addPipeline(pipeline, -1, filePath);
      pipelineRootIndex = m_PipelineModel->index(m_PipelineModel->rowCount() - 1, PipelineItem::Contents);
    }
    else
    {
      QList<AbstractFilter::Pointer> pipelineFilters = pipeline->getFilterContainer();
      std::vector<AbstractFilter::Pointer> filters;
      for(int i = 0; i < pipelineFilters.size(); i++)
      {
        filters.push_back(pipelineFilters[i]);
      }

      addFilters(filters, pipelineRootIndex, insertIndex);
    }

    emit pipelineFilePathUpdated(filePath);

    return 0;
  }

  return -1;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FilterPipeline::Pointer PipelineViewController::readPipelineFromFile(const QString& filePath)
{
  if (m_PipelineModel)
  {
    QFileInfo fi(filePath);
    QString ext = fi.suffix();

    FilterPipeline::Pointer pipeline;
    if(ext == "dream3d")
    {
      H5FilterParametersReader::Pointer dream3dReader = H5FilterParametersReader::New();
      pipeline = dream3dReader->readPipelineFromFile(filePath);
    }
    else if(ext == "json")
    {
      JsonFilterParametersReader::Pointer jsonReader = JsonFilterParametersReader::New();
      pipeline = jsonReader->readPipelineFromFile(filePath);
    }
    else
    {
      pipeline = FilterPipeline::NullPointer();
    }

    return pipeline;
  }

  return FilterPipeline::NullPointer();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewController::executePipeline(const QModelIndex &pipelineRootIndex)
{
  if (m_PipelineModel)
  {
    // Clear out the Issues Table
    emit clearIssuesTriggered();

    FilterPipeline::Pointer pipeline = m_PipelineModel->tempPipeline(pipelineRootIndex);

    for(int i = 0; i < m_PipelineMessageObservers.size(); i++)
    {
      pipeline->addMessageReceiver(m_PipelineMessageObservers[i]);
    }

    pipeline->addMessageReceiver(m_PipelineModel->pipelineOutputTextEdit(pipelineRootIndex));
    pipeline->addMessageReceiver(m_PipelineModel->pipelineMessageObserver(pipelineRootIndex));

    // Give the pipeline one last chance to preflight and get all the latest values from the GUI
    int err = pipeline->preflightPipeline(true);
    if(err < 0)
    {
      pipeline = FilterPipeline::NullPointer();
      emit displayIssuesTriggered();
      return;
    }

    // Save the preferences file NOW in case something happens
    emit writeSIMPLViewSettingsTriggered();

    setPipelineToReadyState(pipelineRootIndex);

    // Start the Pipeline
    PipelineExecutionController::Pointer executionController = PipelineExecutionController::New();
    executionController->setPipeline(pipeline);

    connect(executionController.get(), &PipelineExecutionController::finished, [=] {
      pipelineExecutionFinished(executionController->getPipeline(), pipelineRootIndex);
    });

    m_RunningPipelines.insert(pipelineRootIndex, executionController);
    executionController->execute();

    m_ActionClearPipeline->setDisabled(true);

    emit pipelineStarted(pipelineRootIndex);

    setPipelineToRunningState(pipelineRootIndex);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewController::cancelPipeline(const QModelIndex &pipelineRootIndex)
{
  if (m_PipelineModel)
  {
    emit pipelineCanceling(pipelineRootIndex);

    PipelineExecutionController::Pointer executionController = m_RunningPipelines[pipelineRootIndex];
    FilterPipeline::Pointer pipeline = executionController->getPipeline();

    pipeline->cancelPipeline();
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewController::pipelineExecutionFinished(FilterPipeline::Pointer pipeline, const QModelIndex &pipelineRootIndex)
{
  if (m_PipelineModel)
  {
    // Put back the DataContainerArray for each filter at the conclusion of running
    // the pipeline. this keeps the data browser current and up to date.
    FilterPipeline::FilterContainerType filters = pipeline->getFilterContainer();
    PipelineExecutionController::Pointer executionController = m_RunningPipelines[pipelineRootIndex];
    QVector<DataContainerArray::Pointer> preflightDataContainerArrays = executionController->getPreflightDataContainerArrays();
    for(FilterPipeline::FilterContainerType::size_type i = 0; i < filters.size(); i++)
    {
      filters[i]->setDataContainerArray(preflightDataContainerArrays[i]);
    }

    setPipelineToStoppedState(pipelineRootIndex);

    pipeline->clearMessageReceivers();

    m_RunningPipelines.remove(pipelineRootIndex);

    emit displayIssuesTriggered();

    if (m_PipelineModel)
    {
      m_ActionClearPipeline->setEnabled(m_PipelineModel->rowCount() > 0);
    }
    else
    {
      m_ActionClearPipeline->setDisabled(true);
    }
  }

  if(pipeline->getCancel() == true)
  {
    emit pipelineCanceled(pipelineRootIndex);
  }
  else
  {
    emit pipelineFinished(pipelineRootIndex);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewController::setPipelineToReadyState(const QModelIndex &pipelineRootIndex)
{
  if (m_PipelineModel)
  {
    for(int i = 0; i < m_PipelineModel->rowCount(pipelineRootIndex); i++)
    {
      QModelIndex index = m_PipelineModel->index(i, PipelineItem::Contents, pipelineRootIndex);

      // Do not set state to Completed if the filter is disabled
      PipelineItem::WidgetState wState = static_cast<PipelineItem::WidgetState>(m_PipelineModel->data(index, PipelineModel::WidgetStateRole).toInt());
      if(wState != PipelineItem::WidgetState::Disabled)
      {
        m_PipelineModel->setData(index, static_cast<int>(PipelineItem::WidgetState::Ready), PipelineModel::WidgetStateRole);
      }
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewController::setPipelineToRunningState(const QModelIndex &pipelineRootIndex)
{
  if (m_PipelineModel)
  {
    for(int i = 0; i < m_PipelineModel->rowCount(pipelineRootIndex); i++)
    {
      QModelIndex index = m_PipelineModel->index(i, PipelineItem::Contents, pipelineRootIndex);
      m_PipelineModel->setData(index, static_cast<int>(PipelineItem::PipelineState::Running), PipelineModel::PipelineStateRole);
      FilterInputWidget* inputWidget = m_PipelineModel->filterInputWidget(index);
      inputWidget->toRunningState();
    }

    m_ActionUndo->setDisabled(true);
    m_ActionRedo->setDisabled(true);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewController::setPipelineToStoppedState(const QModelIndex &pipelineRootIndex)
{
  if (m_PipelineModel)
  {
    for(int i = 0; i < m_PipelineModel->rowCount(pipelineRootIndex); i++)
    {
      QModelIndex index = m_PipelineModel->index(i, PipelineItem::Contents, pipelineRootIndex);
      m_PipelineModel->setData(index, static_cast<int>(PipelineItem::PipelineState::Stopped), PipelineModel::PipelineStateRole);
      FilterInputWidget* inputWidget = m_PipelineModel->filterInputWidget(index);
      AbstractFilter::Pointer filter = m_PipelineModel->filter(index);
      inputWidget->toIdleState();

      if(filter->getEnabled() == true)
      {
        // Do not set state to Completed if the filter is disabled
        PipelineItem::WidgetState wState = static_cast<PipelineItem::WidgetState>(m_PipelineModel->data(index, PipelineModel::WidgetStateRole).toInt());
        if(wState != PipelineItem::WidgetState::Disabled)
        {
          m_PipelineModel->setData(index, static_cast<int>(PipelineItem::WidgetState::Ready), PipelineModel::WidgetStateRole);
        }
      }
    }

    m_ActionUndo->setEnabled(true);
    m_ActionRedo->setEnabled(true);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int PipelineViewController::writePipeline(const QModelIndex &pipelineRootIndex, const QString& outputPath)
{
  if (m_PipelineModel)
  {
    QFileInfo fi(outputPath);
    QString ext = fi.completeSuffix();

    // If the filePath already exists - delete it so that we get a clean write to the file
    if(fi.exists() == true && (ext == "dream3d" || ext == "json"))
    {
      QFile f(outputPath);
      if(f.remove() == false)
      {
        QMessageBox::warning(nullptr, QString::fromLatin1("Pipeline Write Error"), QString::fromLatin1("There was an error removing the existing pipeline file. The pipeline was NOT saved."));
        return -1;
      }
    }

    // Create a Pipeline Object and fill it with the filters from this View
    FilterPipeline::Pointer pipeline = m_PipelineModel->tempPipeline(pipelineRootIndex);

    int err = 0;
    if(ext == "dream3d")
    {
      QList<IObserver*> observers;
      for(int i = 0; i < m_PipelineMessageObservers.size(); i++)
      {
        observers.push_back(reinterpret_cast<IObserver*>(m_PipelineMessageObservers[i]));
      }

      H5FilterParametersWriter::Pointer dream3dWriter = H5FilterParametersWriter::New();
      err = dream3dWriter->writePipelineToFile(pipeline, fi.absoluteFilePath(), fi.fileName(), observers);
    }
    else if(ext == "json")
    {
      QList<IObserver*> observers;
      for(int i = 0; i < m_PipelineMessageObservers.size(); i++)
      {
        observers.push_back(reinterpret_cast<IObserver*>(m_PipelineMessageObservers[i]));
      }

      JsonFilterParametersWriter::Pointer jsonWriter = JsonFilterParametersWriter::New();
      jsonWriter->writePipelineToFile(pipeline, fi.absoluteFilePath(), fi.fileName(), observers);
    }
    else
    {
      emit statusMessage(tr("The pipeline was not written to file '%1'. '%2' is an unsupported file extension.").arg(fi.fileName()).arg(ext));
      return -1;
    }

    if(err < 0)
    {
      emit statusMessage(tr("There was an error while saving the pipeline to file '%1'.").arg(fi.fileName()));
      return -1;
    }
    else
    {
      emit statusMessage(tr("The pipeline has been saved successfully to '%1'.").arg(fi.fileName()));
    }

    return 0;
  }

  return -1;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewController::setPipelineModel(PipelineModel* model)
{
  if (m_PipelineModel)
  {
    disconnect(m_PipelineModel, &PipelineModel::preflightTriggered, this, &PipelineViewController::preflightPipeline);

    disconnect(m_PipelineModel, &PipelineModel::rowsInserted, nullptr, nullptr);

    disconnect(m_PipelineModel, &PipelineModel::rowsRemoved, nullptr, nullptr);

    disconnect(m_PipelineModel, &PipelineModel::rowsMoved, nullptr, nullptr);
  }

  m_PipelineModel = model;

  connect(m_PipelineModel, &PipelineModel::preflightTriggered, this, &PipelineViewController::preflightPipeline);

  connect(m_PipelineModel, &PipelineModel::rowsInserted, [=] { m_ActionClearPipeline->setEnabled(true); });

  connect(m_PipelineModel, &PipelineModel::rowsRemoved, [=] { m_ActionClearPipeline->setEnabled(m_PipelineModel->rowCount() > 0); });

  connect(m_PipelineModel, &PipelineModel::rowsMoved, [=] { m_ActionClearPipeline->setEnabled(m_PipelineModel->rowCount() > 0); });

  m_ActionClearPipeline->setEnabled(m_PipelineModel->rowCount() > 0);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewController::addUndoCommand(QUndoCommand* cmd)
{
  m_UndoStack->push(cmd);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewController::addUndoCommandMacro(std::vector<QUndoCommand*> cmds, const QString &cmdText)
{
  m_UndoStack->beginMacro(cmdText);
  for (int i = 0; i < cmds.size(); i++)
  {
    m_UndoStack->push(cmds[i]);
  }
  m_UndoStack->endMacro();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewController::setupUndoStack()
{
  m_UndoStack = QSharedPointer<QUndoStack>(new QUndoStack());
  m_UndoStack->setUndoLimit(10);

  m_ActionUndo = m_UndoStack->createUndoAction(m_UndoStack.data());
  m_ActionRedo = m_UndoStack->createRedoAction(m_UndoStack.data());
  m_ActionUndo->setShortcut(QKeySequence::Undo);
  m_ActionRedo->setShortcut(QKeySequence::Redo);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewController::undo()
{
  m_UndoStack->undo();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewController::redo()
{
  m_UndoStack->redo();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewController::getFilterItemContextMenu(QMenu &menu, const QModelIndex& index)
{
  QModelIndexList selectedIndexes = m_PipelineView->getSelectedRows();
  qSort(selectedIndexes);

  menu.addAction(m_ActionCut);
  menu.addAction(m_ActionCopy);
  menu.addSeparator();

  QAction* actionPasteAbove = new QAction("Paste Above");
  QAction* actionPasteBelow = new QAction("Paste Below");

  QJsonArray pipelinesArray = getPipelinesArrayFromClipboard();
  actionPasteAbove->setEnabled(pipelinesArray.size() > 0);
  actionPasteBelow->setEnabled(pipelinesArray.size() > 0);

  QObject::connect(actionPasteAbove, &QAction::triggered, [=] { handleActionPasteAbove(pipelinesArray, index); });

  QObject::connect(actionPasteBelow, &QAction::triggered, [=] { handleActionPasteBelow(pipelinesArray, index); });

  menu.addAction(actionPasteAbove);
  menu.addAction(actionPasteBelow);
  menu.addSeparator();

  int count = selectedIndexes.size();
  bool widgetEnabled = true;

  for(int i = 0; i < count && widgetEnabled; i++)
  {
    AbstractFilter::Pointer filter = m_PipelineModel->filter(selectedIndexes[i]);
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

    AbstractFilter::Pointer filter = m_PipelineModel->filter(index);
    if(filter != nullptr)
    {
      widgetEnabled = filter->getEnabled();
    }

    QObject::disconnect(m_ActionEnableFilter, &QAction::toggled, nullptr, nullptr);
    QObject::connect(m_ActionEnableFilter, &QAction::toggled, [=] { setFiltersEnabled(toggledIndices); });
  }
  else
  {
    QObject::disconnect(m_ActionEnableFilter, &QAction::toggled, nullptr, nullptr);
    QObject::connect(m_ActionEnableFilter, &QAction::toggled, [=] { setFiltersEnabled(selectedIndexes); });
  }

  m_ActionEnableFilter->setChecked(widgetEnabled);
  m_ActionEnableFilter->setEnabled(true);
//    actionEnableFilter->setDisabled(getPipelineIsRunning());
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
      AbstractFilter::Pointer filter = m_PipelineModel->filter(index);
      removeFilter(filter, index.parent());
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
        AbstractFilter::Pointer filter = m_PipelineModel->filter(persistentList[i]);
        filters.push_back(filter);
      }

      removeFilters(filters, index.parent());
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
  addErrorHandlingContextMenu(menu);
  menu.addSeparator();

  QAction* actionLaunchHelp = new QAction("Filter Help");
  QObject::connect(actionLaunchHelp, &QAction::triggered, [=] {
    AbstractFilter::Pointer filter = m_PipelineModel->filter(index);
    if(filter != nullptr)
    {
      // Launch the help for this filter
      QString className = filter->getNameOfClass();

      DocRequestManager* docRequester = DocRequestManager::Instance();
      docRequester->requestFilterDocs(className);
    }
  });

  menu.addAction(actionLaunchHelp);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewController::handleActionPasteAbove(QJsonArray pipelinesArray, const QModelIndex& index)
{
  std::vector<AbstractFilter::Pointer> filters;
  for (int i = 0; i < pipelinesArray.size(); i++)
  {
    FilterPipeline::Pointer pipeline = getPipelineFromClipboard(i);
    FilterPipeline::FilterContainerType container = pipeline->getFilterContainer();
    for (int j = 0; j < container.size(); j++)
    {
      filters.push_back(container[j]);
    }
  }

  pasteFilters(filters, index.parent(), index.row());
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewController::handleActionPasteBelow(QJsonArray pipelinesArray, const QModelIndex &index)
{
  std::vector<AbstractFilter::Pointer> filters;
  for (int i = 0; i < pipelinesArray.size(); i++)
  {
    FilterPipeline::Pointer pipeline = getPipelineFromClipboard(i);
    FilterPipeline::FilterContainerType container = pipeline->getFilterContainer();
    for (int j = 0; j < container.size(); j++)
    {
      filters.push_back(container[j]);
    }
  }

  pasteFilters(filters, index.parent(), index.row() + 1);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QJsonArray PipelineViewController::getPipelinesArrayFromClipboard()
{
  QClipboard* clipboard = QApplication::clipboard();
  QString jsonString = clipboard->text();
  if (jsonString.isEmpty())
  {
    return QJsonArray();
  }

  QJsonParseError parseError;
  QByteArray byteArray = QByteArray::fromStdString(jsonString.toStdString());
  QJsonDocument doc = QJsonDocument::fromJson(byteArray, &parseError);
  if(parseError.error != QJsonParseError::NoError)
  {
    return QJsonArray();
  }
  QJsonArray pipelinesArray = doc.array();
  return pipelinesArray;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool PipelineViewController::hasActivePipeline()
{
  return m_ActivePipelineIndex.isValid();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QModelIndex PipelineViewController::getActivePipeline() const
{
  return m_ActivePipelineIndex;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewController::updateActivePipeline(const QModelIndex &pipelineIdx)
{
  emit clearIssuesTriggered();

  m_ActivePipelineIndex = pipelineIdx;

  emit activePipelineUpdated(m_ActivePipelineIndex);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewController::clearActivePipeline()
{
  updateActivePipeline(QModelIndex());
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
FilterPipeline::Pointer PipelineViewController::getPipelineFromClipboard(size_t index)
{
  QJsonArray pipelinesArray = getPipelinesArrayFromClipboard();
  if (index > pipelinesArray.size() - 1)
  {
    return FilterPipeline::NullPointer();
  }

  JsonFilterParametersReader::Pointer jsonReader = JsonFilterParametersReader::New();
  QJsonObject obj = pipelinesArray[index].toObject();
  FilterPipeline::Pointer pipeline = jsonReader->readPipelineFromObject(obj);
  return pipeline;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewController::getPipelineItemContextMenu(QMenu &menu, const QModelIndex &index)
{
  menu.addAction("Execute Pipeline", [=] {
    executePipeline(index);
  });

  menu.addSeparator();

  menu.addAction(m_ActionCut);
  menu.addAction(m_ActionCopy);
  menu.addAction(m_ActionPaste);

  menu.addSeparator();

  menu.addAction("Clear Pipeline", [=] {
    m_PipelineView->clearPipeline(index);
  });

  menu.addSeparator();

  menu.addAction("Remove Pipeline", [=] {
    m_PipelineView->removePipeline(index);
  });

  addErrorHandlingContextMenu(menu);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewController::addErrorHandlingContextMenu(QMenu &menu)
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
  connect(showTableErrorAction, &QAction::triggered, [=]() {
    IssuesWidget::SetHideDockSetting(SIMPLView::DockWidgetSettings::HideDockSetting::OnError);
    m_PipelineView->preflightPipeline();
  });
  connect(ignoreTableErrorAction, &QAction::triggered, [=]() {
    IssuesWidget::SetHideDockSetting(SIMPLView::DockWidgetSettings::HideDockSetting::Ignore);
    m_PipelineView->preflightPipeline();
  });

  // Standard Output
  connect(showStdOutErrorAction, &QAction::triggered, [=]() {
    StandardOutputWidget::SetHideDockSetting(SIMPLView::DockWidgetSettings::HideDockSetting::OnError);
    m_PipelineView->preflightPipeline();
  });
  connect(ignoreStdOutErrorAction, &QAction::triggered, [=]() {
    StandardOutputWidget::SetHideDockSetting(SIMPLView::DockWidgetSettings::HideDockSetting::Ignore);
    m_PipelineView->preflightPipeline();
  });

  // Combined
  connect(showCombinedErrorAction, &QAction::triggered, [=]() {
    IssuesWidget::SetHideDockSetting(SIMPLView::DockWidgetSettings::HideDockSetting::OnError);
    StandardOutputWidget::SetHideDockSetting(SIMPLView::DockWidgetSettings::HideDockSetting::OnError);
    m_PipelineView->preflightPipeline();
  });
  connect(ignoreCombinedErrorAction, &QAction::triggered, [=]() {
    IssuesWidget::SetHideDockSetting(SIMPLView::DockWidgetSettings::HideDockSetting::Ignore);
    StandardOutputWidget::SetHideDockSetting(SIMPLView::DockWidgetSettings::HideDockSetting::Ignore);
    m_PipelineView->preflightPipeline();
  });
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewController::getDefaultContextMenu(QMenu &menu)
{
  menu.addAction(m_ActionPaste);
  menu.addSeparator();
  menu.addAction(m_ActionClearPipeline);
}
