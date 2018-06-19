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
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#include "PipelineListWidget.h"

#include "SVWidgetsLib/Widgets/PipelineModel.h"
#include "SVWidgetsLib/Widgets/SVStyle.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PipelineListWidget::PipelineListWidget(QWidget* parent)
  : QFrame(parent)
{
  setupUi(this);

  setupGui();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PipelineListWidget::~PipelineListWidget()
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineListWidget::setupGui()
{
  startPipelineBtn->setStyleSheet(getStartPipelineIdleStyle());
  startPipelineBtn->setDisabled(true);

  pipelineView->addPipelineMessageObserver(this);

  connect(pipelineView, &SVPipelineView::preflightFinished, this, &PipelineListWidget::preflightFinished);

  connect(pipelineView, &SVPipelineView::pipelineFinished, [=] { updateStartButtonState(StartButtonState::Idle); });

  connect(this, &PipelineListWidget::pipelineCanceled, [=] { pipelineView->cancelPipeline(); });
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineListWidget::processPipelineMessage(const PipelineMessage& msg)
{
  if(msg.getType() == PipelineMessage::MessageType::ProgressValue)
  {
    float progValue = static_cast<float>(msg.getProgressValue()) / 100;
    setProgressValue(progValue);
  }
  else if(msg.getType() == PipelineMessage::MessageType::StatusMessageAndProgressValue)
  {
    float progValue = static_cast<float>(msg.getProgressValue()) / 100;
    setProgressValue(progValue);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineListWidget::on_startPipelineBtn_clicked()
{
  PipelineModel* model = pipelineView->getPipelineModel();
  if(startPipelineBtn->text().compare("Cancel Pipeline") == 0)
  {
    emit pipelineCanceled(QModelIndex());

    updateStartButtonState(StartButtonState::Canceling);
    return;
  }
  else if(startPipelineBtn->text().compare("Canceling...") == 0)
  {
    return;
  }

  if (model->rowCount() > 0)
  {
    pipelineView->executePipeline(QModelIndex());
    updateStartButtonState(StartButtonState::Running);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineListWidget::preflightFinished(FilterPipeline::Pointer pipeline, int err)
{  
  if (err >= 0 && pipeline->getFilterContainer().size() > 0)
  {
    startPipelineBtn->setEnabled(true);
  }
  else
  {
    startPipelineBtn->setDisabled(true);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineListWidget::updateStartButtonState(StartButtonState state)
{
  if (state == StartButtonState::Idle)
  {
    startPipelineBtn->setText("Start Pipeline");
    startPipelineBtn->setIcon(QIcon(":/SIMPL/icons/images/media_play_white.png"));
    startPipelineBtn->setStyleSheet(getStartPipelineIdleStyle());
  }
  else if (state == StartButtonState::Canceling)
  {
    startPipelineBtn->setText("Canceling...");
    startPipelineBtn->setIcon(QIcon(":/SIMPL/icons/images/media_stop_white.png"));
  }
  else
  {
    startPipelineBtn->setText("Cancel Pipeline");
    startPipelineBtn->setIcon(QIcon(":/SIMPL/icons/images/media_stop_white.png"));
    startPipelineBtn->setStyleSheet(getStartPipelineInProgressStyle(0.0f));
  }
  update();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineListWidget::setProgressValue(float percent)
{
  startPipelineBtn->setStyleSheet(getStartPipelineInProgressStyle(percent));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString PipelineListWidget::getStartPipelineIdleStyle()
{
  QFont font = SVStyle::Instance()->GetHumanLabelFont();
  QString fontString;
  QTextStream in(&fontString);
  in << "font: " << font.weight() << " " <<
#if defined(Q_OS_MAC)
      font.pointSize() - 2
#elif defined(Q_OS_WIN)
      font.pointSize() - 3
#else
      font.pointSize()
#endif
     << "pt \"" << font.family() << "\";"
     << "font-weight: bold;";

  QString cssStr;
  QTextStream ss(&cssStr);
  ss << "QPushButton:enabled { ";
  ss << "background-color: rgb(0, 118, 6);\n";
  ss << "color: white;\n";
  ss << "border-radius: 0px;\n";
  ss << fontString;
  ss << "}\n\n";

  ss << "QPushButton:disabled { ";
  ss << "background-color: rgb(150, 150, 150);\n";
  ss << "color: rgb(190, 190, 190);\n";
  ss << "border-radius: 0px;\n";
  ss << fontString;
  ss << "}";

  return cssStr;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString PipelineListWidget::getStartPipelineInProgressStyle(float percent)
{
  QFont font = SVStyle::Instance()->GetHumanLabelFont();
  QString fontString;
  QTextStream in(&fontString);
  in << "font: " << font.weight() << " " <<
#if defined(Q_OS_MAC)
      font.pointSize() - 2
#elif defined(Q_OS_WIN)
      font.pointSize() - 3
#else
      font.pointSize()
#endif
     << "pt \"" << font.family() << "\";"
     << "font-weight: bold;";

  QString cssStr;
  QTextStream ss(&cssStr);
  ss << "QPushButton { ";
  ss << QString("background: qlineargradient(x1:0, y1:0.5, x2:1, y2:0.5, stop:0 rgb(29, 168, 29), stop:%1 rgb(29, 168, 29), stop:%2 rgb(0, 118, 6), stop:1 rgb(0, 118, 6));\n")
            .arg(percent)
            .arg(percent + 0.001);
  ss << "color: white;\n";
  ss << "border-radius: 0px;\n";
  ss << fontString;

  ss << "}";

  return cssStr;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
SVPipelineView* PipelineListWidget::getPipelineView()
{
  return pipelineView;
}
