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

#include <latticefx/core/HierarchyUtils.h>
#include <latticefx/core/ChannelData.h>
#include <latticefx/core/ChannelDataImageSet.h>
#include <latticefx/core/ChannelDataLOD.h>
#include <latticefx/core/LogMacros.h>

#include <boost/foreach.hpp>

#include <deque>
#include <sstream>


namespace lfx {
namespace core {


typedef std::deque< int > NameDeque;
static NameDeque s_name;

std::string dequeToString( const NameDeque& nameDeque )
{
    std::ostringstream ostr;
    BOOST_FOREACH( int idx, nameDeque )
    {
        ostr << idx;
    }
    return( ostr.str() );
}

void traverseHeirarchy( ChannelDataPtr cdp, HierarchyCallback& cb )
{
    ChannelDataImageSet* imageData( NULL );
    ChannelDataLOD* lodData( NULL );
    ChannelDataComposite* comp( dynamic_cast< ChannelDataComposite* >( cdp.get() ) );
    if( comp != NULL )
    {
        imageData = comp->getAsSet();
        lodData = comp->getAsLOD();
    }

    if( lodData != NULL )
    {
        for( unsigned int idx=0; idx<lodData->getNumChannels(); ++idx )
        {
            traverseHeirarchy( lodData->getChannel( idx ), cb );
        }
    }
    else if( imageData != NULL )
    {
        for( unsigned int idx=0; idx<imageData->getNumChannels(); ++idx )
        {
            s_name.push_back( idx );
            traverseHeirarchy( imageData->getChannel( idx ), cb );
            s_name.pop_back();
        }
    }
    else
    {
        cb( cdp, dequeToString( s_name ) );
    }
}


// core
}
// lfx
}
