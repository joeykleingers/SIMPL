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

#ifndef _simplh5datareaderrequirements_h_
#define _simplh5datareaderrequirements_h_

#include "SIMPLib/SIMPLib.h"

#include "SIMPLib/Common/SIMPLibSetGetMacros.h"
#include "SIMPLib/DataContainers/AttributeMatrix.h"
#include "SIMPLib/Geometry/IGeometry.h"

/**
 * @brief The SIMPLH5DataReaderRequirements class
 */
class SIMPLib_EXPORT SIMPLH5DataReaderRequirements
{
  typedef QVector< QVector<size_t> > QVectorSizeT;

  public:
    SIMPLH5DataReaderRequirements(size_t allowedCompDim);

    virtual ~SIMPLH5DataReaderRequirements();

    // This enumeration is not a class enumeration because it is not possible to
    // do a bit-wise NOT operation on a class enumeration value.  We need to be
    // able to do a bit-wise NOT operation so that we can turn off certain flags.
    // This enumeration allows us to flip integer bits to turn on/off various types.
    enum PrimitiveTypeFlag : unsigned int {
      Unknown_PType = 0x0,
      Bool_PType = 0x1,
      Float_PType = 0x2,
      Double_PType = 0x4,
      Int8_PType = 0x8,
      UInt8_PType = 0x10,
      Int16_PType = 0x20,
      UInt16_PType = 0x40,
      Int32_PType = 0x80,
      UInt32_PType = 0x100,
      Int64_PType = 0x200,
      UInt64_PType = 0x400,
      StatsDataArray_PType = 0x800,
      NeighborList_PType = 0x1000,
      StringArray_PType = 0x2000,
      NumericalPrimitives_PType = 0x07FE,
      Any_PType = 0x3FFF
    };
    Q_DECLARE_FLAGS(PrimitiveTypeFlags, PrimitiveTypeFlag)

    // This enumeration is not a class enumeration because it is not possible to
    // do a bit-wise NOT operation on a class enumeration value.  We need to be
    // able to do a bit-wise NOT operation so that we can turn off certain flags.
    // This enumeration allows us to flip integer bits to turn on/off various types.
    enum AMTypeFlag : unsigned int {
      Unknown_AMType = 0x0,
      Vertex_AMType = 0x1,
      Face_AMType = 0x2,
      Cell_AMType = 0x4,
      VertexFeature_AMType = 0x8,
      EdgeFeature_AMType = 0x10,
      FaceFeature_AMType = 0x20,
      CellFeature_AMType = 0x40,
      VertexEnsemble_AMType = 0x80,
      EdgeEnsemble_AMType = 0x100,
      FaceEnsemble_AMType = 0x200,
      CellEnsemble_AMType = 0x400,
      MetaData_AMType = 0x800,
      Generic_AMType = 0x1000,
      Any_AMType = 0x1FFF
    };
    Q_DECLARE_FLAGS(AMTypeFlags, AMTypeFlag)

    // This enumeration is not a class enumeration because it is not possible to
    // do a bit-wise NOT operation on a class enumeration value.  We need to be
    // able to do a bit-wise NOT operation so that we can turn off certain flags.
    // This enumeration allows us to flip integer bits to turn on/off various types.
    enum DCGeometryTypeFlag : unsigned int {
      Unknown_DCGeomType = 0x0,
      Image_DCGeomType = 0x1,
      RectGrid_DCGeomType = 0x2,
      Vertex_DCGeomType = 0x4,
      Edge_DCGeomType = 0x8,
      Triangle_DCGeomType = 0x10,
      Quad_DCGeomType = 0x20,
      Tetrahedral_DCGeomType = 0x40,
      Any_DCGeomType = 0x7F
    };
    Q_DECLARE_FLAGS(DCGeometryTypeFlags, DCGeometryTypeFlag)

    SIMPL_INSTANCE_PROPERTY(DCGeometryTypeFlags, DCGeometryTypeFlags)
    SIMPL_INSTANCE_PROPERTY(AMTypeFlags, AMTypeFlags)
    SIMPL_INSTANCE_PROPERTY(PrimitiveTypeFlags, PrimitiveTypeFlags)

    SIMPL_INSTANCE_PROPERTY(QVectorSizeT, ComponentDimensions)

    /**
     * @brief getPrimitiveTypes
     * @return
     */
    QVector<QString> getPrimitiveTypes();

    /**
     * @brief getAMTypes
     * @return
     */
    QVector<AttributeMatrix::Type> getAMTypes();

    /**
     * @brief getDCGeometryTypes
     * @return
     */
    QVector<IGeometry::Type> getDCGeometryTypes();

  private:

};
Q_DECLARE_OPERATORS_FOR_FLAGS(SIMPLH5DataReaderRequirements::PrimitiveTypeFlags)
Q_DECLARE_OPERATORS_FOR_FLAGS(SIMPLH5DataReaderRequirements::AMTypeFlags)
Q_DECLARE_OPERATORS_FOR_FLAGS(SIMPLH5DataReaderRequirements::DCGeometryTypeFlags)

#endif /* _simplh5datareaderrequirements_h_ */
