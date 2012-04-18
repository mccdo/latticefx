
#include <latticefx/PluginManager.h>
#include <latticefx/OperationBase.h>
#include <iostream>


int main()
{
    // Add additional plugin search paths.
    lfx::PluginManager* plug( lfx::PluginManager::instance() );
    plug->loadConfigFiles();

    // Load all plugins named "FactoryPluginTest".
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
