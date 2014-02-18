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
#include <latticefx/core/vtk/VTKBaseRTP.h>
#include <latticefx/core/MiscUtils.h>

#include <vtkBox.h>
#include <vtkDataObject.h>
#include <vtkCompositeDataGeometryFilter.h>
#include <vtkDataSetSurfaceFilter.h>
#include <vtkAlgorithm.h>
#include <vtkAlgorithmOutput.h>

namespace lfx
{

namespace core
{

namespace vtk
{

////////////////////////////////////////////////////////////////////////////////
bool VTKBaseRTP::SetMinMaxScalarRangeValue( double const minVal, double const maxVal )
{   
	bool modified = false;

	if( MiscUtils::isnot_close( m_minScalarValue, minVal, .001 ) )
	{
		m_minScalarValue = minVal;
		modified = true;
	}

	if( MiscUtils::isnot_close( m_maxScalarValue, maxVal, .001 ) )
	{
		m_maxScalarValue = maxVal;
		modified = true;
	}

	return modified;
}

////////////////////////////////////////////////////////////////////////////////
void VTKBaseRTP::GetMinMaxScalarRangeValue( double *minVal, double *maxVal )
{
	*minVal = m_minScalarValue;
    *maxVal = m_maxScalarValue;
}

////////////////////////////////////////////////////////////////////////////////
bool VTKBaseRTP::SetRequestedValue( double const value )
{
	if( MiscUtils::isnot_close( m_requestedValue, value, .001 ) )
	{
		m_requestedValue = value;
		return true;
	}

	return false;
}
////////////////////////////////////////////////////////////////////////////////
void VTKBaseRTP::SetActiveScalar( std::string const scalarName )
{
    m_activeScalar = scalarName;
}
////////////////////////////////////////////////////////////////////////////////
std::string VTKBaseRTP::GetActiveScalar()
{
	return m_activeScalar;
}
////////////////////////////////////////////////////////////////////////////////
void VTKBaseRTP::SetActiveVector( std::string const vectorName )
{
    m_activeVector = vectorName;
}
////////////////////////////////////////////////////////////////////////////////
std::string VTKBaseRTP::GetActiveVector()
{
	return m_activeVector;
}
////////////////////////////////////////////////////////////////////////////////
void VTKBaseRTP::SetMaskValue( double const value )
{
    m_mask = value;
}
////////////////////////////////////////////////////////////////////////////////
void VTKBaseRTP::SetPlaneDirection( const CuttingPlane::SliceDirection& planeDirection )
{
    m_planeDirection = planeDirection;
}

////////////////////////////////////////////////////////////////////////////////
bool VTKBaseRTP::SetPlaneDirection( int planeDirection )
{
	if( planeDirection < CuttingPlane::SliceDirection::BEGIN ) planeDirection = CuttingPlane::SliceDirection::BEGIN;
	if( planeDirection > CuttingPlane::SliceDirection::END ) planeDirection = CuttingPlane::SliceDirection::END;

	if( m_planeDirection == planeDirection ) return false;

	m_planeDirection = (CuttingPlane::SliceDirection)planeDirection;
	return true;
}

////////////////////////////////////////////////////////////////////////////////
void VTKBaseRTP::setDatasetBounds(double *bounds)
{
	for( int i=0; i<6; i++ )
	{
		_dsBounds[i] = bounds[i];
	}
}

////////////////////////////////////////////////////////////////////////////////
void VTKBaseRTP::SetRoiBox(const std::vector<double> &roiBox)
{
	m_roiBox = roiBox;
}
////////////////////////////////////////////////////////////////////////////////
vtkSmartPointer<vtkExtractGeometry> VTKBaseRTP::GetRoi(vtkDataObject *pdo)
{
	if (m_roiBox.size() < 6)
    {
        std::cout << "VTKBaseRTP::GetRoi : No bounding box set." << std::endl;
        return vtkSmartPointer<vtkExtractGeometry>();
    }

	vtkSmartPointer<vtkBox> boxExtract = vtkSmartPointer<vtkBox>::New();
	boxExtract->SetBounds(&m_roiBox[0]);
 
	vtkSmartPointer<vtkExtractGeometry> extract = vtkSmartPointer<vtkExtractGeometry>::New();
	extract->SetImplicitFunction(boxExtract);

	if (m_roiExtractBoundaryCells)
		extract->SetExtractBoundaryCells(1);
	else
		extract->SetExtractBoundaryCells(0);

	extract->SetInput(pdo);

	return extract;
}
////////////////////////////////////////////////////////////////////////////////
vtkSmartPointer<vtkExtractGeometry> VTKBaseRTP::GetRoi(vtkAlgorithmOutput* pOutPin)
{
	if (m_roiBox.size() < 6)
    {
        std::cout << "VTKBaseRTP::GetRoi : No bounding box set." << std::endl;
        return vtkSmartPointer<vtkExtractGeometry>();
    }

	vtkSmartPointer<vtkBox> boxExtract = vtkSmartPointer<vtkBox>::New();
	boxExtract->SetBounds(&m_roiBox[0]);
 
	vtkSmartPointer<vtkExtractGeometry> extract = vtkSmartPointer<vtkExtractGeometry>::New();
	extract->SetImplicitFunction(boxExtract);

	if (m_roiExtractBoundaryCells)
		extract->SetExtractBoundaryCells(1);
	else
		extract->SetExtractBoundaryCells(0);

	extract->SetInputConnection(0, pOutPin);

	return extract;
}
////////////////////////////////////////////////////////////////////////////////
vtkSmartPointer<vtkExtractPolyDataGeometry> VTKBaseRTP::GetRoiPoly(vtkAlgorithmOutput* pOutPin)
{
	if (m_roiBox.size() < 6)
    {
        std::cout << "VTKBaseRTP::GetRoiPoly : No bounding box set." << std::endl;
        return vtkSmartPointer<vtkExtractPolyDataGeometry>();
    }

	vtkSmartPointer<vtkBox> boxExtract = vtkSmartPointer<vtkBox>::New();
	boxExtract->SetBounds(&m_roiBox[0]);
 
	vtkSmartPointer<vtkExtractPolyDataGeometry> extract = vtkSmartPointer<vtkExtractPolyDataGeometry>::New();
	extract->SetImplicitFunction(boxExtract);

	if (m_roiExtractBoundaryCells)
		extract->SetExtractBoundaryCells(1);
	else
		extract->SetExtractBoundaryCells(0);
	

	extract->SetInputConnection(0, pOutPin);

	return extract;
}

////////////////////////////////////////////////////////////////////////////////
void VTKBaseRTP::ConnectGeometryFilter( vtkDataObject* tempVtkDO, vtkAlgorithmOutput* outputPort, vtkAlgorithm* connectTo )
{
    if( tempVtkDO->IsA( "vtkCompositeDataSet" ) )
    {
        vtkCompositeDataGeometryFilter* multiGroupGeomFilter = vtkCompositeDataGeometryFilter::New();
        multiGroupGeomFilter->SetInputConnection( outputPort );
        connectTo->SetInputConnection( multiGroupGeomFilter->GetOutputPort( 0 ) );
        multiGroupGeomFilter->Delete();
    }
    else
    {
        vtkDataSetSurfaceFilter* surfaceFilter = vtkDataSetSurfaceFilter::New();
        surfaceFilter->SetInputConnection( outputPort );
        connectTo->SetInputConnection( surfaceFilter->GetOutputPort() );
        surfaceFilter->Delete();
    }
}

////////////////////////////////////////////////////////////////////////////////
void VTKBaseRTP::serializeData( JsonSerializer *json ) const
{
	// let the parent write its data
	RTPOperation::serializeData( json );

	json->insertObj( VTKBaseRTP::getClassName(), true);
	json->insertObjValue( "requestedValue",  m_requestedValue );
	json->insertObjValue( "minScalarValue",  m_minScalarValue );
	json->insertObjValue( "maxScalarValue",  m_maxScalarValue );
	json->insertObjValue( "activeScalar",  m_activeScalar );
	json->insertObjValue( "activeVector",  m_activeVector );
	json->insertObjValue( "mask",  m_mask );
	json->insertObjValue( "planeDirection",  CuttingPlane::getEnumName( m_planeDirection ) );
	json->insertObjValue( "roiExtractBoundaryCells",  m_roiExtractBoundaryCells );
	json->insertArray( "roiBox", true );
	for( unsigned int i=0; i < m_roiBox.size(); i++ )
	{
		json->insertArrValue( m_roiBox[i] );
	}

	json->popParent();
	json->popParent();
}

////////////////////////////////////////////////////////////////////////////////
bool VTKBaseRTP::loadData( JsonSerializer *json, IObjFactory *pfactory, std::string *perr )
{
	// let the parent load its data
	if ( !RTPOperation::loadData( json, pfactory, perr )) return false;

	// get to this classes data
	if ( !json->getObj( VTKBaseRTP::getClassName() ) )
	{
		if (perr) *perr = "Json: Failed to get VTKBaseRTP data";
		return false;
	}

	json->getValue( "requestedValue",  &m_requestedValue );
	json->getValue( "minScalarValue",  &m_minScalarValue );
	json->getValue( "maxScalarValue",  &m_maxScalarValue );
	json->getValue( "activeScalar",  &m_activeScalar );
	json->getValue( "activeVector",  &m_activeVector );
	json->getValue( "mask",  &m_mask );
	json->getValue( "roiExtractBoundaryCells",  &m_roiExtractBoundaryCells );

	std::string name;
	json->getValue( "planeDirection", &name, CuttingPlane::getEnumName( m_planeDirection ) );
	m_planeDirection = CuttingPlane::getEnumFromName( name );

	m_roiBox.clear();
	json->getArray( "roiBox", true );
	for( unsigned int i=0; i<json->size(); i++)
	{
		double d=0;
		json->getValue( i, &d );
		m_roiBox.push_back( d );
	}

	json->popParent();
	json->popParent();
	return true;
}

////////////////////////////////////////////////////////////////////////////////
void VTKBaseRTP::dumpState( std::ostream &os )
{
	RTPOperation::dumpState( os );

	dumpStateStart( VTKBaseRTP::getClassName(), os );
	os << "_requestedValue: " << m_requestedValue << std::endl;
	os << "_minScalarValue: " << m_minScalarValue << std::endl;
	os << "_maxScalarValue: " << m_maxScalarValue << std::endl;
	os << "_activeScalar: " << m_activeScalar << std::endl;
	os << "_activeVector: " << m_activeVector << std::endl;
	os << "_mask: " << m_mask << std::endl;
	os << "_roiExtractBoundaryCells: " << m_roiExtractBoundaryCells << std::endl;
	os << "_planeDirection " << CuttingPlane::getEnumName( m_planeDirection ) << std::endl;
	os << "_roiBox size: " << m_roiBox.size() << std::endl;
	for( unsigned int i=0; i < m_roiBox.size(); i++ )
	{
		os << "_roiBox[" << i << "]: " << m_roiBox[i] << std::endl;
	}

	dumpStateEnd( VTKBaseRTP::getClassName(), os );
}

////////////////////////////////////////////////////////////////////////////////
}
}
}
