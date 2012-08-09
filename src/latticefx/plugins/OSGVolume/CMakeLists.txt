# Required for ModuleInstall.cmake
set( _libName OSGVolume )

_addLibrary( Plugin ${_libName}
    OSGVolume.cpp
    LATTICEFX_LIBRARIES
        latticefx_core
)

_addPluginINI( OSGVolume.ini )
