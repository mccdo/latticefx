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

#include <latticefx/core/ChannelDataImageSet.h>
#include <latticefx/core/LogMacros.h>

#include <osg/Math>

#include <boost/foreach.hpp>


namespace lfx
{
namespace core
{


ChannelDataImageSet::ChannelDataImageSet( const std::string& name )
    : ChannelDataComposite( ChannelDataComposite::COMPOSITE_SET, name ),
      _offsets( new osg::Vec3Array )
{
}
ChannelDataImageSet::ChannelDataImageSet( const ChannelDataImageSet& rhs )
    : ChannelDataComposite( rhs ),
      _offsets( rhs._offsets )
{
}
ChannelDataImageSet::~ChannelDataImageSet()
{
}


void ChannelDataImageSet::removeChannel( const unsigned int index )
{
    if( index > _offsets->size() )
    {
        LFX_WARNING( "removeChannel: 'index' out of renag." );
        return;
    }
    for( unsigned int idx = index; idx < _offsets->size() - 1; ++idx )
    {
        ( *_offsets )[ idx ] = ( *_offsets )[ idx + 1 ];
    }
    _offsets->resize( _offsets->size() - 1 );

    ChannelDataComposite::removeChannel( index );
}
void ChannelDataImageSet::reserveChannels( const unsigned int count )
{
    ChannelDataComposite::reserveChannels( count );
    _offsets->resize( count );
}

void ChannelDataImageSet::setOffset( const unsigned int index, const osg::Vec3& value )
{
    if( index >= _offsets->size() )
    {
        _offsets->resize( index + 1 );
    }
    ( *_offsets )[ index ] = value;
}
osg::Vec3& ChannelDataImageSet::getOffset( const unsigned int index )
{
    return( ( *_offsets )[ index ] );
}
const osg::Vec3& ChannelDataImageSet::getOffset( const unsigned int index ) const
{
    return( ( *_offsets )[ index ] );
}


// Static
bool ChannelDataImageSet::allImageSetData( const ChannelDataList& data )
{
    BOOST_FOREACH( const ChannelDataPtr cdp, data )
    {
        if( dynamic_cast< ChannelDataImageSet* >( cdp.get() ) == NULL )
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
