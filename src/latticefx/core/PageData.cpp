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

#include <latticefx/core/PageData.h>

#include <iostream>


namespace lfx {
namespace core {


PageData::PageData( const RangeMode rangeMode )
  : osg::Object(),
    _rangeMode( rangeMode ),
    _parent( NULL )
{
}
PageData::PageData( const PageData& rhs, const osg::CopyOp& copyOp )
  : osg::Object( rhs, copyOp ),
    _rangeMode( rhs._rangeMode ),
    _rangeDataMap( rhs._rangeDataMap ),
    _parent( rhs._parent )
{
}
PageData::~PageData()
{
}

void PageData::setRangeMode( const RangeMode rangeMode )
{
    _rangeMode = rangeMode;
}
PageData::RangeMode PageData::getRangeMode() const
{
    return( _rangeMode );
}

void PageData::setMinMaxTime( const TimeValue minTime, const TimeValue maxTime )
{
    _minTime = minTime;
    _maxTime = maxTime;
}
void PageData::getMinMaxTime( TimeValue& minTime, TimeValue& maxTime )
{
    minTime = _minTime;
    maxTime = _maxTime;
}


void PageData::setRangeData( const unsigned int childIndex, const RangeData& rangeData )
{
    _rangeDataMap[ childIndex ] = rangeData;
}
PageData::RangeData* PageData::getRangeData( const unsigned int childIndex )
{
    RangeDataMap::iterator it( _rangeDataMap.find( childIndex ) );
    if( it != _rangeDataMap.end() )
        return( &( it->second ) );
    return( NULL );
}
const PageData::RangeData* PageData::getRangeData( const unsigned int childIndex ) const
{
    RangeDataMap::const_iterator it( _rangeDataMap.find( childIndex ) );
    if( it != _rangeDataMap.end() )
        return( &( it->second ) );
    return( NULL );
}
PageData::RangeDataMap& PageData::getRangeDataMap()
{
    return( _rangeDataMap );
}

void PageData::setParent( osg::Group* parent )
{
    _parent = parent;
}
osg::Group* PageData::getParent()
{
    return( _parent.get() );
}



PageData::RangeData::RangeData()
  : _rangeValues( RangeValues( 0., FLT_MAX ) ),
    _status( UNLOADED )
{
}
PageData::RangeData::RangeData( RangeValues rangeValues, const DBKey& dbKey )
  : _rangeValues( rangeValues ),
    _status( UNLOADED )
{
}
PageData::RangeData::RangeData( double minVal, double maxVal, const DBKey& dbKey )
  : _rangeValues( RangeValues( minVal, maxVal ) ),
    _status( UNLOADED )
{
}



// core
}
// lfx
}
