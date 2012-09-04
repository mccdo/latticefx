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

#include <latticefx/core/LoadRequest.h>
#include <latticefx/core/DBUtils.h>
#include <boost/foreach.hpp>

#include <iostream>


namespace lfx {
namespace core {


LoadRequest::LoadRequest()
  : _path(),
    _keys()
{
}
LoadRequest::LoadRequest( const osg::NodePath& path, const DBKeyList& keys )
  : _path( path ),
    _keys( keys )
{
}
LoadRequest::LoadRequest( const LoadRequest& rhs )
  : _path( rhs._path ),
    _keys( rhs._keys )
{
}
LoadRequest::~LoadRequest()
{
}

osg::Object* LoadRequest::find( const DBKey& dbKey )
{
    ResultsMap::iterator it( _results.find( dbKey ) );
    if( it != _results.end() )
        return( it->second.get() );
    else
        return( NULL );
}


bool LoadRequestImage::load()
{
    bool result( true );
    BOOST_FOREACH( const DBKey key, _keys )
    {
        osg::Image* image( lfx::core::loadImage( key ) );
        if( image == NULL )
            result = false;
        _results[ key ] = image;
    }
    return( result );
}
osg::Image* LoadRequestImage::findAsImage( const DBKey& dbKey )
{
    return( static_cast< osg::Image* >( find( dbKey ) ) );
}


// core
}
// lfx
}
