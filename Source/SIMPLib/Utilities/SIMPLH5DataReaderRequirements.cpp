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

#include "SIMPLH5DataReaderRequirements.h"

#include "SIMPLib/DataArrays/DataArray.hpp"
#include "SIMPLib/DataArrays/StringDataArray.hpp"
#include "SIMPLib/DataArrays/StatsDataArray.h"
#include "SIMPLib/DataArrays/NeighborList.hpp"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
SIMPLH5DataReaderRequirements::SIMPLH5DataReaderRequirements(size_t allowedCompDim)
{
  m_PrimitiveTypeFlags = PrimitiveTypeFlag::Any_PType;

  m_DCGeometryTypeFlags = DCGeometryTypeFlag::Any_DCGeomType;

  m_AMTypeFlags = AMTypeFlag::Any_AMType;

  if(SIMPL::Defaults::AnyComponentSize != allowedCompDim)
  {
    m_ComponentDimensions = QVector<QVector<size_t> >(1, QVector<size_t>(1, allowedCompDim));
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
SIMPLH5DataReaderRequirements::~SIMPLH5DataReaderRequirements()
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QVector<QString> SIMPLH5DataReaderRequirements::getPrimitiveTypes()
{
  QVector<QString> typeStrings;
  if (m_PrimitiveTypeFlags.testFlag(PrimitiveTypeFlag::Bool_PType) == true)
  {
    BoolArrayType::Pointer da = BoolArrayType::CreateArray(0, "DummyArray", false);
    typeStrings.push_back(da->getFullNameOfClass());
  }
  if (m_PrimitiveTypeFlags.testFlag(PrimitiveTypeFlag::Double_PType) == true)
  {
    DoubleArrayType::Pointer da = DoubleArrayType::CreateArray(0, "DummyArray", false);
    typeStrings.push_back(da->getFullNameOfClass());
  }
  if (m_PrimitiveTypeFlags.testFlag(PrimitiveTypeFlag::Float_PType) == true)
  {
    FloatArrayType::Pointer da = FloatArrayType::CreateArray(0, "DummyArray", false);
    typeStrings.push_back(da->getFullNameOfClass());
  }
  if (m_PrimitiveTypeFlags.testFlag(PrimitiveTypeFlag::Int8_PType) == true)
  {
    Int8ArrayType::Pointer da = Int8ArrayType::CreateArray(0, "DummyArray", false);
    typeStrings.push_back(da->getFullNameOfClass());
  }
  if (m_PrimitiveTypeFlags.testFlag(PrimitiveTypeFlag::Int16_PType) == true)
  {
    Int16ArrayType::Pointer da = Int16ArrayType::CreateArray(0, "DummyArray", false);
    typeStrings.push_back(da->getFullNameOfClass());
  }
  if (m_PrimitiveTypeFlags.testFlag(PrimitiveTypeFlag::Int32_PType) == true)
  {
    Int32ArrayType::Pointer da = Int32ArrayType::CreateArray(0, "DummyArray", false);
    typeStrings.push_back(da->getFullNameOfClass());
  }
  if (m_PrimitiveTypeFlags.testFlag(PrimitiveTypeFlag::Int64_PType) == true)
  {
    Int64ArrayType::Pointer da = Int64ArrayType::CreateArray(0, "DummyArray", false);
    typeStrings.push_back(da->getFullNameOfClass());
  }
  if (m_PrimitiveTypeFlags.testFlag(PrimitiveTypeFlag::UInt8_PType) == true)
  {
    UInt8ArrayType::Pointer da = UInt8ArrayType::CreateArray(0, "DummyArray", false);
    typeStrings.push_back(da->getFullNameOfClass());
  }
  if (m_PrimitiveTypeFlags.testFlag(PrimitiveTypeFlag::UInt16_PType) == true)
  {
    UInt16ArrayType::Pointer da = UInt16ArrayType::CreateArray(0, "DummyArray", false);
    typeStrings.push_back(da->getFullNameOfClass());
  }
  if (m_PrimitiveTypeFlags.testFlag(PrimitiveTypeFlag::UInt32_PType) == true)
  {
    UInt32ArrayType::Pointer da = UInt32ArrayType::CreateArray(0, "DummyArray", false);
    typeStrings.push_back(da->getFullNameOfClass());
  }
  if (m_PrimitiveTypeFlags.testFlag(PrimitiveTypeFlag::UInt64_PType) == true)
  {
    UInt64ArrayType::Pointer da = UInt64ArrayType::CreateArray(0, "DummyArray", false);
    typeStrings.push_back(da->getFullNameOfClass());
  }
  if (m_PrimitiveTypeFlags.testFlag(PrimitiveTypeFlag::StringArray_PType) == true)
  {
    StringDataArray::Pointer da = StringDataArray::CreateArray(0, "DummyArray", false);
    typeStrings.push_back(da->getFullNameOfClass());
  }
  if (m_PrimitiveTypeFlags.testFlag(PrimitiveTypeFlag::StatsDataArray_PType) == true)
  {
    StatsDataArray::Pointer da = StatsDataArray::CreateArray(0, "DummyArray", false);
    typeStrings.push_back(da->getFullNameOfClass());
  }
  if (m_PrimitiveTypeFlags.testFlag(PrimitiveTypeFlag::NeighborList_PType) == true)
  {
//    NeighborList::Pointer da = NeighborList::CreateArray(0, "DummyArray", false);
//    typeStrings.push_back(da->getNameOfClass());
  }

  return typeStrings;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QVector<AttributeMatrix::Type> SIMPLH5DataReaderRequirements::getAMTypes()
{
  QVector<AttributeMatrix::Type> amTypes;
  if (m_AMTypeFlags.testFlag(AMTypeFlag::Cell_AMType) == true)
  {
    amTypes.push_back(AttributeMatrix::Type::Cell);
  }
  if (m_AMTypeFlags.testFlag(AMTypeFlag::CellEnsemble_AMType) == true)
  {
    amTypes.push_back(AttributeMatrix::Type::CellEnsemble);
  }
  if (m_AMTypeFlags.testFlag(AMTypeFlag::CellFeature_AMType) == true)
  {
    amTypes.push_back(AttributeMatrix::Type::CellFeature);
  }
  if (m_AMTypeFlags.testFlag(AMTypeFlag::EdgeEnsemble_AMType) == true)
  {
    amTypes.push_back(AttributeMatrix::Type::EdgeEnsemble);
  }
  if (m_AMTypeFlags.testFlag(AMTypeFlag::EdgeFeature_AMType) == true)
  {
    amTypes.push_back(AttributeMatrix::Type::EdgeFeature);
  }
  if (m_AMTypeFlags.testFlag(AMTypeFlag::Face_AMType) == true)
  {
    amTypes.push_back(AttributeMatrix::Type::Face);
  }
  if (m_AMTypeFlags.testFlag(AMTypeFlag::FaceEnsemble_AMType) == true)
  {
    amTypes.push_back(AttributeMatrix::Type::FaceEnsemble);
  }
  if (m_AMTypeFlags.testFlag(AMTypeFlag::FaceFeature_AMType) == true)
  {
    amTypes.push_back(AttributeMatrix::Type::FaceFeature);
  }
  if (m_AMTypeFlags.testFlag(AMTypeFlag::Generic_AMType) == true)
  {
    amTypes.push_back(AttributeMatrix::Type::Generic);
  }
  if (m_AMTypeFlags.testFlag(AMTypeFlag::MetaData_AMType) == true)
  {
    amTypes.push_back(AttributeMatrix::Type::MetaData);
  }
  if (m_AMTypeFlags.testFlag(AMTypeFlag::Vertex_AMType) == true)
  {
    amTypes.push_back(AttributeMatrix::Type::Vertex);
  }
  if (m_AMTypeFlags.testFlag(AMTypeFlag::VertexEnsemble_AMType) == true)
  {
    amTypes.push_back(AttributeMatrix::Type::VertexEnsemble);
  }
  if (m_AMTypeFlags.testFlag(AMTypeFlag::VertexFeature_AMType) == true)
  {
    amTypes.push_back(AttributeMatrix::Type::VertexFeature);
  }

  return amTypes;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QVector<IGeometry::Type> SIMPLH5DataReaderRequirements::getDCGeometryTypes()
{
  QVector<IGeometry::Type> dcGeomTypes;
  if (m_DCGeometryTypeFlags.testFlag(DCGeometryTypeFlag::Edge_DCGeomType) == true)
  {
    dcGeomTypes.push_back(IGeometry::Type::Edge);
  }
  if (m_DCGeometryTypeFlags.testFlag(DCGeometryTypeFlag::Image_DCGeomType) == true)
  {
    dcGeomTypes.push_back(IGeometry::Type::Image);
  }
  if (m_DCGeometryTypeFlags.testFlag(DCGeometryTypeFlag::Quad_DCGeomType) == true)
  {
    dcGeomTypes.push_back(IGeometry::Type::Quad);
  }
  if (m_DCGeometryTypeFlags.testFlag(DCGeometryTypeFlag::RectGrid_DCGeomType) == true)
  {
    dcGeomTypes.push_back(IGeometry::Type::RectGrid);
  }
  if (m_DCGeometryTypeFlags.testFlag(DCGeometryTypeFlag::Tetrahedral_DCGeomType) == true)
  {
    dcGeomTypes.push_back(IGeometry::Type::Tetrahedral);
  }
  if (m_DCGeometryTypeFlags.testFlag(DCGeometryTypeFlag::Triangle_DCGeomType) == true)
  {
    dcGeomTypes.push_back(IGeometry::Type::Triangle);
  }
  if (m_DCGeometryTypeFlags.testFlag(DCGeometryTypeFlag::Vertex_DCGeomType) == true)
  {
    dcGeomTypes.push_back(IGeometry::Type::Vertex);
  }

  return dcGeomTypes;
}
