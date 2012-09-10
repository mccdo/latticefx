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

#include <latticefx/core/OctreeGroup.h>
#include <latticefx/core/LogMacros.h>

#include <boost/foreach.hpp>


namespace lfx {
namespace core {


OctreeGroup::OctreeGroup()
  : Group()
{
}
OctreeGroup::OctreeGroup( const OctreeGroup& rhs, const osg::CopyOp& copyop )
  : Group( rhs, copyop ),
    _offsets( dynamic_cast< osg::Vec3Array* >( copyop( rhs._offsets.get() ) ) )
{
}
OctreeGroup::~OctreeGroup()
{
}


void OctreeGroup::traverse( osg::NodeVisitor& nv )
{
    if( nv.getVisitorType() != osg::NodeVisitor::CULL_VISITOR )
    {
        Group::traverse( nv );
        return;
    }

    // TBD sort children and traverse in back-to-front order.
    // For now, just act like a Group:
    Group::traverse( nv );
    return;
}

bool OctreeGroup::addChild( Node *child, const osg::Vec3& offset )
{
    if( Group::addChild( child ) )
    {
        const unsigned int childIndex( getChildIndex( child ) );
        if( _offsets == NULL )
            _offsets = new osg::Vec3Array;
        if( _offsets->size() <= childIndex )
            _offsets->resize( childIndex+1 );
        (*_offsets)[ childIndex ] = offset;
        return( true );
    }
    else
        return( false );
}


// core
}
// lfx
}
