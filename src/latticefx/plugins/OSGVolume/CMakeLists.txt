# Required for ModuleInstall.cmake
set( _libName OSGVolume )

_addLibrary( Plugin ${_libName}
    OSGVolume.cpp
    LATTICEFX_LIBRARIES
        latticefx
)

_addPluginINI( OSGVolume.ini )
