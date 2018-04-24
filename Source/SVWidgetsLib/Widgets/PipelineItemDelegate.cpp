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
#include "SVWidgetsLib/Widgets/PipelineItemDelegate.h"
#include "SVWidgetsLib/Widgets/PipelineModel.h"
#include "SVWidgetsLib/Widgets/SVPipelineView.h"
#include "SVWidgetsLib/QtSupport/QtSStyles.h"

const int BUTTON_SIZE = 24;
const int TEXT_MARGIN = 4;


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PipelineItemDelegate::PipelineItemDelegate(SVPipelineView* view)
  : QStyledItemDelegate(nullptr)
  , m_View(view)
{
  createDisableButton();
  createDeleteButton();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
PipelineItemDelegate::~PipelineItemDelegate() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QSize PipelineItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
  return {option.rect.width(), 36};
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineItemDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  painter->setRenderHint(QPainter::Antialiasing);

  const PipelineModel* model = getPipelineModel(index);

  PipelineItem::WidgetState wState = model->widgetState(index);
  PipelineItem::PipelineState pState = model->pipelineState(index);
  PipelineItem::ErrorState eState = model->errorState(index);

  AbstractFilter::Pointer filter = model->filter(index);
  QString grpName = filter->getGroupName();

  QColor grpColor = QtSStyles::ColorForFilterGroup(grpName);

  QColor widgetBackgroundColor;
  QColor labelColor;
  QColor indexBackgroundColor;
  QColor bgColor = grpColor;
  QColor disabledBgColor = QColor(124, 124, 124);

  bool drawButtons = false;
  if(option.state & QStyle::State_MouseOver)
  {
    QColor hoveredColor = grpColor;
    hoveredColor.setRedF((hoveredColor.redF() * 1.10 > 1.0) ? 1.0 : hoveredColor.redF() * 1.10);
    hoveredColor.setGreenF((hoveredColor.greenF() * 1.10 > 1.0) ? 1.0 : hoveredColor.greenF() * 1.10);
    hoveredColor.setBlueF((hoveredColor.blueF() * 1.10 > 1.0) ? 1.0 : hoveredColor.blueF() * 1.10);
    bgColor = hoveredColor;

    drawButtons = true;
  }
  switch(wState)
  {
    case PipelineItem::WidgetState::Ready:
      widgetBackgroundColor = bgColor;
      labelColor = QColor(190, 190, 190);
      indexBackgroundColor = QColor(48, 48, 48);
      break;
    case PipelineItem::WidgetState::Executing:
      widgetBackgroundColor = QColor(130, 130, 130);
      labelColor = QColor(190, 190, 190);
      indexBackgroundColor = QColor(6, 140, 190);
      break;
    case PipelineItem::WidgetState::Completed:
      widgetBackgroundColor = bgColor.name();
      labelColor = QColor(190, 190, 190);
      indexBackgroundColor = QColor(6, 118, 6);
      break;
    case PipelineItem::WidgetState::Disabled:
      bgColor = disabledBgColor;
      widgetBackgroundColor = disabledBgColor.name();
      labelColor = QColor(190, 190, 190);
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
        labelColor = QColor(190, 190, 190);
        break;
      case PipelineItem::PipelineState::Stopped:
        widgetBackgroundColor = bgColor.name();
        labelColor = QColor(0, 0, 0);
        break;
      case PipelineItem::PipelineState::Paused:
        widgetBackgroundColor = QColor(160, 160, 160);
        labelColor = QColor(0, 0, 0);
        break;
    }
  }

  switch(eState)
  {
    case PipelineItem::ErrorState::Ok:

      break;
    case PipelineItem::ErrorState::Error:
      indexBackgroundColor = QColor(179, 2, 5);
      break;
    case PipelineItem::ErrorState::Warning:
      indexBackgroundColor = QColor(215, 197, 1);
      break;
  }

  QColor indexFontColor(242, 242, 242);
  QFont font = QtSStyles::GetHumanLabelFont();

#if defined(Q_OS_MAC)
  font.setPointSize(font.pointSize() - 4);
#elif defined(Q_OS_WIN)
  font.setPointSize(font.pointSize() - 3);
#else
  font.setPointSize(font.pointSize() - 1);
#endif

  QFontMetrics fontMetrics(font);
  int fontHeight = fontMetrics.height();
  int fontMargin = ((option.rect.height() - fontHeight) / 2) - 1;

  int indexFontWidth = fontMetrics.width(QString::number(model->getMaxFilterCount()));

  painter->setFont(font);

  // back fill with RED so we know if we missed something
  // painter->fillRect(rect(), QColor(255, 0, 0));

  const int textMargin = 6;
  const int indexBoxWidth = 35;

  // Draw the Index area
  QRect rect = option.rect;
  QRect indexRect = option.rect;
  indexRect.setWidth(2 * textMargin + indexFontWidth);
  painter->fillRect(indexRect, indexBackgroundColor);

  // Draw the Title area
  QRect coloredRect(2 * textMargin + indexFontWidth, rect.y(), rect.width() - (2 * textMargin + indexFontWidth), rect.height()); // +4? without it it does not paint to the edge
  painter->fillRect(coloredRect, widgetBackgroundColor);

  // Draw the Index number
  painter->setPen(QPen(indexFontColor));
  QString number = getFilterIndexString(index); // format the index number with a leading zero
  painter->drawText(rect.x() + textMargin, rect.y() + fontMargin + fontHeight, number);

  // Compute the Width to draw the text based on the visibility of the various buttons
  int fullWidth = rect.width() - indexBoxWidth;
  int allowableWidth = fullWidth;

  //  QModelIndex deleteIndex = model->index(index.row(), PipelineItem::DeleteBtn, index.parent());
  //  QWidget* deleteBtn = m_View->indexWidget(deleteIndex);

  //  if(deleteBtn->isVisible())
  //  {
  //    allowableWidth -= deleteBtn->width();
  //  }

  //  QModelIndex disableIndex = model->index(index.row(), PipelineItem::DisableBtn, index.parent());
  //  QWidget* disableBtn = m_View->indexWidget(disableIndex);
  //  if(disableBtn->isVisible())
  //  {
  //    allowableWidth -= disableBtn->width();
  //  }
  // QString elidedHumanLabel = fontMetrics.elidedText(m_FilterHumanLabel, Qt::ElideRight, allowableWidth);
  int humanLabelWidth = fontMetrics.width(filter->getHumanLabel());

  // Draw the filter human label
  painter->setPen(QPen(labelColor));
  font.setWeight(QFont::Normal);
  painter->setFont(font);

  // Compute a Fade out of the text if it is too long to fit in the widget
  if(humanLabelWidth > allowableWidth)
  {
    QRect fadedRect = coloredRect;
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

  painter->drawText(rect.x() + indexBoxWidth + textMargin, rect.y() + fontMargin + fontHeight, filter->getHumanLabel());

  if (drawButtons == true)
  {
    // Draw the "delete" button
    QRectF deleteBtnRect;
    deleteBtnRect.setX(option.rect.width() - BUTTON_SIZE - TEXT_MARGIN);
    deleteBtnRect.setY(option.rect.y() + ( (option.rect.height() / 2) - (BUTTON_SIZE / 2) ) );
    deleteBtnRect.setWidth(BUTTON_SIZE);
    deleteBtnRect.setHeight(BUTTON_SIZE);

    QPoint mousePos = QCursor::pos();
    mousePos = m_View->viewport()->mapFromGlobal(mousePos);

    QPixmap deleteBtnPixmap;
    if(deleteBtnRect.contains(mousePos))
    {
      deleteBtnPixmap = QPixmap(":/trash_hover.png");
    }
    else
    {
      deleteBtnPixmap = QPixmap(":/trash.png");
    }

    painter->drawPixmap(deleteBtnRect.center().x() - (deleteBtnPixmap.width() / 2), deleteBtnRect.center().y() - (deleteBtnPixmap.height() / 2 + 1), deleteBtnPixmap);  // y is 1px offset due to how the images were cut

    // Draw the "disable" button
    QRectF disableBtnRect;
    disableBtnRect.setX(deleteBtnRect.x() - TEXT_MARGIN - BUTTON_SIZE);
    disableBtnRect.setY(option.rect.y() + ( (option.rect.height() / 2) - (BUTTON_SIZE / 2) ) );
    disableBtnRect.setWidth(BUTTON_SIZE);
    disableBtnRect.setHeight(BUTTON_SIZE);

    QPixmap disableBtnPixmap;
    if(disableBtnRect.contains(mousePos))
    {
      if (model->filterEnabled(index) == false)
      {
        disableBtnPixmap = QPixmap(":/ban_red.png");
      }
      else
      {
        disableBtnPixmap = QPixmap(":/ban_hover.png");
      }
    }
    else if (model->filterEnabled(index) == false)
    {
      disableBtnPixmap = QPixmap(":/ban_red.png");
    }
    else
    {
      disableBtnPixmap = QPixmap(":/ban.png");
    }

    painter->drawPixmap(disableBtnRect.center().x() - (disableBtnPixmap.width() / 2), disableBtnRect.center().y() - (disableBtnPixmap.height() / 2 + 1), disableBtnPixmap);  // y is 1px offset due to how the images were cut
  }

  // If the filter is selected, draw a border around it.
  if(option.state & QStyle::State_Selected)
  {
    QColor selectedColor = QColor::fromHsv(bgColor.hue(), 180, 150);
    QPen pen(QBrush(selectedColor), m_BorderThickness);
    painter->setPen(pen);

    // Draw inside option.rect to avoid painting artifacts
    qreal x = option.rect.x() + (m_BorderThickness / 2);
    qreal y = option.rect.y() + (m_BorderThickness / 2);
    painter->drawRoundedRect(QRectF(x, y, option.rect.width() - m_BorderThickness  + 0.5 , option.rect.height() - m_BorderThickness + 0.5), 1, 1);
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool PipelineItemDelegate::editorEvent(QEvent* event, QAbstractItemModel* model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
  PipelineModel* pipelineModel = dynamic_cast<PipelineModel*>(model);

//  // Ignore mouse events if the filter is disabled (pipeline running)
//  if (pipelineModel->filterEnabled(index) == false)
//  {
//    return true; // don't call the base class
//  }

  QRect deleteBtnRect;
  deleteBtnRect.setX(option.rect.width() - BUTTON_SIZE - TEXT_MARGIN);
  deleteBtnRect.setY(option.rect.y() + (option.rect.height()/2 - BUTTON_SIZE/2));
  deleteBtnRect.setWidth(BUTTON_SIZE);
  deleteBtnRect.setHeight(BUTTON_SIZE);

  QRect disableBtnRect = deleteBtnRect;
  disableBtnRect.setX(disableBtnRect.x() - TEXT_MARGIN - BUTTON_SIZE);

  // Looking for click in the delete button area
  QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
  if (mouseEvent != nullptr)
  {
    if(event->type() == QEvent::MouseMove)
    {
      if (deleteBtnRect.contains(mouseEvent->pos()))
      {
        if (m_CurrentlyHoveredItem != HoverItem::DeleteButton)
        {
          m_CurrentlyHoveredItem = HoverItem::DeleteButton;
          return true;
        }
      }
      else if (disableBtnRect.contains(mouseEvent->pos()))
      {
        if (m_CurrentlyHoveredItem != HoverItem::DisableButton)
        {
          m_CurrentlyHoveredItem = HoverItem::DisableButton;
          return true;
        }
      }
      else if (m_CurrentlyHoveredItem != HoverItem::Widget)
      {
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

        if (deleteBtnRect.contains(mouseEvent->pos()))
        {
          AbstractFilter::Pointer filter = pipelineModel->filter(index);
          m_View->removeFilter(filter);
          return true;
        }
        else if (disableBtnRect.contains(mouseEvent->pos()))
        {
          bool enabled = pipelineModel->filterEnabled(index);
          pipelineModel->setFilterEnabled(index, !enabled);
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
QString PipelineItemDelegate::getFilterIndexString(const QModelIndex &index) const
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
void PipelineItemDelegate::createDeleteButton()
{
  m_DeleteBtn = new QPushButton();
  m_DeleteBtn->setMaximumSize(QSize(24, 24));
  m_DeleteBtn->setStyleSheet(QLatin1String("QPushButton#deleteBtn:pressed \n"
                                           "{\n"
                                           "	border: 0px inset black;\n"
                                           "}\n"
                                           "\n"
                                           "QPushButton#deleteBtn:checked \n"
                                           "{\n"
                                           " 	border: 0px inset black;\n"
                                           "}\n"
                                           ""));
  QIcon icon1;
  icon1.addFile(QStringLiteral(":/trash.png"), QSize(), QIcon::Normal, QIcon::Off);
  m_DeleteBtn->setIcon(icon1);
  m_DeleteBtn->setIconSize(QSize(24, 24));
  m_DeleteBtn->setCheckable(true);
  m_DeleteBtn->setChecked(true);
  m_DeleteBtn->setFlat(true);

#ifndef QT_NO_TOOLTIP
  m_DeleteBtn->setToolTip("Click to remove filter from pipeline");
#endif // QT_NO_TOOLTIP
  m_DeleteBtn->setText(QString());
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void PipelineItemDelegate::createDisableButton()
{
  m_DisableBtn = new QPushButton();
  QSizePolicy sizePolicy1(QSizePolicy::Fixed, QSizePolicy::Fixed);
  sizePolicy1.setHorizontalStretch(0);
  sizePolicy1.setVerticalStretch(0);
  sizePolicy1.setHeightForWidth(m_DisableBtn->sizePolicy().hasHeightForWidth());
  m_DisableBtn->setSizePolicy(sizePolicy1);
  m_DisableBtn->setMaximumSize(QSize(24, 24));
  m_DisableBtn->setStyleSheet(QLatin1String("QPushButton#disableBtn:pressed \n"
                                            "{\n"
                                            "	border: 0px inset black;\n"
                                            "}\n"
                                            "\n"
                                            "QPushButton#disableBtn:checked \n"
                                            "{\n"
                                            " 	border: 0px inset black;\n"
                                            "}\n"
                                            ""));
  QIcon icon;
  icon.addFile(QStringLiteral(":/ban.png"), QSize(), QIcon::Normal, QIcon::Off);
  icon.addFile(QStringLiteral(":/ban_red.png"), QSize(), QIcon::Normal, QIcon::On);
  icon.addFile(QStringLiteral(":/ban_red.png"), QSize(), QIcon::Active, QIcon::On);
  icon.addFile(QStringLiteral(":/ban_red.png"), QSize(), QIcon::Selected, QIcon::On);
  m_DisableBtn->setIcon(icon);
  m_DisableBtn->setIconSize(QSize(24, 24));
  m_DisableBtn->setCheckable(true);
  m_DisableBtn->setChecked(false);
  m_DisableBtn->setFlat(true);

#ifndef QT_NO_TOOLTIP
  m_DisableBtn->setToolTip("Click to disable filter.");
#endif // QT_NO_TOOLTIP
  m_DisableBtn->setText(QString());
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
const PipelineModel* PipelineItemDelegate::getPipelineModel(const QModelIndex &index) const
{
  return dynamic_cast<const PipelineModel*>(index.model());
}
