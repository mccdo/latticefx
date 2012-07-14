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
#include <latticefx/core/Log.h>
#include <latticefx/core/LogMacros.h>

#include <iostream>


using namespace lfx::core;


const std::string logstr( "lfx.demo" );


int main()
{
    Log::instance()->setPriority( Log::PrioInfo, Log::Console );

    // Add additional plugin search paths.
    PluginManager* plug( PluginManager::instance() );
    plug->loadConfigFiles();

    // Load all plugins named "MultipleOperationsPluginTest".
    const std::string pluginName( "MultipleOperationsPluginTest" );
    if( !( plug->loadPlugins( pluginName ) ) )
    {
        LFX_CRITICAL_STATIC( logstr, "\tMake sure you have a valid .ini file in your plugin directory. Copy the" );
        LFX_CRITICAL_STATIC( logstr, "\tfile data/plugin-example.ini to the directory containing multiop.dll/.so" );
        LFX_CRITICAL_STATIC( logstr, "\tand rename it multiop.ini. Edit it, and change the value of the 'Name'" );
        LFX_CRITICAL_STATIC( logstr, "\tvariable to 'MultipleOperationsPluginTest'. Then re-run this test." );
        return( 1 );
    }
    LFX_NOTICE_STATIC( logstr, pluginName + ": Shared library loaded successfully." );

    // Try to create the loaded operations.
    {
        std::string opName( "MyMask" );
        OperationBasePtr op( plug->createOperation( pluginName, opName ) );
        if( op == NULL )
        {
            LFX_ERROR_STATIC( logstr, opName + ": createOperation() returned NULL." );
        }
        else
        {
            LFX_NOTICE_STATIC( logstr, opName + ": loaded and created successfully." );
        }

        opName = "MyPreprocess";
        op = plug->createOperation( pluginName, opName );
        if( op == NULL )
        {
            LFX_ERROR_STATIC( logstr, opName + ": createOperation() returned NULL." );
        }
        else
        {
            LFX_NOTICE_STATIC( logstr, opName + ": loaded and created successfully." );
        }
    }

    return( 0 );
}
