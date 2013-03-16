set( _libName multiop )

_addLibrary( Plugin ${_libName}
    multiop.cpp
    LATTICEFX_LIBRARIES
        latticefx_core
)

_addPluginINI( multiop.ini )

set_property( TARGET ${_libName} PROPERTY FOLDER "Plugins")
