/* ============================================================================
 * Copyright (c) 2019 BlueQuartz Software, LLC
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

#include <QtCore/QJsonObject>
#include <QtCore/QMetaType>
#include <QtCore/QString>

#include "SIMPLib/FilterParameters/FileListInfo.h"

class SIMPLib_EXPORT MontageFileListInfo : public FileListInfo
{
  // Start Python bindings declarations
  PYB11_BEGIN_BINDINGS(MontageFileListInfo)
  PYB11_CREATION(int32_t int32_t int32_t int32_t)
  PYB11_END_BINDINGS()
  // End Python bindings declarations
public:
  MontageFileListInfo();
  ~MontageFileListInfo() override;

  MontageFileListInfo(int32_t rowStart, int32_t rowEnd, int32_t colStart, int32_t colEnd);

  int32_t RowStart = 0;
  int32_t RowEnd = 2;
  int32_t ColStart = 0;
  int32_t ColEnd = 2;

  /**
   * @brief writeJson
   * @param json
   */
  void writeJson(QJsonObject& json) const override;

  /**
   * @brief readJson
   * @param json
   * @return
   */
  bool readJson(QJsonObject& json) override;

public:
  MontageFileListInfo(const MontageFileListInfo&) = default;            // Copy Constructor Not Implemented
  MontageFileListInfo(MontageFileListInfo&&) = default;                 // Move Constructor Not Implemented
  MontageFileListInfo& operator=(const MontageFileListInfo&) = default; // Copy Assignment Not Implemented
  MontageFileListInfo& operator=(MontageFileListInfo&&) = default;      // Move Assignment Not Implemented
};

Q_DECLARE_METATYPE(MontageFileListInfo)
