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

#include <latticefx/core/DBUtils.h>

#include <osg/Node>

#ifdef DB_IMPL_FILESYSTEM
#  include <osgDB/ReadFile>
#  include <osgDB/WriteFile>
#  include <iomanip>
#  include <sstream>
#else
#endif


namespace lfx {
namespace core {


DBKey generateDBKey()
{
    static unsigned int fileNameCounter( 0 );

    std::ostringstream ostr;
    ostr << "pagefile" << std::setfill( '0' ) <<
        std::setw( 5 ) << fileNameCounter++ << ".ive";
    return( DBKey( ostr.str() ) );
}

bool storeImage( const osg::Image* image, const DBKey& dbKey )
{
    return( osgDB::writeImageFile( *image, dbKey ) );
}
osg::Image* loadImage( const DBKey& dbKey )
{
    return( osgDB::readImageFile( dbKey ) );
}


// core
}
// lfx
}
