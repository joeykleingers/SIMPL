/*******************************************************************************
Copyright (C) 2017 Milo Solutions
Contact: https://www.milosolutions.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*******************************************************************************/

#include "RESTClient_UI.h"

#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QMimeDatabase>
#include <QtCore/QMimeType>

#include <QtWidgets/QCompleter>
#include <QtWidgets/QFileSystemModel>
#include <QtWidgets/QFileDialog>

#include <QtNetwork/QNetworkReply>
#include <QtNetwork/QHttpMultiPart>

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
RESTClient_UI::RESTClient_UI(QWidget* parent)
: QMainWindow(parent)
, m_Ui(new Ui::RESTClient_UI)
{
  m_Ui->setupUi(this);

  setupGui();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
RESTClient_UI::~RESTClient_UI()
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RESTClient_UI::setupGui()
{
  // Create the network access manager
  m_Connection = QSharedPointer<QNetworkAccessManager>(new QNetworkAccessManager());

  // Add file system completer to the pipeline file path line edit
  QCompleter *completer = new QCompleter(this);
  completer->setModel(new QFileSystemModel(completer));
  m_Ui->pipelineFilePathLE->setCompleter(completer);

  // Validate the port number field
  QIntValidator* intValidator = new QIntValidator(this);
  m_Ui->portNumberLE->setValidator(intValidator);

  // Validate the IP address field
  QRegExp rx("\\b(?:(?:25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])\\.){3}(?:25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])\\b");
  QRegExpValidator* regExpValidator = new QRegExpValidator(rx, this);
  m_Ui->serverIPAddressLE->setValidator(regExpValidator);

  m_Ui->removeInputFilesBtn->setDisabled(true);

  connectSignalsSlots();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RESTClient_UI::connectSignalsSlots()
{
  // Connection that allows the user to select a pipeline file using the Select button
  connect(m_Ui->selectPipelineFilePathBtn, &QPushButton::clicked, [=] {
    QString filePath = selectFilePath("Select Pipeline File", "Pipeline File", "json");
    if (!filePath.isEmpty())
    {
      m_Ui->pipelineFilePathLE->setText(filePath);
    }
  });

  // Connection that allows the user to add input files to the input files list widget
  connect(m_Ui->addInputFilesBtn, &QPushButton::clicked, [=] {
    QStringList filePaths = selectFilePaths("Select Input Files");
    if (!filePaths.isEmpty())
    {
      m_Ui->inputFilesListWidget->addItems(filePaths);
    }
  });

  // Connection that allows the user to remove input files from the input files list widget
  connect(m_Ui->removeInputFilesBtn, &QPushButton::clicked, [=] {
    QModelIndexList selectedRows = m_Ui->inputFilesListWidget->selectionModel()->selectedRows();
    QPersistentModelIndexList persistentSelections = toPersistentIndexList(selectedRows);
    for (int i = 0; i < persistentSelections.size(); i++)
    {
      QPersistentModelIndex rowIndex = persistentSelections[i];
      QListWidgetItem* item = m_Ui->inputFilesListWidget->takeItem(rowIndex.row());
      delete item;
    }
  });

  // Connection that enables the "remove input files" button only when there are one or more files selected
  connect(m_Ui->inputFilesListWidget->selectionModel(), &QItemSelectionModel::selectionChanged, [=] {
    QModelIndexList selectedRows = m_Ui->inputFilesListWidget->selectionModel()->selectedRows();
    m_Ui->removeInputFilesBtn->setEnabled(!selectedRows.isEmpty());
  });

  // Connection that sends the REST request
  connect(m_Ui->startBtn, &QPushButton::clicked, this, &RESTClient_UI::sendRequest);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString RESTClient_UI::selectFilePath(const QString &title, const QString &fileType, const QString &extension, const QString &currentDir)
{
  QString s = tr("All Files(*.*)");
  if (extension.isEmpty() == false && fileType.isEmpty() == false)
  {
    s = s.prepend(tr("%1 Files (*.%2);;").arg(fileType).arg(extension));
  }

  QString filePath = QFileDialog::getOpenFileName(this, title, currentDir, s);
  if(filePath.isEmpty())
  {
    return QString();
  }

  filePath = QDir::toNativeSeparators(filePath);
  return filePath;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QStringList RESTClient_UI::selectFilePaths(const QString &title, const QString &fileType, const QString &extension, const QString &currentDir)
{
  QString s = tr("All Files(*.*)");
  if (extension.isEmpty() == false && fileType.isEmpty() == false)
  {
    s = s.prepend(tr("%1 Files (*.%2);;").arg(fileType).arg(extension));
  }

  QStringList filePaths = QFileDialog::getOpenFileNames(this, title, currentDir, s);
  if(filePaths.isEmpty())
  {
    return QStringList();
  }

  for (int i = 0; i < filePaths.size(); i++)
  {
    filePaths[i] = QDir::toNativeSeparators(filePaths[i]);
  }

  return filePaths;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QSharedPointer<QNetworkReply> RESTClient_UI::sendExecutePipelineRequest()
{
  int portNumber = m_Ui->portNumberLE->text().toInt();
  QString serverIPAddress = m_Ui->serverIPAddressLE->text();

  QUrl url;
  url.setScheme("http");
  url.setHost(serverIPAddress);
  url.setPort(portNumber);
  url.setPath("/api/v1/ExecutePipeline");

  QStringList filePathList;
  int count = m_Ui->inputFilesListWidget->count();
  for (int i = 0; i < count; i++)
  {
    QListWidgetItem* item = m_Ui->inputFilesListWidget->item(i);
    filePathList.push_back(item->text());
  }

  QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

  QString pipelineFilePath = m_Ui->pipelineFilePathLE->text();

  QFile file(pipelineFilePath);
  if (file.open(QIODevice::ReadOnly))
  {
    QTextStream in(&file);
    QString jsonString = in.readAll();
    file.close();
    QByteArray jsonByteArray = QByteArray::fromStdString(jsonString.toStdString());

    QHttpPart jsonPart;
    jsonPart.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    jsonPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"json\""));

    jsonPart.setBody(jsonByteArray);

    multiPart->append(jsonPart);
  }

  {
    QHttpPart pipelineReplacementLookupPart;
    pipelineReplacementLookupPart.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    pipelineReplacementLookupPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"PipelineMetadata\""));

    QJsonObject rootObj;

    {
      QJsonObject filterMetadataObj;

      QJsonObject inputFileObj;
      inputFileObj["IO_Type"] = "Input";
      filterMetadataObj["InputPath"] = inputFileObj;

      QJsonObject outputFileObj;
      outputFileObj["IO_Type"] = "Output";
      filterMetadataObj["OutputFile"] = outputFileObj;

      rootObj["0"] = filterMetadataObj;
    }

    QJsonDocument doc(rootObj);

    pipelineReplacementLookupPart.setBody(doc.toJson());

    multiPart->append(pipelineReplacementLookupPart);
  }

  return sendMultipartRequest(url, multiPart);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RESTClient_UI::sendRequest()
{
  int portNumber = m_Ui->portNumberLE->text().toInt();
  QString serverIPAddress = m_Ui->serverIPAddressLE->text();

  QUrl url;
  url.setScheme("http");
  url.setHost(serverIPAddress);
  url.setPort(portNumber);

  switch(static_cast<EndpointChoice>(m_Ui->endpointCB->currentIndex()))
  {
    case EndpointChoice::ExecutePipeline:
    {
      QSharedPointer<QNetworkReply> reply = sendExecutePipelineRequest();
    }
    case EndpointChoice::ListFilterParameters:
    {

    }
    case EndpointChoice::ListLoadedPlugins:
    {

    }
    case EndpointChoice::ListLoadedFilters:
    {

    }
    case EndpointChoice::NumberOfFilters:
    {

    }
    case EndpointChoice::ListPluginInformation:
    {

    }
    case EndpointChoice::PreflightPipeline:
    {

    }
    case EndpointChoice::SIMPLibVersion:
    {

    }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QSharedPointer<QNetworkReply> RESTClient_UI::sendDataRequest(QUrl url, QString contentType, QByteArray data)
{
  QNetworkRequest netRequest(url);
  netRequest.setHeader(QNetworkRequest::ContentTypeHeader, contentType);

  QEventLoop waitLoop;
  QSharedPointer<QNetworkReply> reply = QSharedPointer<QNetworkReply>(m_Connection->post(netRequest, data));
  QObject::connect(reply.data(), SIGNAL(finished()), &waitLoop, SLOT(quit()));
  QObject::connect(reply.data(), SIGNAL(uploadProgress(qint64, qint64)), this, SLOT(processUploadProgress(qint64, qint64)));
  waitLoop.exec();

  return reply;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QSharedPointer<QNetworkReply> RESTClient_UI::sendMultipartRequest(QUrl url, QHttpMultiPart* multiPart)
{
  QNetworkRequest netRequest(url);

  QEventLoop waitLoop;
  QSharedPointer<QNetworkReply> reply = QSharedPointer<QNetworkReply>(m_Connection->post(netRequest, multiPart));
  multiPart->setParent(reply.data());
  QObject::connect(reply.data(), SIGNAL(finished()), &waitLoop, SLOT(quit()));
  QObject::connect(reply.data(), SIGNAL(uploadProgress(qint64, qint64)), this, SLOT(processUploadProgress(qint64, qint64)));
  waitLoop.exec();

  return reply;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void RESTClient_UI::processUploadProgress(qint64 bytesSent, qint64 bytesTotal)
{
  double percent;
  if(bytesTotal == 0)
  {
    percent = 0;
  }
  else
  {
    percent = (static_cast<double>(bytesSent) / bytesTotal) * 100;
  }

  m_Ui->stdOutputTextEdit->append(tr("Upload Progress: %1/%2 - %3%").arg(bytesSent).arg(bytesTotal).arg(percent));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
RESTClient_UI::QPersistentModelIndexList RESTClient_UI::toPersistentIndexList(QModelIndexList indexList)
{
  RESTClient_UI::QPersistentModelIndexList persistentList;
  for (int i = 0; i < indexList.size(); i++)
  {
    persistentList.push_back(QPersistentModelIndex(indexList[i]));
  }

  return persistentList;
}
