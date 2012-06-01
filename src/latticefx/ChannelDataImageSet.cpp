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

#include <latticefx/ChannelDataImageSet.h>
#include <osg/Notify>
#include <osg/math>

#include <boost/foreach.hpp>


namespace lfx {


ChannelDataImageSet::ChannelDataImageSet( const std::string& name )
  : ChannelDataComposite( name )
{
}
ChannelDataImageSet::ChannelDataImageSet( const ChannelDataImageSet& rhs )
  : ChannelDataComposite( rhs )
{
}
ChannelDataImageSet::~ChannelDataImageSet()
{
}


// lfx
}
