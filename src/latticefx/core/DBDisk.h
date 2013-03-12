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

#ifndef __LFX_CORE_DB_DISK_H__
#define __LFX_CORE_DB_DISK_H__ 1

#include <latticefx/core/Export.h>
#include <latticefx/core/DBBase.h>
#include <latticefx/core/types.h>

#include <boost/shared_ptr.hpp>


namespace lfx
{
namespace core
{


/** \class DBDisk DBDisk.h <latticefx/core/DBDisk.h>
\brief A DBBase-derived class that supports OSG Image and Array storage using OSG native .ive files.
\details This class essentially uses a directory in the filesystem as a database,
with each file in the directory as a potential data item. Applications should specify
a directory using setRootPath(). Note that getAllKeys() simply returns a list of files
in that directory, so if the directory contains anything other than files intended for
database storage, potential problems could result. For this reason, when creating a
new database, application developers are strongly encouraged to specify an empty
directory in the call to setRootPath(), and to not disturb the the contents of that
directory by manually adding or removing files from it. */
class LATTICEFX_EXPORT DBDisk : public DBBase
{
public:
    DBDisk( const std::string rootPath = std::string( "" ) );
    DBDisk( const DBDisk& rhs );
    ~DBDisk();

    virtual DBKey generateDBKey();
    virtual DBKey generateDBKey( const std::string& baseName, const TimeValue time = ( TimeValue )0. );

    /** If not empty, this is prepended to dbKey.
    Allows DBDisk to read/write files from/to a specific parent path. */
    void setRootPath( const std::string& rootPath );
    const std::string& getRootPath() const;

    virtual bool storeImage( const osg::Image* image, const DBKey& dbKey );
    virtual osg::Image* loadImage( const DBKey& dbKey );

    virtual bool storeArray( const osg::Array* array, const DBKey& dbKey );
    virtual osg::Array* loadArray( const DBKey& dbKey );


    virtual StringSet getAllKeys() const;

protected:
    std::string fileNameFromDBKey( const DBKey& dbKey ) const;

    std::string _rootPath;
};

typedef boost::shared_ptr< DBDisk > DBDiskPtr;


// core
}
// lfx
}


// __LFX_CORE_DB_BASE_H__
#endif
