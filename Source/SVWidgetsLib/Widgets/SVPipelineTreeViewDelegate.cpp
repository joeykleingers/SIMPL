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
#include "SVWidgetsLib/Widgets/SVPipelineTreeViewDelegate.h"
#include "SVWidgetsLib/Widgets/PipelineModel.h"
#include "SVWidgetsLib/Widgets/SVPipelineTreeView.h"
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
SVPipelineTreeViewDelegate::SVPipelineTreeViewDelegate(SVPipelineTreeView* view)
  : QStyledItemDelegate(nullptr)
  , m_View(view)
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
SVPipelineTreeViewDelegate::~SVPipelineTreeViewDelegate() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void SVPipelineTreeViewDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
  painter->save();

  QStyleOptionViewItem opt = option;

  PipelineModel* model = m_View->getPipelineModel();
  if (static_cast<PipelineItem::ItemType>(model->data(index, PipelineModel::Roles::ItemTypeRole).toInt()) == PipelineItem::ItemType::PipelineRoot)
  {
    if (model->getActivePipeline() == index)
    {
      QFont font = opt.font;
      font.setBold(true);
      opt.font = font;
    }
  }

  QStyledItemDelegate::paint(painter, opt, index);

  painter->restore();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString SVPipelineTreeViewDelegate::getFilterIndexString(const QModelIndex &index) const
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
const PipelineModel* SVPipelineTreeViewDelegate::getPipelineModel(const QModelIndex &index) const
{
  return dynamic_cast<const PipelineModel*>(index.model());
}
