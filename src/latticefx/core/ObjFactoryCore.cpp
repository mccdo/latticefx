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

#include <latticefx/core/ObjFactoryCore.h>
#include <latticefx/core/OperationBase.h>
#include <latticefx/core/Preprocess.h>
#include <latticefx/core/HierarchyUtils.h>
#include <latticefx/core/RTPOperation.h>
#include <latticefx/core/SurfaceRenderer.h>
#include <latticefx/core/VectorRenderer.h>
#include <latticefx/core/VolumeRenderer.h>
#include <latticefx/core/DataSet.h>

#include <sstream>

namespace lfx
{
namespace core
{

ObjFactoryCore::ObjFactoryCore( PluginManager *pm )
{
	_pm = pm;
}

ObjBasePtr ObjFactoryCore::createObj(const std::string &typeName, const ObjBase::KeyDataMap &map, std::string *perr)
{
	bool berr = false;
	ObjBasePtr ptr = createPlugin( map, &berr, perr );
	if( ptr ) return ptr;
	if( berr ) return ptr;

	// must not be a plugin so just create the operation type
	ptr = ObjFactoryCore::createStandardObj( typeName, perr );
	if( ptr ) return ptr;

	if (perr)
	{
		std::stringstream ss;
		ss << "Failed to create object with typename: " << typeName;
		*perr = ss.str();
	}

	return ObjBasePtr();
}

ObjBasePtr ObjFactoryCore::createPlugin( const ObjBase::KeyDataMap &map, bool *pbErr, std::string *perr )
{
	if( pbErr ) *pbErr = false;

	if( !_pm ) return ObjBasePtr();

	ObjBase::KeyDataMap::const_iterator itName = map.find( "pluginName" );
	ObjBase::KeyDataMap::const_iterator itClass = map.find( "pluginClassName" );
	if( itName == map.end() ||  itClass == map.end() )
	{
		return ObjBasePtr();
	}

	
	OperationBasePtr p = _pm->createOperation( itName->second, itClass->second );
	if(!p)
	{
		if( pbErr ) *pbErr = true;

		if (perr)
		{
			std::stringstream ss;
			ss << "Failed to create plugin object with name: " << itName->second << " and class: " << itClass->second;
			*perr = ss.str();
		}

		return ObjBasePtr();
	}

	return boost::static_pointer_cast<ObjBase>( p );
}

ObjBasePtr ObjFactoryCore::createStandardObj( const std::string &typeName, std::string *perr )
{
	if( !typeName.compare( "OperationBase" ) )
	{
		return ObjBasePtr( new OperationBase() );
	}
	else if( !typeName.compare( "Preprocess" ) )
	{
		return ObjBasePtr( new Preprocess() );
	}
	else if( !typeName.compare( "LoadHierarchy" ) )
	{
		return ObjBasePtr( new LoadHierarchy() );
	}
	else if( !typeName.compare( "RTPOperation" ) )
	{
		return ObjBasePtr( new RTPOperation() );
	}
	else if( !typeName.compare( "SurfaceRenderer" ) )
	{
		return ObjBasePtr( new SurfaceRenderer() );
	}
	else if( !typeName.compare( "VectorRenderer" ) )
	{
		return ObjBasePtr( new VectorRenderer() );
	}
	else if( !typeName.compare( "VolumeRenderer" ) )
	{
		return ObjBasePtr( new VolumeRenderer() );
	}
	else if( !typeName.compare( "DataSet" ) )
	{
		return ObjBasePtr( new DataSet() );
	}

	return ObjBasePtr();
}

// core
}
// lfx
}