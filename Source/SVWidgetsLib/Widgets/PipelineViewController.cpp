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

#include <QtWidgets/QUndoStack>
#include <QtWidgets/QUndoCommand>
#include <QtWidgets/QWidget>
#include <QtWidgets/QMessageBox>

#include "SIMPLib/CoreFilters/DataContainerReader.h"
#include "SIMPLib/Filtering/FilterManager.h"
#include "SIMPLib/FilterParameters/H5FilterParametersReader.h"
#include "SIMPLib/FilterParameters/H5FilterParametersWriter.h"
#include "SIMPLib/FilterParameters/JsonFilterParametersReader.h"
#include "SIMPLib/FilterParameters/JsonFilterParametersWriter.h"

#include "SVWidgetsLib/Widgets/PipelineModel.h"
#include "SVWidgetsLib/Widgets/PipelineItem.h"
#include "SVWidgetsLib/Widgets/FilterInputWidget.h"
#include "SVWidgetsLib/Widgets/util/AddFilterToModelCommand.h"
#include "SVWidgetsLib/Widgets/util/RemoveFilterFromModelCommand.h"
#include "SVWidgetsLib/QtSupport/QtSFileDragMessageBox.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PipelineViewController::PipelineViewController(QObject* parent) :
  QObject(parent)
{
  initialize();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PipelineViewController::PipelineViewController(PipelineModel* model, QObject* parent) :
  QObject(parent)
, m_PipelineModel(model)
{
  initialize();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PipelineViewController::~PipelineViewController() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewController::initialize()
{
  setupUndoStack();
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
void PipelineViewController::addFilterFromClassName(const QString& filterClassName, int insertIndex, const QModelIndex &pipelineRootIndex)
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
        addFilter(filter, insertIndex, pipelineRootIndex);
      }
    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewController::addFilter(AbstractFilter::Pointer filter, int insertIndex, const QModelIndex &pipelineRootIndex)
{
  if (m_PipelineModel)
  {
    AddFilterToModelCommand* cmd = new AddFilterToModelCommand(filter, m_PipelineModel, insertIndex, pipelineRootIndex, "Add");
    addUndoCommand(cmd);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewController::addFilters(std::vector<AbstractFilter::Pointer> filters, int insertIndex, const QModelIndex &pipelineRootIndex)
{
  if (m_PipelineModel)
  {
    AddFilterToModelCommand* cmd = new AddFilterToModelCommand(filters, m_PipelineModel, insertIndex, pipelineRootIndex, "Add");
    addUndoCommand(cmd);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewController::removeFilter(AbstractFilter::Pointer filter, const QModelIndex &pipelineRootIndex)
{
  if (m_PipelineModel)
  {
    RemoveFilterFromModelCommand* cmd = new RemoveFilterFromModelCommand(filter, m_PipelineModel, pipelineRootIndex, "Remove");
    addUndoCommand(cmd);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewController::removeFilters(std::vector<AbstractFilter::Pointer> filters, const QModelIndex &pipelineRootIndex)
{
  if (m_PipelineModel)
  {
    RemoveFilterFromModelCommand* cmd = new RemoveFilterFromModelCommand(filters, m_PipelineModel, pipelineRootIndex, "Remove");
    addUndoCommand(cmd);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewController::addPipeline(FilterPipeline::Pointer pipeline, int insertIndex)
{
  if (m_PipelineModel)
  {
    QList<AbstractFilter::Pointer> pipelineFilters = pipeline->getFilterContainer();
    std::vector<AbstractFilter::Pointer> filters;
    for(int i = 0; i < pipelineFilters.size(); i++)
    {
      filters.push_back(pipelineFilters[i]);
    }

    AddFilterToModelCommand* cmd = new AddFilterToModelCommand(filters, m_PipelineModel, insertIndex, pipelineRootIndex, "Add");
    addUndoCommand(cmd);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewController::removePipeline(FilterPipeline::Pointer pipeline)
{
  if (m_PipelineModel)
  {
    QList<AbstractFilter::Pointer> pipelineFilters = pipeline->getFilterContainer();
    std::vector<AbstractFilter::Pointer> filters;
    for(int i = 0; i < pipelineFilters.size(); i++)
    {
      filters.push_back(pipelineFilters[i]);
    }

    AddFilterToModelCommand* cmd = new AddFilterToModelCommand(filters, m_PipelineModel, insertIndex, pipelineRootIndex, "Add");
    addUndoCommand(cmd);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewController::cutFilter(AbstractFilter::Pointer filter, const QModelIndex &pipelineRootIndex)
{
  if (m_PipelineModel)
  {
    RemoveFilterFromModelCommand* cmd = new RemoveFilterFromModelCommand(filter, m_PipelineModel, pipelineRootIndex, "Cut");
    addUndoCommand(cmd);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewController::cutFilters(std::vector<AbstractFilter::Pointer> filters, const QModelIndex &pipelineRootIndex)
{
  if (m_PipelineModel)
  {
    RemoveFilterFromModelCommand* cmd = new RemoveFilterFromModelCommand(filters, m_PipelineModel, pipelineRootIndex, "Cut");
    addUndoCommand(cmd);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewController::pasteFilters(int insertIndex, const QModelIndex &pipelineRootIndex)
{
  if (m_PipelineModel)
  {
    QClipboard* clipboard = QApplication::clipboard();
    QString jsonString = clipboard->text();

    JsonFilterParametersReader::Pointer jsonReader = JsonFilterParametersReader::New();
    FilterPipeline::Pointer pipeline = jsonReader->readPipelineFromString(jsonString);
    FilterPipeline::FilterContainerType container = pipeline->getFilterContainer();

    std::vector<AbstractFilter::Pointer> filters;
    for(int i = 0; i < container.size(); i++)
    {
      filters.push_back(container[i]);
    }

    AddFilterToModelCommand* addCmd = new AddFilterToModelCommand(filters, m_PipelineModel, insertIndex, pipelineRootIndex, "Paste");
    addUndoCommand(addCmd);
  }
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

    emit clearIssuesTriggered();

    // Create a Pipeline Object and fill it with the filters from this View
    FilterPipeline::Pointer pipeline = getFilterPipeline(pipelineRootIndex);

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
  }
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

    RemoveFilterFromModelCommand* removeCmd = new RemoveFilterFromModelCommand(filters, m_PipelineModel, pipelineRootIndex, "Clear");
    addUndoCommand(removeCmd);
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
FilterPipeline::Pointer PipelineViewController::getFilterPipeline(const QModelIndex &pipelineRootIndex)
{
  if (m_PipelineModel)
  {
    // Create a Pipeline Object and fill it with the filters from this View
    FilterPipeline::Pointer pipeline = FilterPipeline::New();

    qint32 count = m_PipelineModel->rowCount(pipelineRootIndex);
    for(qint32 i = 0; i < count; ++i)
    {
      QModelIndex childIndex = m_PipelineModel->index(i, PipelineItem::Contents, pipelineRootIndex);

      PipelineItem::WidgetState wState = static_cast<PipelineItem::WidgetState>(m_PipelineModel->data(childIndex, PipelineModel::WidgetStateRole).toInt());
      if(childIndex.isValid() && wState != PipelineItem::WidgetState::Disabled)
      {
        AbstractFilter::Pointer filter = m_PipelineModel->filter(childIndex);
        pipeline->pushBack(filter);
      }
    }
    for(int i = 0; i < m_PipelineMessageObservers.size(); i++)
    {
      pipeline->addMessageReceiver(m_PipelineMessageObservers[i]);
    }
    return pipeline;
  }

  return FilterPipeline::NullPointer();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
int PipelineViewController::openPipeline(const QString& filePath, int insertIndex, const QModelIndex &pipelineRootIndex)
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

          addFilter(reader, insertIndex);
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

    QList<AbstractFilter::Pointer> pipelineFilters = pipeline->getFilterContainer();
    std::vector<AbstractFilter::Pointer> filters;
    for(int i = 0; i < pipelineFilters.size(); i++)
    {
      filters.push_back(pipelineFilters[i]);
    }

    // Populate the pipeline view
    addFilters(filters, insertIndex, pipelineRootIndex);

    emit pipelineFilePathUpdated(filePath);
    emit pipelineDataChanged(pipelineRootIndex);

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

    // Create a FilterPipeline Object
    //  m_PipelineInFlight = getCopyOfFilterPipeline();
    FilterPipeline::Pointer pipeline = getFilterPipeline(pipelineRootIndex);

    emit stdOutMessage("<b>Preflight Pipeline.....</b>");
    // Give the pipeline one last chance to preflight and get all the latest values from the GUI
    int err = pipeline->preflightPipeline();
    if(err < 0)
    {
      pipeline = FilterPipeline::NullPointer();
      emit displayIssuesTriggered();
      return;
    }
    emit stdOutMessage("    Preflight Results: 0 Errors");

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

    emit pipelineStarted(pipelineRootIndex);

    setPipelineToRunningState(pipelineRootIndex);
    emit stdOutMessage("");
    emit stdOutMessage("<b>*************** PIPELINE STARTED ***************</b>");
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineViewController::cancelPipeline(const QModelIndex &pipelineRootIndex)
{
  if (m_PipelineModel)
  {
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
    if(pipeline->getCancel() == true)
    {
      emit stdOutMessage("<b>*************** PIPELINE CANCELED ***************</b>");
    }
    else
    {
      emit stdOutMessage("<b>*************** PIPELINE FINISHED ***************</b>");
    }
    emit stdOutMessage("");

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

    m_RunningPipelines.remove(pipelineRootIndex);

    emit displayIssuesTriggered();
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
    for(int i = 0; i < m_PipelineModel->rowCount(); i++)
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
    FilterPipeline::Pointer pipeline = getFilterPipeline(pipelineRootIndex);

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
  }

  connect(model, &PipelineModel::preflightTriggered, this, &PipelineViewController::preflightPipeline);

  m_PipelineModel = model;
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
