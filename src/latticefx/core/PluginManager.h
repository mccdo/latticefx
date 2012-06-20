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

#ifndef __LATTICEFX_PLUGIN_MANAGER_H__
#define __LATTICEFX_PLUGIN_MANAGER_H__ 1


#include <latticefx/core/Export.h>
#include <latticefx/core/OperationBase.h>
#include <Poco/Path.h>
#include <string>
#include <set>


namespace lfx {



/** \addtogroup PluginSupport Plugin Support
*/
/**@{*/

/** \class OperationInfo PluginManager.h <latticefx/core/PluginManager.h>
\brief A collection of information for classes exported from a plugin.
\details Plugins create an instance of OperationInfo for each class (OperationInfo-derived class)
exported by the plugin. The constructor automatically registers all information with the PluginManager
singleton, using PluginManager::addOperation().
\see REGISTER_OPERATION
*/
struct LATTICEFX_EXPORT OperationInfo
{
    OperationInfo( lfx::OperationBasePtr instance, const std::string& className,
            const std::string& baseClassName, const std::string& description );

    std::string _pluginName;
    std::string _className, _baseClassName, _description;
    lfx::OperationBasePtr _opInstance;

    friend bool operator<( const OperationInfo& lhs, const OperationInfo& rhs );
};
typedef std::vector< OperationInfo > OperationInfoVec;

bool operator<( const OperationInfo& lhs, const OperationInfo& rhs );


/** \def REGISTER_OPERATION
\brief Convenience CPP macro for plugin class registration.
\details Plugins use this macro once for every exporter class they contain.
The macro declares a static instance of type OperationInfo. Static instances
are initialized immediately following plugin (shared libraries) load, which
causes each exported class to immediately register itself with the PluginManager.
See PluginManager::addOperation(), which is invoked by the OperationInfo
constructor. */
#define REGISTER_OPERATION(_instance,_className,_baseClassName,_description) \
static lfx::OperationInfo staticOperationRegistration_##_className( \
    lfx::OperationBasePtr( _instance ), #_className, _baseClassName, _description );



/** \class PluginManager PluginManager.h <latticefx/core/PluginManager.h>
\brief A singleton for managing plugins, plugin search paths, and plugin contents.
\details The PluginManager provides an interface for finding plugins, loading them,
and creating instances of classes contained in plugin.

PluginManager is a singleton, so the first access to PluginManager::instance()
creates the single instance. instance() takes parameter to indicate default plugin
search paths. By default, the current directory, and all directories listed in the
environment variable LATTICEFX_PLUGIN_PATH, are added to the list of search paths.
Additional paths can be added using addPath() and addPaths().

The PluginManager doesn't search for plugins (shared libraries) directly. Instead,
it searches for plugin description files with the extension *.ini. These files use
the Microsoft INI file format and contain a generic plugin name and descrition text.
For an example plugin description file, see data/plugin-example.ini. Applications
load a plugin using loadPlugin() and loadPlugins(). The name parameter is the generic
plugin name from the .ini file. Both the .ini file and the shared library must share the
same base name and exist in the same directory. This system allows multiple plugins to
use the same generic plugin name.

Plugins use the REGISTER_OPERATION macro to register a class (derived from OperationBase)
with the PluginManager. Use of REGISTER_OPERATION results in a call to addOperation()
when the plugin is loaded.

To create an instance of a plugin class object, applications call createOperation().
Currently, there is no way to query the list of classes contained in a plugin, but this
capability can be added as future work. */
class LATTICEFX_EXPORT PluginManager
{
public:
    typedef enum {
        NO_PATHS                    = 0,
        USE_CURRENT_DIRECTORY       = ( 1 << 0x0 ),
        USE_LFX_PLUGIN_PATN_ENV_VAR = ( 1 << 0x1 ),
        USE_SYSTEM_PATH             = ( 1 << 0x2 ),
        USE_LD_LIBRARY_PATH         = ( 1 << 0x3 )
    } InitFlags;

    /** \brief Create the PluginManager singleton instance.
    \param initFlags specifies default plugin search paths.
    */
    static PluginManager* instance( const int initFlags=( USE_CURRENT_DIRECTORY | USE_LFX_PLUGIN_PATN_ENV_VAR ) );
    virtual ~PluginManager();

    /** \brief Add a single plugin search path.
    \details Adds the specified path to the list of paths.
    \param loadConfigs If true, implicitly call loadConfigFiles() to search all
    paths for plugin config files. This is inefficient when calling addPath()
    multiple times. If false, loadConfigFiles() is not called; the calling code
    is responsible for calling this function to search for plugin config files.
    */
    void addPath( const std::string& path, const bool loadConfigs=true );
    /** \brief Like addPath, but loads multiple paths.
    \details \c paths is a colon (on Unix) or semi-colon (on Windows) separated
    list of paths to add. */
    void addPaths( const std::string& paths, const bool loadConfigs=true );
    /** \brief Clear the list of plugin search paths. */
    void clearPaths();
    /** \brief Find plugin config files in all plugin search paths.
    \details In general, applications don't need to call this function explicitly.
    It's called automatically with the second parameter to addPath() and addPaths()
    is true. Also called automatically by the PluginManager constructor when the singleton
    instance is first created. However, applications that make multiple calls to addPath()
    should pass false as the second parameter, then call loadConfigFiles() after the final
    addPath() for efficiency reasons. */
    void loadConfigFiles();

    /** \brief Load all plugins with plugin name equal to \c name. */
    bool loadPlugins( const std::string& name );
    /** \brief Load all plugins whose name and description match the parameters. */
    bool loadPlugins( const std::string& name, const std::string& description );
    /** \brief Load the specific plugin named in \c _pathName. Loaded operations
    will be associated with the plugin name \c name. */
    bool loadPlugin( const std::string& name, const std::string& pathName );

    /** \brief Returns a list of all plugin path names with name \c name. */
    Poco::Path::StringVec find( const std::string& name );
    /** \brief Returns a list of all plugin path names that match the given parameters. */
    Poco::Path::StringVec find( const std::string& name, const std::string& description );

    /** \brief Adds an OperationInfo record to the list of registered operations.
    \details The operation is associated with the actively loading plugin (getActivelyLoadingPlugin()).
    This function is primarily used by the OperationInfo constructor, and indirectly
    by the REFISTER_OPERATION convenience macro for registration of plugin Operations. */
    void addOperation( const OperationInfo& opInfo );

    /** \brief Create a new instance of the registered Operation.
    \details NULL is returned if no Operation named \c className is registered, or it
    is registered for a different plugin than \c pluginName. */
    OperationBasePtr createOperation( const std::string& pluginName, const std::string& className );

    /** \brief Returns the plugin name of the most recently loaded plugin.
    \details This function is not inteded for direct application use, but is public
    for easy access by the OperationInfo class.

    The actively loading plugin name is set by the loadPlugin() and loadPlugins()
    functions immediately prior to actually loading the plugin. When the load occurs, 
    plugin static initializers invoke addOperation(), which calls getActivelyLoadingPlugin()
    and associates all newly registered plugin operations with the actively loading plugin. */
    std::string getActivelyLoadingPlugin() { return( _activelyLoadingPlugin ); }


    typedef struct PluginInfo
    {
        PluginInfo( const std::string& name=std::string( "" ) ) : _name( name ) {}

        Poco::Path _path;         /**< Full path and name to plugin shared library. */
        std::string _name;        /**< Taken from plugin .ini file */
        std::string _description; /**< Taken from plugin .ini file */

        friend bool operator<( const PluginInfo& lhs, const PluginInfo& rhs );
    };
    typedef std::multiset< PluginInfo > PluginInfoSet;
    typedef std::pair< PluginInfoSet::iterator, PluginInfoSet::iterator > PluginInfoSetRange;

protected:
    PluginManager( const int initFlags );

    bool internalLoadLibraries( const Poco::Path::StringVec& libNames );

    std::string _activelyLoadingPlugin;

    Poco::Path::StringVec _paths;
    PluginInfoSet _pluginInfo;

    OperationInfoVec _opInfo;
};

bool operator<( const PluginManager::PluginInfo& lhs, const PluginManager::PluginInfo& rhs );

/**@}*/


// lfx
}


// __LATTICEFX_PLUGIN_MANAGER_H__
#endif
