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

#include <latticefx/core/vtk/ObjFactoryVtk.h>
#include <latticefx/core/vtk/VTKBaseRTP.h>
#include <latticefx/core/vtk/VTKContourSliceRTP.h>
#include <latticefx/core/vtk/VTKIsoSurfaceRTP.h>
#include <latticefx/core/vtk/VTKPrimitiveSetGenerator.h>
#include <latticefx/core/vtk/VTKSurfaceRenderer.h>
#include <latticefx/core/vtk/VTKSurfaceWrapRTP.h>
#include <latticefx/core/vtk/VTKVectorFieldGlyphRTP.h>
#include <latticefx/core/vtk/VTKVectorFieldRTP.h>
#include <latticefx/core/vtk/VTKVectorRenderer.h>
#include <latticefx/core/vtk/VTKVectorSliceRTP.h>

#include <sstream>

namespace lfx
{
namespace core
{
namespace vtk
{

ObjFactoryVtk::ObjFactoryVtk( PluginManager *pm )
	: ObjFactoryCore( pm )
{
}

ObjBasePtr ObjFactoryVtk::createObj(const std::string &typeName, const ObjBase::KeyDataMap &map, std::string *perr)
{
	bool berr = false;
	ObjBasePtr ptr = createPlugin( map, &berr, perr );
	if( ptr ) return ptr;
	if( berr ) return ptr;

	// must not be a plugin so just create the vtk operation type
	ptr = createVtkObj( typeName, perr );
	if( ptr ) return ptr;

	// must not be a vtk type so try and create a standard operation type
	ptr = createStandardObj( typeName, perr );
	if( ptr ) return ptr;


	if (perr)
	{
		std::stringstream ss;
		ss << "Failed to create object with typename: " << typeName;
		*perr = ss.str();
	}

	return ObjBasePtr();
}

ObjBasePtr ObjFactoryVtk::createVtkObj( const std::string &typeName, std::string *perr )
{
	if( !typeName.compare( "VTKBaseRTP" ) )
	{
		return ObjBasePtr( new VTKBaseRTP( RTPOperation::Undefined, "" ) );
	}
	else if( !typeName.compare( "VTKContourSliceRTP" ) )
	{
		return ObjBasePtr( new VTKContourSliceRTP() );
	}
	else if( !typeName.compare( "VTKIsoSurfaceRTP" ) )
	{
		return ObjBasePtr( new VTKIsoSurfaceRTP() );
	}
	else if( !typeName.compare( "VTKPrimitiveSetGenerator" ) )
	{
		return ObjBasePtr( new VTKPrimitiveSetGenerator() );
	}
	else if( !typeName.compare( "VTKSurfaceRenderer" ) )
	{
		return ObjBasePtr( new VTKSurfaceRenderer() );
	}
	else if( !typeName.compare( "VTKSurfaceWrapRTP" ) )
	{
		return ObjBasePtr( new VTKSurfaceWrapRTP() );
	}
	else if( !typeName.compare( "VTKVectorFieldGlyphRTP" ) )
	{
		return ObjBasePtr( new VTKVectorFieldGlyphRTP() );
	}
	else if( !typeName.compare( "VTKVectorFieldRTP" ) )
	{
		return ObjBasePtr( new VTKVectorFieldRTP() );
	}
	else if( !typeName.compare( "VTKVectorRenderer" ) )
	{
		return ObjBasePtr( new VTKVectorRenderer() );
	}
	else if( !typeName.compare( "VTKVectorSliceRTP" ) )
	{
		return ObjBasePtr( new VTKVectorSliceRTP() );
	}

	return ObjBasePtr();
}

// vtk
}
// core
}
// lfx
}