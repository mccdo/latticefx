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


osg::FloatArray* genFloatArray( const int n )
{
    osg::ref_ptr< osg::FloatArray > floatsOrig( new osg::FloatArray() );
    for( unsigned int idx = 0; idx < 100; ++idx )
    {
        floatsOrig->push_back( ( float )idx );
    }
    return( floatsOrig.release() );
}

bool arrayCompare( const osg::FloatArray* a, const osg::FloatArray* b )
{
    if( a->size() != b->size() )
    {
        LFX_CRITICAL_STATIC( logstr, "osg::FloatArray size mismatch." );
        return( false );
    }
    for( unsigned int idx = 0; idx < a->size(); ++idx )
    {
        if( ( *a )[ idx ] != ( *b )[ idx ] )
        {
            LFX_CRITICAL_STATIC( logstr, "osg::FloatArray element mismatch." );
            return( false );
        }
    }
    return( true );
}


bool performTests( DBBasePtr db )
{
    const std::string keySuffix( ( db->getImplementationType() == DBBase::DISK ) ? ".ive" : "" );

    {
        LFX_CRITICAL_STATIC( logstr, "Testing simple array save/load to DB." );

        const DBKey localKey( "floats-test" + keySuffix );
        osg::ref_ptr< osg::FloatArray > floatsOrig( genFloatArray( 100 ) );
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

        bool pass( arrayCompare( floatsOrig.get(), floatsLoad.get() ) );
        if( !pass )
            return( false );
    }

    {
        LFX_CRITICAL_STATIC( logstr, "Testing DBSave FloatArray ChannelData." );


        // Stick the array in a ChannelData. Set up a DataSet pipe with a DBSave
        // operation to write the ChannelData to DB.
        osg::ref_ptr< osg::FloatArray > floatsOrig( genFloatArray( 100 ) );
        ChannelDataOSGArrayPtr cdArray( new ChannelDataOSGArray( "floatChannel", floatsOrig.get() ) );
        const DBKey localKey( "floats-dbsave-test" + keySuffix );
        cdArray->setDBKey( localKey );

        {
            DataSetPtr dsp( new DataSet() );
            dsp->addChannel( cdArray );

            DBSavePtr dbSave( new DBSave( db ) );
            dbSave->addInput( "floatChannel" );
            dsp->addOperation( dbSave );

            dsp->updateAll();
        }


        // Do a simple retrieve from the DB and do a match test.
        osg::Array* newArray( db->loadArray( localKey ) );
        if( newArray == NULL )
        {
            LFX_CRITICAL_STATIC( logstr, "Loaded osg::FloatArray is NULL." );
            return( false );
        }
        osg::ref_ptr< osg::FloatArray > floatsLoad( static_cast< osg::FloatArray* >( newArray ) );

        bool pass( arrayCompare( floatsOrig.get(), floatsLoad.get() ) );
        if( !pass )
            return( false );


        LFX_CRITICAL_STATIC( logstr, "Testing DBLoad FloatArray ChannelData." );


        // Create a DataSet with a DBLoad to load the array into a ChannelData.
        ChannelDataPtr loadedChannel;
        {
            DataSetPtr dsp( new DataSet() );

            const std::string channelName( "loadedChannel" );
            DBLoadPtr dbLoad( new DBLoad( db, localKey, channelName ) );
            dsp->addPreprocess( dbLoad );

            dsp->updateAll();

            loadedChannel = dsp->getChannel( channelName );
            if( loadedChannel == NULL )
            {
                LFX_CRITICAL_STATIC( logstr, "DataSet::getChannel() returned NULL." );
                return( false );
            }
        }

        // Compare the loaded array with the original array.
        ChannelDataOSGArrayPtr cdap( boost::dynamic_pointer_cast< ChannelDataOSGArray >( loadedChannel ) );
        if( cdap == NULL )
        {
            LFX_CRITICAL_STATIC( logstr, "Loaded Channel is not a ChannelDataOSGArray." );
            return( false );
        }
        osg::FloatArray* loadedArray( dynamic_cast< osg::FloatArray* >( cdap->asOSGArray() ) );
        if( loadedArray == NULL )
        {
            LFX_CRITICAL_STATIC( logstr, "ChannelDataOSGArray does not contain a FloatArray." );
            return( false );
        }

        pass = arrayCompare( floatsOrig.get(), loadedArray );
        if( !pass )
            return( false );
    }



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
    {
        return( 1 );
    }


    // Delete the DB. If any tests failed, we return before
    // executing this line, so DBDisk file contents can be
    // examined during failure analysis if desired.
    tempDBFile.remove( true );


    LFX_CRITICAL_STATIC( logstr, "\nPass." );
    return( 0 );
}
