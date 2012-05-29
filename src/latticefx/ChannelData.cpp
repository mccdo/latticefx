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

#include <latticefx/ChannelData.h>

#include <boost/foreach.hpp>


namespace lfx {


ChannelData::ChannelData( const std::string& name )
  : _name( name )
{
}
ChannelData::ChannelData( const ChannelData& rhs )
  : _name( rhs._name )
{
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



ChannelDataPtr findChannelData( const std::string& name, const ChannelDataList& dataList )
{
    BOOST_FOREACH( ChannelDataPtr cdp, dataList )
    {
        if( cdp->getName() == name )
            return( cdp );
    }
    return( ChannelDataPtr( ( ChannelData* )NULL ) );
}

void replaceChannelData( const ChannelDataPtr channel, ChannelDataList& dataList )
{
    const std::string name( channel->getName() );

    size_t index( 0 );
    while( index < dataList.size() )
    {
        if( dataList[ index ]->getName() == name )
        {
            dataList[ index ] = channel;
            return;
        }
        ++index;
    }

    // If we get this far, we never found a ChannelData with the same name,
    // so just tack the input onto the end of the list.
    dataList.push_back( channel );
}


// lfx
}
