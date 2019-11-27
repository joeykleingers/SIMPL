/* ============================================================================
* Copyright (c) 2009-2016 BlueQuartz Software, LLC
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
* The code contained herein was partially funded by the followig contracts:
*    United States Air Force Prime Contract FA8650-07-D-5800
*    United States Air Force Prime Contract FA8650-10-D-5210
*    United States Prime Contract Navy N00173-07-C-2068
*
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */


#pragma once

#include <memory>

#include "SIMPLib/SIMPLib.h"
#include "SIMPLib/Filtering/AbstractFilter.h"

class IDataArray;
using IDataArrayWkPtrType = std::weak_ptr<IDataArray>;

class IDataArray;

/**
 * @brief The ReplaceValueInArray class. See [Filter documentation](@ref replacevalueinarray) for details.
 */
class SIMPLib_EXPORT ReplaceValueInArray : public AbstractFilter
{
    Q_OBJECT

#ifdef SIMPL_ENABLE_PYTHON
    PYB11_CREATE_BINDINGS(ReplaceValueInArray SUPERCLASS AbstractFilter)
    PYB11_SHARED_POINTERS(ReplaceValueInArray)
    PYB11_FILTER_NEW_MACRO(ReplaceValueInArray)
    PYB11_FILTER_PARAMETER(DataArrayPath, SelectedArray)
    PYB11_FILTER_PARAMETER(double, RemoveValue)
    PYB11_FILTER_PARAMETER(double, ReplaceValue)
    PYB11_PROPERTY(DataArrayPath SelectedArray READ getSelectedArray WRITE setSelectedArray)
    PYB11_PROPERTY(double RemoveValue READ getRemoveValue WRITE setRemoveValue)
    PYB11_PROPERTY(double ReplaceValue READ getReplaceValue WRITE setReplaceValue)
#endif

  public:
    using Self = ReplaceValueInArray;
    using Pointer = std::shared_ptr<Self>;
    using ConstPointer = std::shared_ptr<const Self>;
    using WeakPointer = std::weak_ptr<Self>;
    using ConstWeakPointer = std::weak_ptr<const Self>;
    static Pointer NullPointer();

    static std::shared_ptr<ReplaceValueInArray> New();

    /**
     * @brief Returns the name of the class for ReplaceValueInArray
     */
    QString getNameOfClass() const override;
    /**
     * @brief Returns the name of the class for ReplaceValueInArray
     */
    static QString ClassName();

    ~ReplaceValueInArray() override;

    /**
     * @brief Setter property for SelectedArray
     */
    void setSelectedArray(const DataArrayPath& value);
    /**
     * @brief Getter property for SelectedArray
     * @return Value of SelectedArray
     */
    DataArrayPath getSelectedArray() const;

    Q_PROPERTY(DataArrayPath SelectedArray READ getSelectedArray WRITE setSelectedArray)

    /**
     * @brief Setter property for RemoveValue
     */
    void setRemoveValue(double value);
    /**
     * @brief Getter property for RemoveValue
     * @return Value of RemoveValue
     */
    double getRemoveValue() const;

    Q_PROPERTY(double RemoveValue READ getRemoveValue WRITE setRemoveValue)

    /**
     * @brief Setter property for ReplaceValue
     */
    void setReplaceValue(double value);
    /**
     * @brief Getter property for ReplaceValue
     * @return Value of ReplaceValue
     */
    double getReplaceValue() const;

    Q_PROPERTY(double ReplaceValue READ getReplaceValue WRITE setReplaceValue)

    /**
     * @brief getCompiledLibraryName Reimplemented from @see AbstractFilter class
     */
    QString getCompiledLibraryName() const override;

    /**
     * @brief getBrandingString Returns the branding string for the filter, which is a tag
     * used to denote the filter's association with specific plugins
     * @return Branding string
     */
    QString getBrandingString() const override;

    /**
     * @brief getFilterVersion Returns a version string for this filter. Default
     * value is an empty string.
     * @return
     */
    QString getFilterVersion() const override;

    /**
     * @brief newFilterInstance Reimplemented from @see AbstractFilter class
     */
    AbstractFilter::Pointer newFilterInstance(bool copyFilterParameters) const override;

    /**
     * @brief getGroupName Reimplemented from @see AbstractFilter class
     */
    QString getGroupName() const override;

    /**
     * @brief getSubGroupName Reimplemented from @see AbstractFilter class
     */
    QString getSubGroupName() const override;

    /**
     * @brief getUuid Return the unique identifier for this filter.
     * @return A QUuid object.
     */
    QUuid getUuid() const override;

    /**
     * @brief getHumanLabel Reimplemented from @see AbstractFilter class
     */
    QString getHumanLabel() const override;

    /**
     * @brief setupFilterParameters Reimplemented from @see AbstractFilter class
     */
    void setupFilterParameters() override;

    /**
     * @brief readFilterParameters Reimplemented from @see AbstractFilter class
     */
    void readFilterParameters(AbstractFilterParametersReader* reader, int index) override;

    /**
     * @brief execute Reimplemented from @see AbstractFilter class
     */
    void execute() override;

    /**
    * @brief preflight Reimplemented from @see AbstractFilter class
    */
    void preflight() override;

  signals:
    /**
     * @brief updateFilterParameters Emitted when the Filter requests all the latest Filter parameters
     * be pushed from a user-facing control (such as a widget)
     * @param filter Filter instance pointer
     */
    void updateFilterParameters(AbstractFilter* filter);

    /**
     * @brief parametersChanged Emitted when any Filter parameter is changed internally
     */
    void parametersChanged();

    /**
     * @brief preflightAboutToExecute Emitted just before calling dataCheck()
     */
    void preflightAboutToExecute();

    /**
     * @brief preflightExecuted Emitted just after calling dataCheck()
     */
    void preflightExecuted();

  protected:
    ReplaceValueInArray();
    /**
     * @brief dataCheck Checks for the appropriate parameter values and availability of arrays
     */
    void dataCheck();

    /**
     * @brief Initializes all the private instance variables.
     */
    void initialize();


  private:
    IDataArrayWkPtrType m_ArrayPtr;

    DataArrayPath m_SelectedArray = {};
    double m_RemoveValue = {};
    double m_ReplaceValue = {};

  public:
    ReplaceValueInArray(const ReplaceValueInArray&) = delete; // Copy Constructor Not Implemented
    ReplaceValueInArray(ReplaceValueInArray&&) = delete;      // Move Constructor Not Implemented
    ReplaceValueInArray& operator=(const ReplaceValueInArray&) = delete; // Copy Assignment Not Implemented
    ReplaceValueInArray& operator=(ReplaceValueInArray&&) = delete;      // Move Assignment Not Implemented
};

