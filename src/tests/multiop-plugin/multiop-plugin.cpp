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


int main()
{
    lfx::Log::instance()->setPriority( lfx::Log::PrioInfo, lfx::Log::Console );

    // Add additional plugin search paths.
    lfx::PluginManager* plug( lfx::PluginManager::instance() );
    plug->loadConfigFiles();

    // Load all plugins named "MultipleOperationsPluginTest".
    const std::string pluginName( "MultipleOperationsPluginTest" );
    if( !( plug->loadPlugins( pluginName ) ) )
    {
        std::cout << "\tMake sure you have a valid .ini file in your plugin directory. Copy the" << std::endl;
        std::cout << "\tfile data/plugin-example.ini to the directory containing multiop.dll/.so" << std::endl;
        std::cout << "\tand rename it multiop.ini. Edit it, and change the value of the 'Name'" << std::endl;
        std::cout << "\tvariable to 'MultipleOperationsPluginTest'. Then re-run this test." << std::endl;
        return( 1 );
    }
    std::cout << pluginName << ": Shared library loaded successfully." << std::endl;

    // Try to create the loaded operations.
    {
        std::string opName( "MyMask" );
        lfx::OperationBasePtr op( plug->createOperation( pluginName, opName ) );
        if( op == NULL )
            std::cerr << opName << ": createOperation() returned NULL." << std::endl;
        else
            std::cout << opName << ": loaded and created successfully." << std::endl;

        opName = "MyPreprocess";
        op = plug->createOperation( pluginName, opName );
        if( op == NULL )
            std::cerr << opName << ": createOperation() returned NULL." << std::endl;
        else
            std::cout << opName << ": loaded and created successfully." << std::endl;
    }

    return( 0 );
}
