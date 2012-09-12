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

#include <latticefx/core/OctreeGroup.h>
#include <latticefx/core/LogMacros.h>

#include <osgUtil/CullVisitor>
#include <osg/Matrix>

#include <boost/foreach.hpp>

#include <set>


namespace lfx {
namespace core {


OctreeGroup::OctreeGroup( const osg::Vec3 center )
  : Group(),
    _center( center )
{
}
OctreeGroup::OctreeGroup( const OctreeGroup& rhs, const osg::CopyOp& copyop )
  : Group( rhs, copyop ),
    _offsets( dynamic_cast< osg::Vec3Array* >( copyop( rhs._offsets.get() ) ) ),
    _center( rhs._center )
{
}
OctreeGroup::~OctreeGroup()
{
}


struct DotSortedChild
{
    DotSortedChild( const float sortValue, const unsigned int index )
      : _sortValue( sortValue ), _index( index )
    {
    }
    DotSortedChild( const DotSortedChild& rhs )
      : _sortValue( rhs._sortValue ), _index( rhs._index )
    {
    }

    bool operator<( const DotSortedChild& rhs ) const
    {
        return( _sortValue < rhs._sortValue );
    }

    float _sortValue;
    unsigned int _index;
};

void OctreeGroup::traverse( osg::NodeVisitor& nv )
{
    if( nv.getVisitorType() != osg::NodeVisitor::CULL_VISITOR )
    {
        Group::traverse( nv );
        return;
    }

#if 0
    // TBD sort children and traverse in back-to-front order.
    // For now, just act like a Group:
    Group::traverse( nv );
    return;
#else

    // Convert all offsets into eye coords. This is done by multiplying the
    // offsets by the normal matrix.
    osgUtil::CullVisitor* cv( static_cast< osgUtil::CullVisitor* >( &nv ) );
    osg::Matrix mv( *( cv->getModelViewMatrix() ) );
    osg::Matrix mvOrient( mv );
    mvOrient.setTrans( 0., 0., 0. );
    osg::Matrix mvInv;
    mvInv.invert( mvOrient );
    // The normal matrix is the inverse transpose of the modelview matrix.
    // However, we can avoid the transpose if we use post-multiply when
    // converting the offset into eye coords.

    // _sortedChildren will sort child indices from furthest to
    // nearest based on the dot product of the eye coord offset and
    // the negation of the eye coord center position.
    typedef std::multiset< DotSortedChild > SortedChildSet;
    SortedChildSet _sortedChildren;
    const osg::Vec3 ecNegCenter = -( _center * mv );

    for( unsigned int idx=0; idx<_offsets->size(); ++idx )
    {
        osg::Vec3 ecVec( mvInv * (*_offsets)[ idx ] ); // Post-multiply because mvInv was not transposed.
        const float dot( ecVec * ecNegCenter );
        const DotSortedChild childInfo( dot, idx );
        _sortedChildren.insert( childInfo );
    }

    BOOST_FOREACH( DotSortedChild childInfo, _sortedChildren )
    {
        getChild( childInfo._index )->accept( nv );
    }

#endif
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



#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>
#include <osgDB/ParameterOutput>
#include <osgDB/ParameterOutput>
#include <osg/io_utils>


/** \addtogroup DotOSGSupport */
/** \{ */


bool OctreeGroup_readLocalData( osg::Object& obj, osgDB::Input& fr );
bool OctreeGroup_writeLocalData( const osg::Object& obj, osgDB::Output& fw );

osgDB::RegisterDotOsgWrapperProxy OctreeGroup_Proxy
(
    new lfx::core::OctreeGroup,
    "OctreeGroup",
    "Object Node OctreeGroup Group",
    OctreeGroup_readLocalData,
    OctreeGroup_writeLocalData
);

bool OctreeGroup_readLocalData( osg::Object& obj, osgDB::Input& fr )
{
    lfx::core::OctreeGroup& og( static_cast< lfx::core::OctreeGroup& >( obj ) );
    bool advance( false );

    if( fr[ 0 ].getStr() == std::string( "OffsetArray" ) )
    {
        const int entry( fr[0].getNoNestedBrackets() );

        unsigned int size;
        fr[ 1 ].getUInt( size );
        fr += 3; // Dkip "OffsetArray", size, and opening brace.

        osg::ref_ptr< osg::Vec3Array > offsets( new osg::Vec3Array );
        offsets->reserve( size );
        while( !fr.eof() && ( fr[0].getNoNestedBrackets() > entry ) )
        {
            osg::Vec3 v;
            if( fr[0].getFloat( v.x() ) && fr[1].getFloat( v.y() ) && fr[2].getFloat( v.z() ) )
            {
                fr += 3;
                offsets->push_back( v );
            }
            else ++fr;
        }
        og.setOffsets( offsets.get() );
        ++fr; // For closing brace.

        advance = true;
    }
    if( fr[ 0 ].getStr() == std::string( "Center" ) )
    {
        ++fr;
        osg::Vec3 v;
        if( fr[0].getFloat( v.x() ) && fr[1].getFloat( v.y() ) && fr[2].getFloat( v.z() ) )
        {
            fr += 3;
            og.setCenter( v );
            advance = true;
        }
    }
    
    return( advance );
}

bool OctreeGroup_writeLocalData( const osg::Object& obj, osgDB::Output& fw )
{
    const lfx::core::OctreeGroup& og( static_cast< const lfx::core::OctreeGroup& >( obj ) );

    const osg::Vec3Array* oArray( og.getOffsets() );
    fw.indent() << "OffsetArray " << oArray->size() << std::endl;
    osgDB::writeArray( fw, oArray->begin(), oArray->end(), 1 );

    fw.indent() << "Center " << og.getCenter() << std::endl;

    return( true );
}

/** \} */
