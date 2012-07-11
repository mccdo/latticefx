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

#include <latticefx/core/PluginManager.h>
#include <latticefx/core/OperationBase.h>
#include <latticefx/core/DataSet.h>
#include <latticefx/core/ChannelDataOSGImage.h>
#include <latticefx/core/Log.h>
#include <latticefx/core/LogMacros.h>

#include <osgDB/ReadFile>

#include <boost/shared_ptr.hpp>

#include <iostream>




lfx::DataSetPtr preprocess( const std::string& fileName )
{
    osg::Image* image( osgDB::readImageFile( fileName ) );
    if( image == NULL )
    {
        LFX_ERROR_STATIC( "lfx.demo", "Can't read image from file \"" + fileName + "\"." );
        return( lfx::DataSetPtr( ( lfx::DataSet* )NULL ) );
    }

    lfx::ChannelDataOSGImagePtr volumeData( new lfx::ChannelDataOSGImage( "volumedata", image ) );

    lfx::DataSetPtr dsp( new lfx::DataSet() );
    dsp->addChannel( volumeData );


    // Load Plugin to get the Downsample preprocess operation.
    lfx::PluginManager* plug( lfx::PluginManager::instance() );
    plug->loadConfigFiles();
    const std::string pluginName( "OSGVolume" );
    if( !( plug->loadPlugins( pluginName ) ) )
    {
        LFX_WARNING_STATIC( "lfx.demo", "Couldn't load \"OSGVolume\"." );
        return( lfx::DataSetPtr( (lfx::DataSet*)NULL ) );
    }

    // Create an instance of the Downsample preprocess operation.
    std::string opName( "Downsample" );
    lfx::OperationBasePtr op( plug->createOperation( pluginName, opName ) );
    if( op == NULL )
    {
        LFX_WARNING_STATIC( "lfx.demo", opName + ": createOperation() returned NULL." );
        return( lfx::DataSetPtr( (lfx::DataSet*)NULL ) );
    }


    op->addInput( "volumedata" );
    lfx::PreprocessPtr pre( boost::static_pointer_cast< lfx::Preprocess >( op ) );
    dsp->addPreprocess( pre );

    return( dsp );
}


int main( int argc, char** argv )
{
    lfx::Log::instance()->setPriority( lfx::Log::PrioInfo, lfx::Log::Console );

    osg::ArgumentParser arguments( &argc, argv );

    std::string fileName( "HeadVolume.dds" );
    arguments.read( "-f", fileName );

    lfx::DataSetPtr dsp( preprocess( fileName ) );
    dsp->updateAll();

    return( 0 );
}
