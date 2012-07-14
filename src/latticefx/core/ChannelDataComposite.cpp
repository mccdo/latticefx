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

#include <latticefx/core/ChannelDataComposite.h>
#include <latticefx/core/LogMacros.h>

#include <osg/math>

#include <boost/foreach.hpp>
#include <sstream>


namespace lfx {
namespace core {


ChannelDataComposite::ChannelDataComposite( const CompositeType compositeType, const std::string& name )
  : ChannelData( name ),
    _compositeType( compositeType )
{
}
ChannelDataComposite::ChannelDataComposite( const ChannelDataComposite& rhs )
  : ChannelData( rhs ),
    _compositeType( rhs._compositeType )
{
}
ChannelDataComposite::~ChannelDataComposite()
{
}

ChannelDataComposite::CompositeType ChannelDataComposite::getCompositeType() const
{
    return( _compositeType );
}

unsigned int ChannelDataComposite::addChannel( const ChannelDataPtr channel )
{
    unsigned int index( _data.size() );
    _data.push_back( channel );
    return( index );
}

unsigned int ChannelDataComposite::getNumChannels() const
{
    return( _data.size() );
}

ChannelDataPtr ChannelDataComposite::getChannel( const unsigned int index )
{
    if( index < _data.size() )
        return( _data[ index ] );

    if( LFX_LOG_WARNING )
    {
        std::ostringstream ostr;
        ostr << "Composite::getChannel(): Invalid index: " << index << ". _data.size()=" << _data.size();
        LFX_WARNING( ostr.str() );
    }
    return( ChannelDataPtr( ( ChannelData* )NULL ) );
}

const ChannelDataPtr ChannelDataComposite::getChannel( const unsigned int index ) const
{
    ChannelDataComposite* nonConstThis = const_cast< ChannelDataComposite* >( this );
    return( nonConstThis->getChannel( index ) );
}

void ChannelDataComposite::getDimensions( unsigned int& x, unsigned int& y, unsigned int& z )
{
    x = y = z = 0;
    BOOST_FOREACH( ChannelDataPtr cdp, _data )
    {
        unsigned int lx, ly, lz;
        cdp->getDimensions( lx, ly, lz );
        x = osg::maximum( x, lx );
        y = osg::maximum( y, ly );
        z = osg::maximum( z, lz );
    }
}


// core
}
// lfx
}
