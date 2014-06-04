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
#include <latticefx/core/vtk/VTKPolyDataPointsRTP.h>
#include <latticefx/core/vtk/ChannelDatavtkPolyData.h>
#include <latticefx/core/vtk/ChannelDatavtkDataObject.h>

#include <vtkDataObject.h>
#include <vtkPolyData.h>

namespace lfx
{

namespace core
{

namespace vtk
{

////////////////////////////////////////////////////////////////////////////////
lfx::core::ChannelDataPtr VTKPolyDataPointsRTP::channel( const lfx::core::ChannelDataPtr maskIn )
{
    lfx::core::vtk::ChannelDatavtkDataObjectPtr cddoPtr = boost::static_pointer_cast< lfx::core::vtk::ChannelDatavtkDataObject >( getInput( "vtkDataObject" ) );
    vtkDataObject* vtkdo = cddoPtr->GetDataObject();

	if( vtkdo == NULL ) return lfx::core::ChannelDataPtr();
	if( !vtkdo->IsA( "vtkPolyData" ) ) return lfx::core::ChannelDataPtr();

	vtkPolyData* pd = ( vtkPolyData* )vtkdo;
	//vtkCellTypes* types = vtkCellTypes::New();
    //pd->GetCellTypes( types );

	vtkPoints *pts = pd->GetPoints();
	if( !pts )
	{
		std::cout << "VTKPolyDataPointsRTP::channel : There are no points in this polydata." << std::endl;
	}

	return lfx::core::vtk::ChannelDatavtkPolyDataPtr( new lfx::core::vtk::ChannelDatavtkPolyData( pd, "vtkPolyData" ) );
}

////////////////////////////////////////////////////////////////////////////////
void VTKPolyDataPointsRTP::serializeData( JsonSerializer *json ) const
{
	// let the parent write its data
	VTKBaseRTP::serializeData( json );

	json->insertObj( VTKPolyDataPointsRTP::getClassName(), true );
	//json->insertObjValue( "warpSurface",  _warpSurface );
	json->popParent();
}

////////////////////////////////////////////////////////////////////////////////
bool VTKPolyDataPointsRTP::loadData( JsonSerializer *json, IObjFactory *pfactory, std::string *perr )
{
	// let the parent load its data
	if ( !VTKBaseRTP::loadData( json, pfactory, perr )) return false;

	// get to this classes data
	if ( !json->getObj( VTKPolyDataPointsRTP::getClassName() ) )
	{
		if (perr) *perr = "Json: Failed to get VTKPolyDataPointsRTP data";
		return false;
	}

	//json->getValue( "warpSurface", &_warpSurface );
	json->popParent();
	return true;
}

////////////////////////////////////////////////////////////////////////////////
void VTKPolyDataPointsRTP::dumpState( std::ostream &os )
{
	VTKBaseRTP::dumpState( os );

	dumpStateStart( VTKPolyDataPointsRTP::getClassName(), os );
	//os << "_warpSurface: " << _warpSurface<< std::endl;
	dumpStateEnd( VTKPolyDataPointsRTP::getClassName(), os );
}

////////////////////////////////////////////////////////////////////////////////
}
}
}

