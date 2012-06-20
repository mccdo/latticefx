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
#include <Poco/Glob.h>
#include <Poco/File.h>
#include <Poco/SharedLibrary.h>
#include <Poco/ClassLoader.h>
#include <Poco/Util/IniFileConfiguration.h>
#include <Poco/Environment.h>
#include <Poco/AutoPtr.h>
#include <boost/foreach.hpp>
#include <iostream>
#include <set>


using Poco::Util::IniFileConfiguration;


namespace lfx {


OperationInfo::OperationInfo( lfx::OperationBasePtr instance, const std::string& className,
            const std::string& baseClassName, const std::string& description )
  : _opInstance( instance ),
    _pluginName( lfx::PluginManager::instance()->getActivelyLoadingPlugin() ),
    _className( className ),
    _baseClassName( baseClassName ),
    _description( description )
{
    lfx::PluginManager::instance()->addOperation( *this );
}

bool operator<( const OperationInfo& lhs, const OperationInfo& rhs )
{
    if( lhs._pluginName < rhs._pluginName )
        return( true );
    else if( lhs._pluginName > rhs._pluginName )
        return( false );

    if( lhs._className < rhs._className )
        return( true );
    else if( lhs._className > rhs._className )
        return( false );

    if( lhs._baseClassName < rhs._baseClassName )
        return( true );
    else if( lhs._baseClassName > rhs._baseClassName )
        return( false );

    if( lhs._description < rhs._description )
        return( true );
    else
        return( false );
}




PluginManager* PluginManager::instance( const int initFlags )
{
    static PluginManager* s_instance( new PluginManager( initFlags ) );
    return( s_instance );
}

PluginManager::PluginManager( const int initFlags )
{
    if( ( initFlags & USE_CURRENT_DIRECTORY ) != 0 )
    {
        addPath( Poco::Path::current(), false );
    }
    if( ( initFlags & USE_LFX_PLUGIN_PATN_ENV_VAR ) != 0 )
    {
        std::string paths;
        try {
            paths = Poco::Environment::get( "LATTICEFX_PLUGIN_PATH" );
        } catch (...) {}
        if( !paths.empty() )
            addPaths( paths, false );
    }
    if( ( initFlags & USE_SYSTEM_PATH ) != 0 )
    {
        std::string paths;
        try {
            Poco::Environment::get( "PATH" );
        } catch (...) {}
        if( !paths.empty() )
            addPaths( paths, false );
    }
    if( ( initFlags & USE_LD_LIBRARY_PATH ) != 0 )
    {
        std::string paths;
        try {
            Poco::Environment::get( "LD_LIBRARY_PATH" );
        } catch (...) {}
        if( !paths.empty() )
            addPaths( paths, false );
    }

    if( !_paths.empty() )
        loadConfigFiles();
}
PluginManager::~PluginManager()
{
}

void PluginManager::addPath( const std::string& path, const bool loadConfigs )
{
    _paths.push_back( path );
    if( loadConfigs )
        loadConfigFiles();
}
void PluginManager::addPaths( const std::string& paths, const bool loadConfigs )
{
    if( paths.empty() )
        return;

    const char sep( Poco::Path::pathSeparator() );
    std::string::size_type pos, lastPos( 0 );
    do {
        pos = paths.find( sep, lastPos );
        const std::string::size_type len( pos - lastPos );
        //std::cout << paths.substr( lastPos, len ) << " " << pos << " " << lastPos << std::endl;
        if( len > 0 )
            _paths.push_back( paths.substr( lastPos, len ) );
        lastPos = pos+1;
    } while( pos != paths.npos );

    if( loadConfigs )
        loadConfigFiles();
}
void PluginManager::clearPaths()
{
    _paths.clear();
}


bool PluginManager::loadPlugins( const std::string& name )
{
    Poco::Path::StringVec pluginPaths( find( name ) );
    if( pluginPaths.empty() )
    {
        std::cerr << "No plugin found for \"" << name << "\"." << std::endl;
        std::cerr << "Possible missing or corrupt .ini file." << std::endl;
        return( false );
    }

    _activelyLoadingPlugin = name;
    return( internalLoadLibraries( pluginPaths ) );
}
bool PluginManager::loadPlugins( const std::string& name, const std::string& description )
{
    Poco::Path::StringVec pluginPaths( find( name, description ) );
    if( pluginPaths.empty() )
    {
        std::cerr << "No plugin found for \"" << name << "\"." << std::endl;
        std::cerr << "Possible missing or corrupt .ini file." << std::endl;
        return( false );
    }

    _activelyLoadingPlugin = name;
    return( internalLoadLibraries( pluginPaths ) );
}
bool PluginManager::loadPlugin( const std::string& name, const std::string& pathName )
{
    Poco::Path::StringVec pluginPaths;
    pluginPaths.push_back( pathName );
    _activelyLoadingPlugin = name;
    return( internalLoadLibraries( pluginPaths ) );
}

bool PluginManager::internalLoadLibraries( const Poco::Path::StringVec& libNames )
{
    if( libNames.empty() )
        return( false );

    // Attempt to load all shared libraries found.
    BOOST_FOREACH( Poco::Path::StringVec::value_type libName, libNames )
    {
        typedef Poco::ClassLoader< lfx::OperationBase > LibLoader;
        LibLoader loader;
        try {
            loader.loadLibrary( libName );
        } catch( Poco::LibraryLoadException lle ) {
            std::cerr << "Caught Poco::LibraryLoadException." << std::endl;
            std::cerr << "Exception message: " << lle.message() << std::endl;
            return( false );
        } catch( ... ) {
            std::cerr << "Can't load \"" << libName << "\", unknown exception." << std::endl;
            return( false );
        }
    }

    return( true );
}


Poco::Path::StringVec PluginManager::find( const std::string& name )
{
    Poco::Path::StringVec returnPaths;
    PluginInfoSetRange range( _pluginInfo.equal_range( PluginInfo( name ) ) );
    PluginInfoSet::const_iterator it( range.first );
    while( it != range.second )
    {
        returnPaths.push_back( it->_path.toString() );
        it++;
    }

    return( returnPaths );
}
Poco::Path::StringVec PluginManager::find( const std::string& name, const std::string& description )
{
    std::cout << "This function is not yet implemented." << std::endl;
    return( Poco::Path::StringVec() );
}

void PluginManager::addOperation( const OperationInfo& opInfo )
{
    _opInfo.push_back( opInfo );
}

lfx::OperationBasePtr PluginManager::createOperation( const std::string& pluginName, const std::string& className )
{
    BOOST_FOREACH( const OperationInfo& opInfo, _opInfo )
    {
        if( ( opInfo._pluginName == pluginName ) && 
                ( opInfo._className == className ) )
            return( OperationBasePtr( opInfo._opInstance->create() ) );
    }
    return( OperationBasePtr( ( OperationBase* )( NULL ) ) );
}


void PluginManager::loadConfigFiles()
{
    typedef std::set< std::string > StringSet;

    _pluginInfo.clear();

    BOOST_FOREACH( Poco::Path::StringVec::value_type s, _paths )
    {
        Poco::Path path, libPath;
        if( !( path.tryParse( s.c_str() ) ) )
            continue;
        path.makeDirectory();
        path.setFileName( "*.ini" );

        StringSet stringSet;
        Poco::Glob::glob( path, stringSet, Poco::Glob::GLOB_DOT_SPECIAL );
        BOOST_FOREACH( StringSet::value_type iniFileName, stringSet )
        {
            //std::cout << iniFileName << std::endl;

            Poco::AutoPtr< IniFileConfiguration > conf( new IniFileConfiguration( iniFileName ) );
            PluginInfo pi;
            try {
                pi._name = conf->getString( "LatticeFXPlugin.Name" );
                pi._description = conf->getString( "LatticeFXPlugin.Description" );
            }
            catch( ... )
            {
                // Not one of our .ini files, or badly formed .ini file.
                continue;
            }

            pi._path = path;
            pi._path.setFileName( Poco::Path( iniFileName ).getBaseName()
                + Poco::SharedLibrary::suffix() );
            if( !( Poco::File( pi._path ).exists() ) )
                continue;

            _pluginInfo.insert( pi );

            //std::cout << pi._path.toString() << std::endl;
            //std::cout << pi._name << std::endl;
            //std::cout << pi._description << std::endl;
        }
    }
}


bool operator<( const PluginManager::PluginInfo& lhs, const PluginManager::PluginInfo& rhs )
{
    if( lhs._name < rhs._name )
        return( true );
    else
        return( false );
}


// lfx
}
