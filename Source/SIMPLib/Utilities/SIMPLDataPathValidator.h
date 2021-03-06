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

#include <QtCore/QObject>
#include <QtCore/QString>

#include "SIMPLib/SIMPLib.h"

/**
 * @brief The SIMPLDataPathValidator class
 */
class SIMPLib_EXPORT SIMPLDataPathValidator : public QObject
{
  Q_OBJECT

public:
  ~SIMPLDataPathValidator() override;

  static SIMPLDataPathValidator* Instance();

  /**
   * @brief Getter property for SIMPLDataDirectory
   * @return Value of SIMPLDataDirectory
   */
  QString getSIMPLDataDirectory() const;

  /**
   * @brief setSIMPLDataDirectory
   * @param path
   */
  void setSIMPLDataDirectory(const QString& path);

  /**
   * @brief This function creates an absolute path based on the location of the application executing the code BUT more
   * assumes that this code is running from a packaged application in which case there are more assumptions made about
   * where certain directories. We may try to navigate up/down directories based on platform.
   * @param relativePath
   * @return
   */
  QString convertToAbsolutePath(const QString& relativePath);

protected:
  SIMPLDataPathValidator();

Q_SIGNALS:
  void dataDirectoryChanged(const QString& path);

private:
  static SIMPLDataPathValidator* m_Self;

  QString m_SIMPLDataDirectory;

public:
  SIMPLDataPathValidator(const SIMPLDataPathValidator&) = delete;            // Copy Constructor Not Implemented
  SIMPLDataPathValidator(SIMPLDataPathValidator&&) = delete;                 // Move Constructor Not Implemented
  SIMPLDataPathValidator& operator=(const SIMPLDataPathValidator&) = delete; // Copy Assignment Not Implemented
  SIMPLDataPathValidator& operator=(SIMPLDataPathValidator&&) = delete;      // Move Assignment Not Implemented
};
