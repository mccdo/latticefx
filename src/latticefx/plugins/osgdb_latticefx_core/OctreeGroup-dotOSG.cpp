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

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>
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
            else
            {
                ++fr;
            }
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
