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

#include <latticefx/core/DBDisk.h>
#include <latticefx/core/LogMacros.h>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include <Poco/Path.h>
#include <Poco/File.h>
#include <Poco/Glob.h>

#include <iostream>

namespace lfx {
namespace core {


DBDisk::DBDisk( const std::string rootPath )
  : DBBase( DISK ),
    _rootPath( rootPath )
{
}
DBDisk::DBDisk( const DBDisk& rhs )
  : DBBase( rhs ),
    _rootPath( rhs._rootPath )
{
}
DBDisk::~DBDisk()
{
}


DBKey DBDisk::generateDBKey()
{
    DBKey key( DBBase::generateDBKey() );
    key += ".ive";
    return( key );
}
DBKey DBDisk::generateDBKey( const std::string& baseName, const TimeValue time )
{
    DBKey key( DBBase::generateDBKey( baseName, time ) );
    key += ".ive";
    return( key );
}


void DBDisk::setRootPath( const std::string& rootPath )
{
    _rootPath = rootPath;
}
const std::string& DBDisk::getRootPath() const
{
    return( _rootPath );
}


bool DBDisk::storeImage( const osg::Image* image, const DBKey& dbKey )
{
    std::string fileName( fileNameFromDBKey( dbKey ) );
    return( osgDB::writeImageFile( *image, fileName ) );
}
osg::Image* DBDisk::loadImage( const DBKey& dbKey )
{
    std::string fileName( fileNameFromDBKey( dbKey ) );
    osg::Image* image( osgDB::readImageFile( fileName ) );
    if( image == NULL )
    {
        LFX_ERROR( "Can't load file " + fileName );
    }

    if( image->getFileName().empty() )
        // Required for paging, in case the image load doesn't set it.
        image->setFileName( dbKey );

    return( image );
}

bool DBDisk::storeArray( const osg::Array* array, const DBKey& dbKey )
{
    std::string fileName( fileNameFromDBKey( dbKey ) );
    return( osgDB::writeObjectFile( *array, fileName ) );
}
osg::Array* DBDisk::loadArray( const DBKey& dbKey )
{
    std::string fileName( fileNameFromDBKey( dbKey ) );
    return( dynamic_cast< osg::Array* >( osgDB::readObjectFile( fileName ) ) );
}


DBBase::StringSet DBDisk::getAllKeys() const
{
    const std::string globPattern( fileNameFromDBKey( DBKey( "*" ) ) );
    Poco::Path globPath( globPattern );
    DBBase::StringSet keys;
    Poco::Glob::glob( globPath, keys );

    return( keys );
}


std::string DBDisk::fileNameFromDBKey( const DBKey& dbKey ) const
{
    const std::string keyName( dbKey );
    if( _rootPath.empty() )
        return( std::string( keyName ) );

    // Let Poco combine the _rootPath with the dbKey.
    // Automatically handles things like path separators,
    // whether dbKey is a fileName or relative path, etc.
    Poco::Path pocoPath( _rootPath );
    pocoPath.makeDirectory();
    Poco::Path fileName( keyName );
    pocoPath.resolve( fileName );

    // Create the directory, if it doesn't exist.
    Poco::Path pathOnly( pocoPath );
    pathOnly.makeParent();
    if( !pathOnly.toString().empty() )
    {
        Poco::File pocoFile( pathOnly );
        try
        {
            pocoFile.createDirectories();
        }
        catch( Poco::Exception& e )
        {
            std::cout << e.displayText() << std::endl;
        }
    }
    return( pocoPath.toString() );
}


// core
}
// lfx
}
