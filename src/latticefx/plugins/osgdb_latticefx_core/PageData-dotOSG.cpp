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

#include <latticefx/core/PageData.h>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>
#include <osgDB/ParameterOutput>
#include <osg/io_utils>


/** \addtogroup DotOSGSupport */
/** \{ */


bool PageData_readLocalData( osg::Object& obj, osgDB::Input& fr );
bool PageData_writeLocalData( const osg::Object& obj, osgDB::Output& fw );

osgDB::RegisterDotOsgWrapperProxy PageData_Proxy
(
    new lfx::core::PageData,
    "PageData",
    "Object PageData",
    PageData_readLocalData,
    PageData_writeLocalData
);

bool PageData_readLocalData( osg::Object& obj, osgDB::Input& fr )
{
    lfx::core::PageData& pd( static_cast< lfx::core::PageData& >( obj ) );
    bool advance( false );

    // TBD Not yet implemented.

    return( advance );
}

bool PageData_writeLocalData( const osg::Object& obj, osgDB::Output& fw )
{
    const lfx::core::PageData& pd( static_cast< const lfx::core::PageData& >( obj ) );

    // TBD Not yet implemented.

    return( true );
}

/** \} */
