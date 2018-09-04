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

#include <QtWidgets/QStyle>
#include <QtWidgets/QStylePainter>

#include "SVWidgetsLib/Widgets/PipelineItem.h"
#include "SVWidgetsLib/Widgets/SVPipelineListViewDelegate.h"
#include "SVWidgetsLib/Widgets/PipelineModel.h"
#include "SVWidgetsLib/Widgets/SVPipelineListView.h"
#include "SVWidgetsLib/Widgets/SVStyle.h"

namespace {
  const int k_ButtonSize = 24;
  const int k_TextMargin = 4;

  const QColor k_DropIndicatorWidgetBackgroundColor = QColor(150, 150, 150);
  const QColor k_DropIndicatorIndexBackgroundColor = QColor(48, 48, 48);
  const QColor k_DropIndicatorLabelColor = QColor(242, 242, 242);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
SVPipelineListViewDelegate::SVPipelineListViewDelegate(SVPipelineListView* view)
  : QStyledItemDelegate(nullptr)
  , m_View(view)
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
SVPipelineListViewDelegate::~SVPipelineListViewDelegate() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineListViewDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  PipelineModel* model = m_View->getPipelineModel();
  if (model->isPipelineRootItem(index))
  {
    return;
  }

  painter->setRenderHint(QPainter::Antialiasing);

  const int textMargin = 6;
  const int indexBoxWidth = 35;

  QFont font = SVStyle::Instance()->GetHumanLabelFont();

#if defined(Q_OS_MAC)
  font.setPointSize(font.pointSize() - 4);
#elif defined(Q_OS_WIN)
  font.setPointSize(font.pointSize() - 3);
#else
  font.setPointSize(font.pointSize() - 1);
#endif

  QFontMetrics fontMetrics(font);
  int fontHeight = fontMetrics.height();
  int indexFontWidth = fontMetrics.width(QString::number(model->getMaxFilterCount()));

  QRect filterIndexRect = option.rect;
  int fontMargin = ((filterIndexRect.height() - fontHeight) / 2) - 1;
  filterIndexRect.setWidth(2 * textMargin + indexFontWidth);

  QRect filterTitleRect(2 * textMargin + indexFontWidth, option.rect.y(), option.rect.width() - (2 * textMargin + indexFontWidth), option.rect.height()); // +4? without it it does not paint to the edge

  QRect dropIndicatorIndexRect = filterIndexRect;
  QRect dropIndicatorTitleRect = filterTitleRect;

  if (m_DropRow >= 0 && index.row() >= m_DropRow)
  {
    filterIndexRect.setY(dropIndicatorIndexRect.y() + dropIndicatorIndexRect.height() + (m_View->spacing() * 2));
    filterTitleRect.setY(dropIndicatorTitleRect.y() + dropIndicatorTitleRect.height() + (m_View->spacing() * 2));

    filterIndexRect.setHeight(dropIndicatorIndexRect.height());
    filterTitleRect.setHeight(dropIndicatorIndexRect.height());
  }
  else if (m_DropRow == model->rowCount(index.parent()) && index.row() == m_DropRow - 1)
  {
    // We need to draw the drop indicator as the last item in the list
    dropIndicatorIndexRect.setY(filterIndexRect.y() + filterIndexRect.height() + (m_View->spacing() * 2));
    dropIndicatorTitleRect.setY(filterTitleRect.y() + filterTitleRect.height() + (m_View->spacing() * 2));

    dropIndicatorIndexRect.setHeight(filterIndexRect.height());
    dropIndicatorTitleRect.setHeight(filterIndexRect.height());
  }

  PipelineItem::WidgetState wState = static_cast<PipelineItem::WidgetState>(model->data(index, PipelineModel::WidgetStateRole).toInt());
  PipelineItem::PipelineState pState = static_cast<PipelineItem::PipelineState>(model->data(index, PipelineModel::PipelineStateRole).toInt());
  PipelineItem::ErrorState eState = static_cast<PipelineItem::ErrorState>(model->data(index, PipelineModel::ErrorStateRole).toInt());

  PipelineItem::ItemType itemType = static_cast<PipelineItem::ItemType>(model->data(index, PipelineModel::ItemTypeRole).toInt());

  AbstractFilter::Pointer filter = model->filter(index);

  QColor grpColor;
  if (filter.get() != nullptr)
  {
    QString grpName = filter->getGroupName();
    grpColor = SVStyle::Instance()->GetFilterBackgroundColor();
  }

  QColor widgetBackgroundColor;
  QColor labelColor = SVStyle::Instance()->GetFilterFontColor();
  QColor indexBackgroundColor;
  QColor bgColor = grpColor;
  #if 1
    QColor selectedBgColor = SVStyle::Instance()->GetFilterSelectionColor();
  #else
  QColor selectedBgColor = m_View->palette().color(QPalette::Highlight);
  #endif
  QColor disabledBgColor = QColor(124, 124, 124);

  bool drawButtons = false;
  if(option.state & QStyle::State_Selected)
  {
    bgColor = selectedBgColor;
  }

  if((option.state & QStyle::State_MouseOver))
  {
    if((option.state & QStyle::State_Selected) == false)
    {
      QColor hoveredColor = bgColor;
      hoveredColor.setRedF((hoveredColor.redF() * 1.10 > 1.0) ? 1.0 : hoveredColor.redF() * 1.10);
      hoveredColor.setGreenF((hoveredColor.greenF() * 1.10 > 1.0) ? 1.0 : hoveredColor.greenF() * 1.10);
      hoveredColor.setBlueF((hoveredColor.blueF() * 1.10 > 1.0) ? 1.0 : hoveredColor.blueF() * 1.10);
      bgColor = hoveredColor;
    }

    if (m_DropRow < 0)
    {
      drawButtons = true;
    }
  }

  switch(wState)
  {
    case PipelineItem::WidgetState::Ready:
      widgetBackgroundColor = bgColor;
      //labelColor = QColor(240, 240, 240);
      indexBackgroundColor = QColor(48, 48, 48);
      break;
    case PipelineItem::WidgetState::Executing:
      widgetBackgroundColor = QColor(130, 130, 130);
      //labelColor = QColor(20, 20, 20);
      indexBackgroundColor = QColor(6, 140, 190);
      break;
    case PipelineItem::WidgetState::Completed:
      widgetBackgroundColor = bgColor.name();
      //labelColor = QColor(240, 240, 240);
      indexBackgroundColor = QColor(6, 118, 6);
      break;
    case PipelineItem::WidgetState::Disabled:
      bgColor = disabledBgColor;
      widgetBackgroundColor = disabledBgColor.name();
      //labelColor = QColor(240, 240, 240);
      indexBackgroundColor = QColor(96, 96, 96);
      break;
  }
  QColor selectedColor = QColor::fromHsv(bgColor.hue(), 100, 120);

  // Do not change the background color if the widget is disabled.
  if(wState != PipelineItem::WidgetState::Disabled)
  {
    switch(pState)
    {
      case PipelineItem::PipelineState::Running:
        widgetBackgroundColor = selectedColor.name();
        labelColor = QColor(20, 20, 20);
        break;
      case PipelineItem::PipelineState::Stopped:
        widgetBackgroundColor = bgColor.name();
        //labelColor = QColor(240, 240, 240);
        break;
      case PipelineItem::PipelineState::Paused:
        widgetBackgroundColor = QColor(160, 160, 160);
        labelColor = QColor(0, 0, 0);
        break;
    }

    switch(eState)
    {
      case PipelineItem::ErrorState::Ok:

        break;
      case PipelineItem::ErrorState::Error:
        indexBackgroundColor = QColor(179, 2, 5);
        break;
      case PipelineItem::ErrorState::Warning:
        indexBackgroundColor = QColor(232, 189, 0);
        break;
    }
  }

  if(option.state & QStyle::State_Selected)
  {
    labelColor = m_View->palette().color(QPalette::HighlightedText);
  }

  QColor indexFontColor(242, 242, 242);

  if(m_View->getPipelineState() == SVPipelineListView::PipelineViewState::Running)
  {
    drawButtons = false;
  }

  painter->setFont(font);

  // back fill with RED so we know if we missed something
  // painter->fillRect(rect(), QColor(255, 0, 0));

  // Draw the filter Index area
  painter->fillRect(filterIndexRect, indexBackgroundColor);

  // Draw the filter Title area
  painter->fillRect(filterTitleRect, widgetBackgroundColor);

  // Draw the border that separates the Index area and the Title area
  painter->setPen(QPen(QBrush(QColor(Qt::black)), m_BorderSize));
  painter->drawLine(2 * textMargin + indexFontWidth, filterIndexRect.y() + 1, 2 * textMargin + indexFontWidth, filterIndexRect.y() + filterIndexRect.height() - 0.5);

  if (m_DropRow >= 0 && (index.row() == m_DropRow || (m_DropRow == model->rowCount(index.parent()) && index.row() == m_DropRow - 1)))
  {
    // Draw the drop indicator index area
    painter->fillRect(dropIndicatorIndexRect, k_DropIndicatorIndexBackgroundColor);
    // Draw the drop indicator title area
    painter->fillRect(dropIndicatorTitleRect, k_DropIndicatorWidgetBackgroundColor);
    // Draw the border
    painter->drawLine(2 * textMargin + indexFontWidth, dropIndicatorIndexRect.y() + 1, 2 * textMargin + indexFontWidth, dropIndicatorIndexRect.y() + dropIndicatorIndexRect.height() - 0.5);

    if (fontHeight <= dropIndicatorIndexRect.height())
    {
      painter->setPen(QPen(k_DropIndicatorLabelColor));
      QString number = getFilterIndexString(m_DropRow); // format the index number with a leading zero
      painter->drawText(dropIndicatorIndexRect.x() + textMargin, dropIndicatorIndexRect.y() + fontMargin + fontHeight, number);
    }
  }

  // Draw the Index number
  painter->setPen(QPen(indexFontColor));
  QString number;
  if (m_DropRow >= 0 && index.row() >= m_DropRow)
  {
    number = getFilterIndexString(index.row() + 1); // format the index number with a leading zero
  }
  else
  {
    number = getFilterIndexString(index.row()); // format the index number with a leading zero
  }

  if (fontHeight <= filterIndexRect.height())
  {
    painter->drawText(filterIndexRect.x() + textMargin, filterIndexRect.y() + fontMargin + fontHeight, number);
  }

  // Compute the Width to draw the text based on the visibility of the various buttons
  int fullWidth = option.rect.width() - indexBoxWidth;
  int allowableWidth = fullWidth;

  if (drawButtons == true)
  {
    // Draw the "delete" button
    QRectF deleteBtnRect;
    deleteBtnRect.setX(option.rect.width() - ::k_ButtonSize - ::k_TextMargin);
    deleteBtnRect.setY(filterTitleRect.y() + ( (option.rect.height() / 2) - (::k_ButtonSize / 2) ) );
    deleteBtnRect.setWidth(::k_ButtonSize);
    deleteBtnRect.setHeight(::k_ButtonSize);

    QPoint mousePos = QCursor::pos();
    mousePos = m_View->viewport()->mapFromGlobal(mousePos);

    QPixmap deleteBtnPixmap;
    if(deleteBtnRect.contains(mousePos))
    {
      deleteBtnPixmap = m_View->getDeleteBtnHoveredPixmap(option.state & QStyle::State_Selected);
      if (painter->device()->devicePixelRatio() == 2)
      {
        deleteBtnPixmap = m_View->getHighDPIDeleteBtnHoveredPixmap(option.state & QStyle::State_Selected);
      }
    }
    else
    {
      deleteBtnPixmap = m_View->getDeleteBtnPixmap(option.state & QStyle::State_Selected);
      if (painter->device()->devicePixelRatio() == 2)
      {
        deleteBtnPixmap = m_View->getHighDPIDeleteBtnPixmap(option.state & QStyle::State_Selected);
      }
    }

    painter->drawPixmap(deleteBtnRect.center().x() - (deleteBtnRect.width() / 2), deleteBtnRect.center().y() - (deleteBtnRect.height() / 2 + 1), deleteBtnPixmap);  // y is 1px offset due to how the images were cut

    // Draw the "disable" button
    QRectF disableBtnRect;
    disableBtnRect.setX(deleteBtnRect.x() - ::k_TextMargin - ::k_ButtonSize);
    disableBtnRect.setY(filterTitleRect.y() + ( (filterTitleRect.height() / 2) - (::k_ButtonSize / 2) ) );
    disableBtnRect.setWidth(::k_ButtonSize);
    disableBtnRect.setHeight(::k_ButtonSize);

    QPixmap disableBtnPixmap;
    PipelineItem::WidgetState wState = static_cast<PipelineItem::WidgetState>(model->data(index, PipelineModel::WidgetStateRole).toInt());
    if (wState == PipelineItem::WidgetState::Disabled)
    {
      disableBtnPixmap = m_View->getDisableBtnActivatedPixmap(option.state & QStyle::State_Selected);
      if (painter->device()->devicePixelRatio() == 2)
      {
         disableBtnPixmap = m_View->getHighDPIDisableBtnActivatedPixmap(option.state & QStyle::State_Selected);
      }
    }
    else if(disableBtnRect.contains(mousePos))
    {
      disableBtnPixmap = m_View->getDisableBtnHoveredPixmap(option.state & QStyle::State_Selected);
      if (painter->device()->devicePixelRatio() == 2)
      {
         disableBtnPixmap = m_View->getHighDPIDisableBtnHoveredPixmap(option.state & QStyle::State_Selected);
      }
    }
    else
    {
      disableBtnPixmap = m_View->getDisableBtnPixmap(option.state & QStyle::State_Selected);
      if (painter->device()->devicePixelRatio() == 2)
      {
         disableBtnPixmap = m_View->getHighDPIDisableBtnPixmap(option.state & QStyle::State_Selected);
      }
    }

    allowableWidth -= deleteBtnRect.width();
    allowableWidth -= disableBtnRect.width();

    painter->drawPixmap(disableBtnRect.center().x() - (disableBtnRect.width() / 2), disableBtnRect.center().y() - (disableBtnRect.height() / 2 + 1), disableBtnPixmap);  // y is 1px offset due to how the images were cut
  }

  int humanLabelWidth;
  if (m_DropRow >= 0 && (index.row() == m_DropRow || (m_DropRow == model->rowCount(index.parent()) && index.row() == m_DropRow - 1)))
  {
    humanLabelWidth = fontMetrics.width(m_DropText);
  }
  else
  {
    humanLabelWidth = fontMetrics.width(filter->getHumanLabel());
  }

  // Draw the filter human label
  painter->setPen(QPen(labelColor));
  font.setWeight(QFont::Normal);
  painter->setFont(font);

  // Compute a Fade out of the text if it is too long to fit in the widget
  if(humanLabelWidth > allowableWidth)
  {
    QRect fadedRect = filterTitleRect;
    fadedRect.setWidth(fullWidth);
    if(option.state & QStyle::State_MouseOver)
    {
      fadedRect.setWidth(allowableWidth);
    }

    QLinearGradient gradient(fadedRect.topLeft(), fadedRect.topRight());
    gradient.setColorAt(0.8, labelColor);
    gradient.setColorAt(1.0, QColor(0, 0, 0, 10));

    QPen pen;
    pen.setBrush(QBrush(gradient));
    painter->setPen(pen);
  }

  if (fontHeight <= filterTitleRect.height())
  {
    if (m_DropRow >= 0 && (index.row() == m_DropRow || (m_DropRow == model->rowCount(index.parent()) && index.row() == m_DropRow - 1)))
    {
      painter->drawText(dropIndicatorIndexRect.x() + indexBoxWidth + textMargin, dropIndicatorTitleRect.y() + fontMargin + fontHeight, m_DropText);
    }

    if (itemType == PipelineItem::ItemType::Filter)
    {
      painter->drawText(filterIndexRect.x() + indexBoxWidth + textMargin, filterTitleRect.y() + fontMargin + fontHeight, filter->getHumanLabel());
    }
  }

  QPen pen(QBrush(QColor(Qt::black)), m_BorderSize);
//  QPen pen(QBrush(QColor(48, 48, 48)), m_BorderSize);
  painter->setPen(pen);

  // Draw inside option.rect to avoid painting artifacts
  qreal x = filterIndexRect.x() + (m_BorderSize / 2);
  qreal y = filterIndexRect.y() + (m_BorderSize / 2);
  painter->drawRoundedRect(QRectF(x, y, option.rect.width() - m_BorderSize, filterIndexRect.height() - m_BorderSize), 1, 1);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool SVPipelineListViewDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
  PipelineModel* pipelineModel = dynamic_cast<PipelineModel*>(model);
  if (pipelineModel->isPipelineRootItem(index))
  {
    return QStyledItemDelegate::editorEvent(event, model, option, index);
  }

  QStyleOptionViewItem opt = option;
  if (m_DropRow >= 0 && index.row() >= m_DropRow)
  {
    opt.rect.setY(option.rect.y() + option.rect.height() + m_View->spacing());
  }

  QRect deleteBtnRect;
  deleteBtnRect.setX(option.rect.width() - ::k_ButtonSize - ::k_TextMargin);
  deleteBtnRect.setY(opt.rect.y() + (opt.rect.height()/2 - ::k_ButtonSize/2));
  deleteBtnRect.setWidth(::k_ButtonSize);
  deleteBtnRect.setHeight(::k_ButtonSize);

  QRect disableBtnRect = deleteBtnRect;
  disableBtnRect.setX(disableBtnRect.x() - ::k_TextMargin - ::k_ButtonSize);

  // Looking for click in the delete button area
  QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
  if (mouseEvent != nullptr)
  {
    if(event->type() == QEvent::MouseMove)
    {
      if (deleteBtnRect.contains(mouseEvent->pos()))
      {
        // We are inside the delete button
        if (m_CurrentlyHoveredItem != HoverItem::DeleteButton)
        {
          // We were not inside the delete button before, so set the current hovered item to DeleteButton and schedule a repaint
          m_CurrentlyHoveredItem = HoverItem::DeleteButton;
          return true;
        }
      }
      else if (disableBtnRect.contains(mouseEvent->pos()))
      {
        // We are inside the disable button
        if (m_CurrentlyHoveredItem != HoverItem::DisableButton)
        {
          // We were not inside the disable button before, so set the current hovered item to DisableButton and schedule a repaint
          m_CurrentlyHoveredItem = HoverItem::DisableButton;
          return true;
        }
      }
      else if (m_CurrentlyHoveredItem != HoverItem::Widget)
      {
        // Otherwise, we have to be inside the main widget.
        // We were not inside the main widget before, so set the current hovered item to Widget and schedule a repaint
        m_CurrentlyHoveredItem = HoverItem::Widget;
        return true;
      }
    }

    if(event->type() == QEvent::MouseButtonPress)
    {
      if(deleteBtnRect.contains(mouseEvent->pos()) || disableBtnRect.contains(mouseEvent->pos()))
      {
        m_MousePressIndex = index.row();
        return true;  // don't call the base class, we handled the event here
      }
    }

    if(event->type() == QEvent::MouseButtonRelease)
    {
      if(deleteBtnRect.contains(mouseEvent->pos()) || disableBtnRect.contains(mouseEvent->pos()))
      {
        m_MousePressIndex = -1;
        //qDebug() << "Clicked the Pipeline Filter delete button on: " << index.data(Qt::DisplayRole).toString();

        if(deleteBtnRect.contains(mouseEvent->pos()) && m_View->getPipelineState() != SVPipelineListView::PipelineViewState::Running)
        {
          AbstractFilter::Pointer filter = pipelineModel->filter(index);
          m_View->removeFilter(filter);
          return true;
        }
        else if(disableBtnRect.contains(mouseEvent->pos()) && m_View->getPipelineState() != SVPipelineListView::PipelineViewState::Running)
        {
          AbstractFilter::Pointer filter = pipelineModel->filter(index);
          bool enabled = filter->getEnabled();
          if (enabled)
          {
            filter->setEnabled(false);
            model->setData(index, static_cast<int>(PipelineItem::WidgetState::Disabled), PipelineModel::WidgetStateRole);
          }
          else
          {
            filter->setEnabled(true);
            model->setData(index, static_cast<int>(PipelineItem::WidgetState::Ready), PipelineModel::WidgetStateRole);
          }

          m_View->preflightPipeline(index.parent());
          return true;
        }
      }
    }
  }

  return QStyledItemDelegate::editorEvent(event, model, option, index);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QSize SVPipelineListViewDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  QSize size = QStyledItemDelegate::sizeHint(option, index);
  return QSize(size.width(), 28);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString SVPipelineListViewDelegate::getFilterIndexString(int row) const
{
  PipelineModel* model = m_View->getPipelineModel();
  int numFilters = model->rowCount();
  int i = row + 1;

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
QPixmap SVPipelineListViewDelegate::createPixmap(const QModelIndex &index) const
{
  QRect indexRect = m_View->visualRect(index);

  QPixmap pixmap(indexRect.width(), indexRect.height());
  QPainter painter;

  QStyleOptionViewItem option;
  indexRect.setHeight(indexRect.height() - indexRect.y());
  indexRect.setY(0);
  indexRect.setX(0);
  option.rect = indexRect;
  option.state = QStyle::State_None;

  painter.begin(&pixmap);
  paint(&painter, option, index);
  painter.end();

  return pixmap;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const PipelineModel* SVPipelineListViewDelegate::getPipelineModel(const QModelIndex &index) const
{
  return dynamic_cast<const PipelineModel*>(index.model());
}
