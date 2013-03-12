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

#include <latticefx/core/ChannelDataLOD.h>

#include <osg/Math>

#include <boost/foreach.hpp>


namespace lfx
{
namespace core
{


ChannelDataLOD::ChannelDataLOD( const std::string& name )
    : ChannelDataComposite( ChannelDataComposite::COMPOSITE_LOD, name )
{
}
ChannelDataLOD::ChannelDataLOD( const ChannelDataLOD& rhs )
    : ChannelDataComposite( rhs ),
      _ranges( rhs._ranges )
{
}
ChannelDataLOD::~ChannelDataLOD()
{
}

void ChannelDataLOD::setRange( const unsigned int index, const RangeValues& value )
{
    if( index >= _ranges.size() )
    {
        _ranges.resize( index + 1 );
    }
    _ranges[ index ] = value;
}
RangeValues& ChannelDataLOD::getRange( const unsigned int index )
{
    return( _ranges[ index ] );
}
const RangeValues& ChannelDataLOD::getRange( const unsigned int index ) const
{
    return( _ranges[ index ] );
}


// Static
bool ChannelDataLOD::allLODData( const ChannelDataList& data )
{
    BOOST_FOREACH( const ChannelDataPtr cdp, data )
    {
        if( dynamic_cast< ChannelDataLOD* >( cdp.get() ) == NULL )
        {
            return( false );
        }
    }
    return( true );
}


// core
}
// lfx
}
