/*************** <auto-copyright.rb BEGIN do not edit this line> **************
 *
 * Copyright 2012-2012 by Ames Laboratory
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 *************** <auto-copyright.rb END do not edit this line> ***************/
#ifndef __LATTICEFX_CORE_VTK_BASE_RTP_OPERATION_H__
#define __LATTICEFX_CORE_VTK_BASE_RTP_OPERATION_H__ 1

#include <latticefx/core/RTPOperation.h>
#include <latticefx/core/LogBase.h>

#include <latticefx/core/vtk/Export.h>

#include <latticefx/core/vtk/CuttingPlane.h>

#include <vtkSmartPointer.h>
#include <vtkExtractGeometry.h>
#include <vtkExtractPolyDataGeometry.h>
#include <vtkAlgorithmOutput.h>

namespace lfx
{

namespace core
{

namespace vtk
{

/** \class VTKBaseRTP VTKBaseRTP.h <latticefx/core/vtk/VTKBaseRTP.h>
 \brief Base class for VTK based RTP operations.
 \details This class holds common functions that most VTK RTP operations utilized
 in creating vtkPolyData. */

class LATTICEFX_CORE_VTK_EXPORT VTKBaseRTP : protected lfx::core::LogBase, public lfx::core::RTPOperation
{
public:

    ///Default constructor
    VTKBaseRTP( const RTPOpType rtpOpType, const std::string& logName=std::string( "" ) )
        :
        lfx::core::LogBase( logName.empty() ? "lfx.core.vtk.VTKBaseRTP" : logName ),
        lfx::core::RTPOperation( rtpOpType ),
        m_requestedValue( 0.2 ),
        m_minScalarValue( 0.0 ),
        m_maxScalarValue( 100.0 ),
        m_mask( 1.0 ),
        m_planeDirection( CuttingPlane::X_PLANE ),
		m_roiExtractBoundaryCells( true )
    {
        ;
    }

    ///Destructor
    virtual ~VTKBaseRTP()
    {
        ;
    }

    ///Set the value for a plane location or scalar value for an isosurface
    void SetRequestedValue( double const value );

    ///Set the min max values for scalar ranges for coloring objects
    void SetMinMaxScalarRangeValue( double const minVal, double const maxVal );

    ///Set the active scalar to use for this pipeline
    void SetActiveScalar( std::string const scalarName );

    ///Set the active vector to use for this pipeline
    void SetActiceVector( std::string const vectorName );

    ///If points need to be masked via the vtkMaskPoints class this method
    ///controls the number of points masked
    void SetMaskValue( double const value );

    ///Set the plane direction if it is needed for this rtp operation
    void SetPlaneDirection( const CuttingPlane::SliceDirection& planeDirection );

	//Set the region of interest bounding box
	//vector must contain 6 values in this order: xmin,xmax,ymin,ymax,zmin,zmax
	//If no roi is set, the default sate, then the entire dataset will be used for any operations
	void SetRoiBox(const std::vector<double> &roiBox);

	void ExtractBoundaryCells(bool extract) { m_roiExtractBoundaryCells = extract; }

protected:
	vtkSmartPointer<vtkExtractGeometry> GetRoi(vtkDataObject *pdo);
	vtkSmartPointer<vtkExtractGeometry> GetRoi(vtkAlgorithmOutput* pOutPin);
	vtkSmartPointer<vtkExtractPolyDataGeometry> GetRoiPoly(vtkAlgorithmOutput* pOutPin);

protected:
    ///Value for setting the position or value for an iso surface
    double m_requestedValue;

    ///Values for setting color ranges on full VTK pipelines
    double m_minScalarValue;
    double m_maxScalarValue;

    ///The active scalar name
    std::string m_activeScalar;

    ///The active vector name
    std::string m_activeVector;

    ///Number of points to be masked
    double m_mask;

    ///Plane direction
    CuttingPlane::SliceDirection m_planeDirection;

	std::vector<double> m_roiBox;
	bool m_roiExtractBoundaryCells;
};

typedef boost::shared_ptr< VTKBaseRTP > VTKBaseRTPPtr;

}
}
}
// __LATTICEFX_CORE_VTK_ISOSURFACE_RTP_OPERATION_H__
#endif