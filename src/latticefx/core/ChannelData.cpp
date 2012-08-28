/*************** <auto-copyright.rb BEGIN do not edit this line> *************
*
* latticeFX is (C) Copyright 2011-2012 by Ames Laboratory
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Library General Public
* License as published by the Free Software Foundation; 
* version 2.1 of the License.
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
* -----------------------------------------------------------------
* Date modified: $Date$
* Version:       $Rev$
* Author:        $Author$
* Id:            $Id$
* -----------------------------------------------------------------
*
*************** <auto-copyright.rb END do not edit this line> **************/

#include <latticefx/core/ChannelData.h>
#include <latticefx/core/DBUtils.h>
#include <latticefx/core/LogMacros.h>

#include <boost/foreach.hpp>

#include <sstream>


namespace lfx {
namespace core {


ChannelData::ChannelData( const std::string& name )
  : LogBase( "lfx.core.channel" ),
    _name( name ),
    _storageMode( STORE_IN_RAM ),
    _dbKey( "" )
{
    memset( _dimensions, 0, sizeof( _dimensions ) );
}
ChannelData::ChannelData( const ChannelData& rhs )
  : LogBase( rhs ),
    _name( rhs._name ),
    _storageMode( rhs._storageMode ),
    _dbKey( rhs._dbKey )
{
    memcpy( _dimensions, rhs._dimensions, sizeof( _dimensions ) );
}
ChannelData::~ChannelData()
{
}


void ChannelData::setName( const std::string& name )
{
    _name = name;
}
const std::string& ChannelData::getName() const
{
    return( _name );
}

void ChannelData::setStorageModeHint( const StorageModeHint& storageMode )
{
    _storageMode = storageMode;
}
ChannelData::StorageModeHint ChannelData::getStorageModeHint() const
{
    return( _storageMode );
}

void ChannelData::setDBKey( const DBKey dbKey )
{
    _dbKey = dbKey;
}
DBKey ChannelData::getDBKey() const
{
    if( _dbKey.empty() )
    {
    }
    return( _dbKey );
}

void ChannelData::getDimensions( unsigned int& x, unsigned int& y, unsigned int& z )
{
    x = _dimensions[ 0 ];
    y = _dimensions[ 1 ];
    z = _dimensions[ 2 ];
}
void ChannelData::setDimensions( const unsigned int x, const unsigned int y, const unsigned int z )
{
    _dimensions[ 0 ] = x;
    _dimensions[ 1 ] = y;
    _dimensions[ 2 ] = z;
}


ChannelDataList::ChannelDataList()
  : ChannelDataListBase()
{
}
ChannelDataList::ChannelDataList( const ChannelDataList& rhs )
  : ChannelDataListBase( rhs )
{
}
ChannelDataList::~ChannelDataList()
{
}

ChannelDataPtr ChannelDataList::findData( const std::string& name )
{
    BOOST_FOREACH( ChannelDataPtr cdp, *this )
    {
        if( cdp->getName() == name )
            return( cdp );
    }
    return( ChannelDataPtr( ( ChannelData* )NULL ) );
}

void ChannelDataList::replaceData( const ChannelDataPtr channel )
{
    const std::string name( channel->getName() );

    size_t index( 0 );
    while( index < size() )
    {
        if( (*this)[ index ]->getName() == name )
        {
            (*this)[ index ] = channel;
            return;
        }
        ++index;
    }

    // If we get this far, we never found a ChannelData with the same name,
    // so just tack the input onto the end of the list.
    push_back( channel );
}


// core
}
// lfx
}
