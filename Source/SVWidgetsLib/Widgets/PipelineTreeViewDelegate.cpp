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

#include <QtCore/QFileInfo>

#include <QtWidgets/QLineEdit>

#include <QtGui/QIntValidator>
#include <QtGui/QPainter>
#include <QtGui/QMouseEvent>

#include <QtWidgets/QStylePainter>

#include "SVWidgetsLib/Widgets/PipelineItem.h"
#include "SVWidgetsLib/Widgets/PipelineTreeViewDelegate.h"
#include "SVWidgetsLib/Widgets/PipelineModel.h"
#include "SVWidgetsLib/Widgets/PipelineTreeView.h"
#include "SVWidgetsLib/Widgets/SVStyle.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PipelineTreeViewDelegate::PipelineTreeViewDelegate(PipelineTreeView* view)
  : QStyledItemDelegate(nullptr)
  , m_View(view)
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PipelineTreeViewDelegate::~PipelineTreeViewDelegate() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineTreeViewDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  if (m_View->getActivePipeline() == index)
  {
    QFont font = option.font;
    font.setBold(true);

  }

  QStyledItemDelegate::paint(painter, option, index);

//  PipelineModel* model = m_View->getPipelineModel();

//  painter->setRenderHint(QPainter::Antialiasing);

//  PipelineItem::WidgetState wState = static_cast<PipelineItem::WidgetState>(model->data(index, PipelineModel::WidgetStateRole).toInt());
//  PipelineItem::PipelineState pState = static_cast<PipelineItem::PipelineState>(model->data(index, PipelineModel::PipelineStateRole).toInt());
//  PipelineItem::ErrorState eState = static_cast<PipelineItem::ErrorState>(model->data(index, PipelineModel::ErrorStateRole).toInt());

//  PipelineItem::ItemType itemType = static_cast<PipelineItem::ItemType>(model->data(index, PipelineModel::ItemTypeRole).toInt());

//  AbstractFilter::Pointer filter = model->filter(index);
//  QColor grpColor;

//  if (itemType == PipelineItem::ItemType::Filter)
//  {
//    if (filter.get() == nullptr)
//    {
//      return;
//    }

//    QString grpName = filter->getGroupName();
//    grpColor = SVStyle::Instance()->GetFilterBackgroundColor();
//  }

//  QColor widgetBackgroundColor;
//  QColor labelColor = SVStyle::Instance()->GetFilterFontColor();
//  QColor indexBackgroundColor;
//  QColor bgColor = grpColor;
//  #if 1
//    QColor selectedBgColor = SVStyle::Instance()->GetFilterSelectionColor();
//  #else
//  QColor selectedBgColor = m_View->palette().color(QPalette::Highlight);
//  #endif
//  QColor disabledBgColor = QColor(124, 124, 124);

//  bool drawButtons = false;
//  if(option.state & QStyle::State_Selected)
//  {
//    bgColor = selectedBgColor;
//  }

//  QColor indexFontColor(230, 230, 230);

//  if((option.state & QStyle::State_MouseOver))
//  {
//    if((option.state & QStyle::State_Selected) == false)
//    {
//      QColor hoveredColor = bgColor;
//      hoveredColor.setRedF((hoveredColor.redF() * 1.10 > 1.0) ? 1.0 : hoveredColor.redF() * 1.10);
//      hoveredColor.setGreenF((hoveredColor.greenF() * 1.10 > 1.0) ? 1.0 : hoveredColor.greenF() * 1.10);
//      hoveredColor.setBlueF((hoveredColor.blueF() * 1.10 > 1.0) ? 1.0 : hoveredColor.blueF() * 1.10);
//      bgColor = hoveredColor;
//    }

//    drawButtons = true;
//  }

//  switch(wState)
//  {
//    case PipelineItem::WidgetState::Ready:
//      widgetBackgroundColor = bgColor;
//      //labelColor = QColor(240, 240, 240);
//      indexBackgroundColor = QColor(48, 48, 48);
//      break;
//    case PipelineItem::WidgetState::Executing:
//      widgetBackgroundColor = QColor(130, 130, 130);
//      //labelColor = QColor(20, 20, 20);
//      indexBackgroundColor = QColor(6, 140, 190);
//      break;
//    case PipelineItem::WidgetState::Completed:
//      widgetBackgroundColor = bgColor.name();
//      //labelColor = QColor(240, 240, 240);
//      indexBackgroundColor = QColor(6, 118, 6);
//      break;
//    case PipelineItem::WidgetState::Disabled:
//      bgColor = disabledBgColor;
//      widgetBackgroundColor = disabledBgColor.name();
//      //labelColor = QColor(240, 240, 240);
//      indexBackgroundColor = QColor(96, 96, 96);
//      break;
//  }
//  QColor selectedColor = QColor::fromHsv(bgColor.hue(), 100, 120);

//  // Do not change the background color if the widget is disabled.
//  if(wState != PipelineItem::WidgetState::Disabled)
//  {
//    switch(pState)
//    {
//      case PipelineItem::PipelineState::Running:
//        widgetBackgroundColor = selectedColor.name();
//        labelColor = QColor(20, 20, 20);
//        break;
//      case PipelineItem::PipelineState::Stopped:
//        widgetBackgroundColor = bgColor.name();
//        labelColor = QColor(48, 48, 48);
//        break;
//      case PipelineItem::PipelineState::Paused:
//        widgetBackgroundColor = QColor(160, 160, 160);
//        labelColor = QColor(0, 0, 0);
//        break;
//    }

//    switch(eState)
//    {
//      case PipelineItem::ErrorState::Ok:

//        break;
//      case PipelineItem::ErrorState::Error:
//        indexBackgroundColor = QColor(179, 2, 5);
//        break;
//      case PipelineItem::ErrorState::Warning:
//        indexBackgroundColor = QColor(232, 189, 0);
//        break;
//    }
//  }

//  if(option.state & QStyle::State_Selected)
//  {
//    labelColor = m_View->palette().color(QPalette::HighlightedText);
//  }

//  if(m_View->getPipelineState() == PipelineView::PipelineViewState::Running)
//  {
//    drawButtons = false;
//  }

//  QFont font = SVStyle::Instance()->GetHumanLabelFont();

//#if defined(Q_OS_MAC)
//  font.setPointSize(font.pointSize() - 4);
//#elif defined(Q_OS_WIN)
//  font.setPointSize(font.pointSize() - 3);
//#else
//  font.setPointSize(font.pointSize() - 1);
//#endif

//  QFontMetrics fontMetrics(font);
//  int fontHeight = fontMetrics.height();
//  int fontMargin = ((option.rect.height() - fontHeight) / 2) - 1;

//  int indexFontWidth = fontMetrics.width(QString::number(model->getMaxFilterCount()));

//  painter->setFont(font);

//  // back fill with RED so we know if we missed something
//  // painter->fillRect(rect(), QColor(255, 0, 0));

//  const int textMargin = 6;
//  const int indexBoxWidth = 10;

//  QRect rect = option.rect;

//  if (itemType == PipelineItem::ItemType::Filter)
//  {
//    // Draw the Title area
//    QRect coloredRect(rect.x() + 2 * textMargin + indexFontWidth, rect.y(), rect.width() - (2 * textMargin + indexFontWidth), rect.height()); // +4? without it it does not paint to the edge
////    painter->fillRect(coloredRect, widgetBackgroundColor);

//    QRect indexRect = option.rect;
//    indexRect.setWidth(2 * textMargin + indexFontWidth);

//    // Draw the Index area
////    QPainterPath path;
////    path.addRoundedRect(indexRect, 3, 3);
////    QPen pen(Qt::white, 1);
////    painter->setPen(pen);
////    painter->fillPath(path, indexBackgroundColor);
////    painter->drawPath(path);

//    // Draw the Index number
//    painter->setPen(QPen(QColor(120, 120, 120)));
//    QString number = getFilterIndexString(index); // format the index number with a leading zero
//    if (fontHeight <= indexRect.height())
//    {
//      painter->drawText(rect.x() + textMargin, rect.y() + fontMargin + fontHeight, "|");
//    }
//  }
//  else if (itemType == PipelineItem::ItemType::PipelineRoot)
//  {
////    QPainterPath path;
////    path.addRoundedRect(option.rect, 3, 3);
////    QPen pen(Qt::white, 1);
////    painter->setPen(pen);
////    painter->fillPath(path, indexBackgroundColor);
////    painter->drawPath(path);
//  }

//  // Compute the Width to draw the text based on the visibility of the various buttons
//  int fullWidth = rect.width() - indexBoxWidth;
//  int allowableWidth = fullWidth;

//  int humanLabelWidth = 0;
//  if (itemType == PipelineItem::ItemType::Filter)
//  {
//    humanLabelWidth = fontMetrics.width(filter->getHumanLabel());
//  }
//  else if (itemType == PipelineItem::ItemType::PipelineRoot)
//  {
//    FilterPipeline::Pointer pipeline = model->tempPipeline(index);
//    if (pipeline != nullptr)
//    {
//      humanLabelWidth = fontMetrics.width(pipeline->getName());
//    }
//  }

//  // Draw the filter human label
//  painter->setPen(QPen(labelColor));
//  font.setWeight(QFont::Normal);
//  painter->setFont(font);

//  // Compute a Fade out of the text if it is too long to fit in the widget
//  if(humanLabelWidth > allowableWidth)
//  {
//    QRect fadedRect = rect;
//    fadedRect.setWidth(fullWidth);
//    if(option.state & QStyle::State_MouseOver)
//    {
//      fadedRect.setWidth(allowableWidth);
//    }

//    QLinearGradient gradient(fadedRect.topLeft(), fadedRect.topRight());
//    gradient.setColorAt(0.8, labelColor);
//    gradient.setColorAt(1.0, QColor(0, 0, 0, 10));

//    QPen pen;
//    pen.setBrush(QBrush(gradient));
//    painter->setPen(pen);
//  }

//  if (fontHeight <= rect.height())
//  {
//    if (itemType == PipelineItem::ItemType::Filter)
//    {
//      painter->setPen(QPen(indexBackgroundColor));

////      painter->drawText(rect.x() + textMargin, rect.y() + fontMargin + fontHeight, filter->getHumanLabel());
//      painter->drawText(rect.x() + indexBoxWidth + textMargin, rect.y() + fontMargin + fontHeight, filter->getHumanLabel());
//    }
//    else if (itemType == PipelineItem::ItemType::PipelineRoot)
//    {
//      painter->setPen(QPen(labelColor));

//      if (m_View->getActivePipeline() == index)
//      {
//        font.setWeight(QFont::Bold);
//        painter->setFont(font);
//      }
//      else
//      {
//      }

//      QString pipelineName = "";
//      FilterPipeline::Pointer pipeline = model->tempPipeline(index);
//      if (pipeline != nullptr)
//      {
//        pipelineName = pipeline->getName();
//      }
//      if (model->data(index, PipelineModel::Roles::PipelineModifiedRole).toBool())
//      {
//        pipelineName.append("*");
//      }
//      painter->drawText(rect.x() + textMargin, rect.y() + fontMargin + fontHeight, pipelineName);
//    }
//  }
}

//// -----------------------------------------------------------------------------
////
//// -----------------------------------------------------------------------------
//QSize PipelineTreeViewDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
//{
//  QSize size = QStyledItemDelegate::sizeHint(option, index);
//  return QSize(size.width(), 20);
//}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString PipelineTreeViewDelegate::getFilterIndexString(const QModelIndex &index) const
{
  const PipelineModel* model = getPipelineModel(index);
  int numFilters = model->rowCount();
  int i = index.row() + 1;

  if(numFilters < 10)
  {
    numFilters = 11;
  }
  QString numStr = QString::number(i);

  if(numFilters > 9)
  {
    int mag = 0;
    int max = numFilters;
    while(max > 0)
    {
      mag++;
      max = max / 10;
    }
    numStr = "";             // Clear the string
    QTextStream ss(&numStr); // Create a QTextStream to set up the padding
    ss.setFieldWidth(mag);
    ss.setPadChar('0');
    ss << i;
  }
  QString paddedIndex = numStr;

  return paddedIndex;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const PipelineModel* PipelineTreeViewDelegate::getPipelineModel(const QModelIndex &index) const
{
  return dynamic_cast<const PipelineModel*>(index.model());
}
