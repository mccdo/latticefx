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

#ifndef __LFX_CORE_OCTREE_GROUP_H__
#define __LFX_CORE_OCTREE_GROUP_H__ 1


#include <latticefx/core/Export.h>

#include <osg/Group>
#include <osg/Array>



namespace lfx
{
namespace core
{


/** \class OctreeGroup OctreeGroup.h <latticefx/core/OctreeGroup.h>
\brief TBD
\details TBD
*/
class LATTICEFX_EXPORT OctreeGroup : public osg::Group
{
public:
    OctreeGroup( const osg::Vec3 center = osg::Vec3( 0., 0., 0. ) );
    OctreeGroup( const OctreeGroup& rhs, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY );
    virtual ~OctreeGroup();

    META_Node( latticefx_core, OctreeGroup );


    virtual void traverse( osg::NodeVisitor& nv );

    using osg::Group::addChild;

    virtual bool addChild( Node* child, const osg::Vec3& offset );

    /** \brief Convenience accessor for dot OSG support. */
    void setOffsets( osg::Vec3Array* offsets )
    {
        _offsets = offsets;
    }
    /** \brief Convenience accessor for dot OSG support. */
    const osg::Vec3Array* getOffsets() const
    {
        return( _offsets.get() );
    }
    /** \brief Convenience accessor for dot OSG support. */
    void setCenter( const osg::Vec3& center )
    {
        _center = center;
    }
    /** \brief Convenience accessor for dot OSG support. */
    const osg::Vec3& getCenter() const
    {
        return( _center );
    }

protected:
    osg::ref_ptr< osg::Vec3Array > _offsets;
    osg::Vec3 _center;
};


// core
}
// lfx
}


// __LFX_CORE_OCTREE_GROUP_H__
#endif
