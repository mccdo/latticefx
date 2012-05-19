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

#include <latticefx/ChannelDataComposite.h>
#include <osg/Notify>
#include <osg/math>

#include <boost/foreach.hpp>


namespace lfx {


ChannelDataComposite::ChannelDataComposite( const std::string& name )
  : ChannelData( name )
{
}
ChannelDataComposite::ChannelDataComposite( const ChannelDataComposite& rhs )
  : ChannelData( rhs )
{
}
ChannelDataComposite::~ChannelDataComposite()
{
}

void ChannelDataComposite::addChannel( const ChannelDataPtr channel, const unsigned int level )
{
    OSG_WARN << "Composite LOD not yet supported." << std::endl;
}

ChannelDataPtr ChannelDataComposite::getChannel( const unsigned int level )
{
    OSG_WARN << "Composite LOD not yet supported." << std::endl;
    return( ChannelDataPtr( ( ChannelData* )NULL ) );
}

const ChannelDataPtr ChannelDataComposite::getChannel( const unsigned int level ) const
{
    ChannelDataComposite* nonConstThis = const_cast< ChannelDataComposite* >( this );
    return( nonConstThis->getChannel( level ) );
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


// lfx
}
