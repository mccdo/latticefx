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
#include <latticefx/core/ObjFactoryCore.h>
#include <latticefx/core/Log.h>
#include <latticefx/core/LogMacros.h>

#include <iostream>
#include <sstream>


using namespace lfx::core;


const std::string logstr( "lfx.ctest.multiop" );


int main()
{
    Log::instance()->setPriority( Log::PrioInfo, Log::Console );

    LFX_CRITICAL_STATIC( logstr, "This is a CTest regression test. To launch under Visual Studio, build the" );
    LFX_CRITICAL_STATIC( logstr, "RUN_TESTS target. Under Linux, enter 'make test' at a shell prompt.\n" );


    // Add additional plugin search paths.
    PluginManager* plug( PluginManager::instance() );
    if( plug == NULL )
    {
        LFX_ERROR_STATIC( logstr, "Failure: NULL PluginManager." );
        return( 1 );
    }
    plug->loadConfigFiles();

    // Load all plugins named "MultipleOperationsPluginTest".
    const std::string pluginName( "MultipleOperationsPluginTest" );
    if( !( plug->loadPlugins( pluginName ) ) )
    {
        LFX_CRITICAL_STATIC( logstr, "\tMake sure you have a valid .ini file in your plugin directory. Copy the" );
        LFX_CRITICAL_STATIC( logstr, "\tfile data/plugin-example.ini to the directory containing multiop.dll/.so" );
        LFX_CRITICAL_STATIC( logstr, "\tand rename it multiop.ini. Edit it, and change the value of the 'Name'" );
        LFX_CRITICAL_STATIC( logstr, "\tvariable to 'MultipleOperationsPluginTest'. Then re-run this test." );

        LFX_ERROR_STATIC( logstr, "Failure: Can't load plugin." );
        return( 1 );
    }
    LFX_NOTICE_STATIC( logstr, pluginName + ": Shared library loaded successfully." );

    // Try to create the loaded operations.
    {
        std::string opName( "MyMask" );
        OperationBasePtr op( plug->createOperation( pluginName, opName ) );
        if( op == NULL )
        {
            LFX_ERROR_STATIC( logstr, "Failure: " + opName + ": createOperation() returned NULL." );
            return( 1 );
        }
        else
        {
            LFX_NOTICE_STATIC( logstr, opName + ": loaded and created successfully." );
        }

        opName = "MyPreprocess";
        OperationBasePtr pr = plug->createOperation( pluginName, opName );
        if( op == NULL )
        {
            LFX_ERROR_STATIC( logstr, "Failure: " + opName + ": createOperation() returned NULL." );
            return( 1 );
        }
        else
        {
            LFX_NOTICE_STATIC( logstr, opName + ": loaded and created successfully." );
        }

        RTPOperationPtr rtpOp = boost::dynamic_pointer_cast<RTPOperation>( op );
        PreprocessPtr prePr = boost::dynamic_pointer_cast<Preprocess>( pr );
        DataSet set;
        set.addOperation( rtpOp );
        set.addPreprocess( prePr );

        std::string err, file( "multiop_pipeline.json" );
        if( !set.savePipeline( file, &err) )
        {
            LFX_ERROR_STATIC( logstr, "Json Serialization: Failed to save the pipeline: " + err );
            return( 1 );
        }

        DataSet newDataSet;
		ObjFactoryCore objf( plug );
        if( !newDataSet.loadPipeline( &objf, file, &err ) )
        {
            LFX_ERROR_STATIC( logstr, "Json Serialization: Failed to load the pipeline: " + err );
            return( 1 );
        }
        if( newDataSet.getNumPreprocess() != 1 )
        {
            std::ostringstream ostr; ostr << newDataSet.getNumPreprocess();
            LFX_ERROR_STATIC( logstr, "Json serialization failed: Expected 1 Preprocess, got " + ostr.str() );
            return( 1 );
        }
        if( newDataSet.getPreprocess( 0 )->getClassName() != prePr->getClassName() )
        {
            LFX_ERROR_STATIC( logstr, "Json serialization failed: Incorrect Preprocess class name: " + newDataSet.getPreprocess( 0 )->getClassName() );
            return( 1 );
        }
        if( newDataSet.getNumOperations() != 1 )
        {
            std::ostringstream ostr; ostr << newDataSet.getNumOperations();
            LFX_ERROR_STATIC( logstr, "Json serialization failed: Expected 1 RTPOperation, got " + ostr.str() );
            return( 1 );
        }
        if( newDataSet.getOperation( 0 )->getClassName() != rtpOp->getClassName() )
        {
            LFX_ERROR_STATIC( logstr, "Json serialization failed: Incorrect RTPOperation class name: " + newDataSet.getOperation( 0 )->getClassName() );
            return( 1 );
        }

        LFX_NOTICE_STATIC( logstr, "Json Serialization: saved and loaded successfully." );

    }

    LFX_CRITICAL_STATIC( logstr, "Pass." );
    return( 0 );
}
