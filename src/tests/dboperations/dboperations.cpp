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

#include <latticefx/core/DBOperations.h>
#include <latticefx/core/DataSet.h>
#include <latticefx/core/ChannelDataOSGArray.h>
#include <latticefx/core/ChannelDataOSGImage.h>
#include <latticefx/core/DBBase.h>
#include <latticefx/core/DBDisk.h>

#include <osg/Array>
#include <osg/Image>

#include <latticefx/core/Log.h>
#include <latticefx/core/LogMacros.h>

#include <Poco/Path.h>
#include <Poco/File.h>

static std::string logstr( "lfx.ctest.dbops" );


using namespace lfx::core;


bool performTests( DBBasePtr db )
{
    const std::string arraySuffix( ( db->getImplementationType() == DBBase::DISK ) ? ".osg" : "" );
    const std::string imageSuffix( ( db->getImplementationType() == DBBase::DISK ) ? ".ive" : "" );

    {
        const DBKey localKey( "floats-test" + arraySuffix );
        osg::ref_ptr< osg::FloatArray > floatsOrig( new osg::FloatArray() );
        for( unsigned int idx=0; idx<100; ++idx )
            floatsOrig->push_back( (float)idx );
        bool result = db->storeArray( floatsOrig.get(), localKey );
        if( !result )
        {
            LFX_CRITICAL_STATIC( logstr, "DBBase::storeArray() returned false for osg::FloatArray." );
            return( false );
        }

        osg::Array* newArray( db->loadArray( localKey ) );
        if( newArray == NULL )
        {
            LFX_CRITICAL_STATIC( logstr, "Loaded osg::FloatArray is NULL." );
            return( false );
        }
        osg::ref_ptr< osg::FloatArray > floatsLoad( static_cast< osg::FloatArray* >( newArray ) );

        if( floatsOrig->size() != floatsLoad->size() )
        {
            LFX_CRITICAL_STATIC( logstr, "osg::FloatArray size mismatch." );
            return( false );
        }
        for( unsigned int idx=0; idx<floatsOrig->size(); ++idx )
        {
            if( (*floatsOrig)[ idx ] != (*floatsLoad)[ idx ] )
            {
                LFX_CRITICAL_STATIC( logstr, "osg::FloatArray element mismatch." );
                return( false );
            }
        }
    }

    /*
    NameStringGenerator nsg( osg::Vec3s( 128, 128, 128 ) );

    std::string result( nsg.getNameString( osg::Vec3s( 128, 128, 128 ), osg::Vec3s( 0, 0, 0 ) ) );
    std::string correct( "" );
    if( result != correct )
    {
        LFX_ERROR_STATIC( logstr, "Failure: Root node has non-null name string: " + result );
        return( 1 );
    }
    */

    return( true );
}

int main( int argc, char** argv )
{
    Log::instance()->setPriority( Log::PrioInfo, Log::Console );

    LFX_CRITICAL_STATIC( logstr, "This is a CTest regression test. To launch under Visual Studio, build the" );
    LFX_CRITICAL_STATIC( logstr, "RUN_TESTS target. Under Linux, enter 'make test' at a shell prompt.\n" );


    // Create a DB.
    Poco::Path tempDBPath( "db" );
    tempDBPath.makeDirectory();
    Poco::File tempDBFile( tempDBPath );
    try
    {
        tempDBFile.createDirectories();
    }
    catch( Poco::Exception& e )
    {
        LFX_CRITICAL_STATIC( logstr, e.displayText() );
        return( 1 );
    }
    DBBasePtr db( new DBDisk( tempDBPath.toString() ) );
    LFX_INFO_STATIC( logstr, "Testing with DBDisk.\n" );


    if( !performTests( db ) )
        return( 1 );


    // Delete the DB.
    //tempDBFile.remove( true );


    LFX_CRITICAL_STATIC( logstr, "Pass." );
    return( 0 );
}
