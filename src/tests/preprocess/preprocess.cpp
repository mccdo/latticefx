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

#include <latticefx/core/PluginManager.h>
#include <latticefx/core/OperationBase.h>
#include <latticefx/core/DataSet.h>
#include <latticefx/core/ChannelDataOSGImage.h>
#include <latticefx/core/Log.h>
#include <latticefx/core/LogMacros.h>

#include <osgDB/ReadFile>

#include <boost/shared_ptr.hpp>

#include <iostream>


using namespace lfx::core;


DataSetPtr preprocess( const std::string& fileName )
{
    osg::Image* image( osgDB::readImageFile( fileName ) );
    if( image == NULL )
    {
        LFX_ERROR_STATIC( "lfx.demo", "Can't read image from file \"" + fileName + "\"." );
        return( DataSetPtr( ( DataSet* )NULL ) );
    }

    ChannelDataOSGImagePtr volumeData( new ChannelDataOSGImage( "volumedata", image ) );

    DataSetPtr dsp( new DataSet() );
    dsp->addChannel( volumeData );


    // Load Plugin to get the Downsample preprocess operation.
    PluginManager* plug( PluginManager::instance() );
    plug->loadConfigFiles();
    const std::string pluginName( "OSGVolume" );
    if( !( plug->loadPlugins( pluginName ) ) )
    {
        LFX_WARNING_STATIC( "lfx.demo", "Couldn't load \"OSGVolume\"." );
        return( DataSetPtr( (DataSet*)NULL ) );
    }

    // Create an instance of the Downsample preprocess operation.
    std::string opName( "Downsample" );
    OperationBasePtr op( plug->createOperation( pluginName, opName ) );
    if( op == NULL )
    {
        LFX_WARNING_STATIC( "lfx.demo", opName + ": createOperation() returned NULL." );
        return( DataSetPtr( (DataSet*)NULL ) );
    }


    op->addInput( "volumedata" );
    PreprocessPtr pre( boost::static_pointer_cast< Preprocess >( op ) );
    dsp->addPreprocess( pre );

    return( dsp );
}


int main( int argc, char** argv )
{
    Log::instance()->setPriority( Log::PrioInfo, Log::Console );

    osg::ArgumentParser arguments( &argc, argv );

    std::string fileName( "HeadVolume.dds" );
    arguments.read( "-f", fileName );

    DataSetPtr dsp( preprocess( fileName ) );
    dsp->updateAll();

    return( 0 );
}
