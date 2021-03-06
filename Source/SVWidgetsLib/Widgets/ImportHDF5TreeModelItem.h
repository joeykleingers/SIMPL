/* ============================================================================
 * Copyright (c) 2017 BlueQuartz Software, LLC
 * All rights reserved.
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
 * Neither the names of any of the BlueQuartz Software contributors
 * may be used to endorse or promote products derived from this software without
 * specific prior written permission.
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

#pragma once

#include <QtCore/QList>
#include <QtCore/QVariant>
#include <QtGui/QIcon>

#include <hdf5.h>

#include "SVWidgetsLib/SVWidgetsLib.h"

class SVWidgetsLib_EXPORT ImportHDF5TreeModelItem
{
public:
  ImportHDF5TreeModelItem(hid_t fileId, const QString& data, ImportHDF5TreeModelItem* parent = 0);
  ~ImportHDF5TreeModelItem();

  /**
   * @brief Setter property for HasErrors
   */
  void setHasErrors(bool value);
  /**
   * @brief Getter property for HasErrors
   * @return Value of HasErrors
   */
  bool getHasErrors() const;

  void appendChild(ImportHDF5TreeModelItem* child);

  ImportHDF5TreeModelItem* child(int row);
  int childCount();
  int columnCount();
  QVariant data(int column);
  int row();
  ImportHDF5TreeModelItem* parent();

  QString generateHDFPath();

  bool isGroup();
  bool isImage();
  bool isTable();
  bool isString();
  Qt::CheckState getCheckState();

  void setCheckState(Qt::CheckState checkState);

  int numAttributes();
  int numDimensions();

  QIcon icon();

protected:
  void initializeChildItems();
  void initializeChildCount();

private:
  bool m_HasErrors = {};

  QList<ImportHDF5TreeModelItem*> m_ChildItems;
  int m_ChildItemsInitialized = 0;
  int m_ChildCount = -1;
  QVariant m_ItemData;
  ImportHDF5TreeModelItem* m_ParentItem;
  hid_t m_FileId;
  int m_NumAttrs = -1;
  int m_NumDims = -1;
  bool m_IsGroup = false;
  bool m_IsImage = false;
  bool m_IsTable = false;
  bool m_IsString = false;
  std::string m_DataType = "";
  Qt::CheckState m_CheckState = Qt::Unchecked;
};
