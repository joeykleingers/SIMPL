/* ============================================================================
 * Copyright (c) 2009-2019 BlueQuartz Software, LLC
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
 * The code contained herein was partially funded by the following contracts:
 *    United States Air Force Prime Contract FA8650-15-D-5231
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#pragma once

#include <memory>

#include "SIMPLib/Messages/AbstractMessage.h"

class AbstractMessageHandler;

/**
 * @class AbstractProgressMessage AbstractProgressMessage.h SIMPLib/Messages/AbstractProgressMessage.h
 * @brief This class is an abstract progress message superclass.
 */
class SIMPLib_EXPORT AbstractProgressMessage : public AbstractMessage
{
public:
  using Self = AbstractProgressMessage;
  using Pointer = std::shared_ptr<Self>;
  using ConstPointer = std::shared_ptr<const Self>;
  using WeakPointer = std::weak_ptr<Self>;
  using ConstWeakPointer = std::weak_ptr<const Self>;
  static Pointer NullPointer();

  /**
   * @brief Returns the name of the class for AbstractProgressMessage
   */
  QString getNameOfClass() const override;
  /**
   * @brief Returns the name of the class for AbstractProgressMessage
   */
  static QString ClassName();

  ~AbstractProgressMessage() override;

  /**
   * @brief Setter property for ProgressValue
   */
  void setProgressValue(int value);
  /**
   * @brief Getter property for ProgressValue
   * @return Value of ProgressValue
   */
  int getProgressValue() const;

protected:
  AbstractProgressMessage();

  AbstractProgressMessage(const QString& msgText, int progress);

private:
  int m_ProgressValue = {};
};
Q_DECLARE_METATYPE(AbstractProgressMessage::Pointer)
