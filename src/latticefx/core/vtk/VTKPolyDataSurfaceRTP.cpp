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
#include <latticefx/core/vtk/VTKPolyDataSurfaceRTP.h>
#include <latticefx/core/vtk/ChannelDatavtkPolyDataMapper.h>
#include <latticefx/core/vtk/ChannelDatavtkDataObject.h>
#include <latticefx/core/MiscUtils.h>

#include <vtkDataObject.h>
#include <vtkCompositeDataGeometryFilter.h>
#include <vtkPolyDataMapper.h>
#include <vtkCellDataToPointData.h>
#include <vtkPolyDataNormals.h>
#include <vtkWarpVector.h>
#include <vtkAlgorithm.h>
#include <vtkAlgorithmOutput.h>

namespace lfx
{

namespace core
{

namespace vtk
{

////////////////////////////////////////////////////////////////////////////////
lfx::core::ChannelDataPtr VTKPolyDataSurfaceRTP::channel( const lfx::core::ChannelDataPtr maskIn )
{
    lfx::core::vtk::ChannelDatavtkDataObjectPtr cddoPtr = boost::static_pointer_cast< lfx::core::vtk::ChannelDatavtkDataObject >( getInput( "vtkDataObject" ) );
    vtkDataObject* vtkdo = cddoPtr->GetDataObject();
	lfx::core::vtk::ChannelDatavtkPolyDataMapperPtr cdpd;

	if( vtkdo == NULL ) return NULL;
	if( !vtkdo->IsA( "vtkPolyData" ) ) return NULL;

	vtkPolyData* pd = ( vtkPolyData* )vtkdo;
	//vtkCellTypes* types = vtkCellTypes::New();
    //pd->GetCellTypes( types );

	vtkPoints *pts = pd->GetPoints();
	if( !pts )
	{
		std::cout << "VTKPolyDataSurfaceRTP::channel : There are no points in this surface." << std::endl;
	}

	vtkPolyDataNormals* normalGen = vtkPolyDataNormals::New();
	normalGen->SetInput( pd );
    normalGen->NonManifoldTraversalOn();
	normalGen->AutoOrientNormalsOn();
    normalGen->ConsistencyOn();
    normalGen->SplittingOn();

	if( _warpSurface )
	{
		vtkWarpVector* warper = vtkWarpVector::New();
		vtkPolyData* pd2 = normalGen->GetOutput();
		vtkPoints *pts2 = pd2->GetPoints(); // if we don't have any points, then we need to go back to the original polydata.
		if( !pts2 )
		{
			warper->SetInput( pd );
		}
		else
		{
			warper->SetInput( normalGen->GetOutput() );
		}

        warper->SetScaleFactor( _warpedContourScale );
        warper->Update();//can this go???
		cdpd.reset( new lfx::core::vtk::ChannelDatavtkPolyDataMapper( warper->GetOutputPort(), "vtkPolyDataMapper" ) );
		warper->Delete();
	}
	else
	{
		vtkPolyData* pd2 = normalGen->GetOutput();
		vtkPoints *pts2 = pd2->GetPoints();
		if( !pts2 )
		{
			// if we don't have any points, then we need to go back to the original polydata.
			cdpd.reset( new lfx::core::vtk::ChannelDatavtkPolyDataMapper( pd, "vtkPolyDataMapper" ) );
		}
		else
		{
			cdpd.reset( new lfx::core::vtk::ChannelDatavtkPolyDataMapper( normalGen->GetOutput(), "vtkPolyDataMapper" ) );
		}
	}

    normalGen->Delete();

    return( cdpd );
}

////////////////////////////////////////////////////////////////////////////////
bool VTKPolyDataSurfaceRTP::setWarpSurface( bool value )
{
	if( _warpSurface != value )
	{
		_warpSurface = value;
		return true;
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////
bool VTKPolyDataSurfaceRTP::setWarpedContourScale( double value )
{
	if( MiscUtils::isnot_close( _warpedContourScale , value, .001 ) )
	{
		_warpedContourScale = value;
		return true;
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////
void VTKPolyDataSurfaceRTP::serializeData( JsonSerializer *json ) const
{
	// let the parent write its data
	VTKBaseRTP::serializeData( json );

	json->insertObj( VTKPolyDataSurfaceRTP::getClassName(), true );
	json->insertObjValue( "warpSurface",  _warpSurface );
	json->insertObjValue( "warpedContourScale",  _warpedContourScale );
	json->popParent();
}

////////////////////////////////////////////////////////////////////////////////
bool VTKPolyDataSurfaceRTP::loadData( JsonSerializer *json, IObjFactory *pfactory, std::string *perr )
{
	// let the parent load its data
	if ( !VTKBaseRTP::loadData( json, pfactory, perr )) return false;

	// get to this classes data
	if ( !json->getObj( VTKPolyDataSurfaceRTP::getClassName() ) )
	{
		if (perr) *perr = "Json: Failed to get VTKPolyDataSurfaceRTP data";
		return false;
	}

	json->getValue( "warpSurface", &_warpSurface );
	json->getValue( "warpedContourScale", &_warpedContourScale );
	json->popParent();
	return true;
}

////////////////////////////////////////////////////////////////////////////////
void VTKPolyDataSurfaceRTP::dumpState( std::ostream &os )
{
	VTKBaseRTP::dumpState( os );

	dumpStateStart( VTKPolyDataSurfaceRTP::getClassName(), os );
	os << "_warpSurface: " << _warpSurface<< std::endl;
	os << "_warpedContourScale: " << _warpedContourScale<< std::endl;
	dumpStateEnd( VTKPolyDataSurfaceRTP::getClassName(), os );
}

////////////////////////////////////////////////////////////////////////////////
}
}
}