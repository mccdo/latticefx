set( _libName SimpleDownsample )

_addLibrary( Plugin ${_libName}
    SimpleDownsample.cpp
    LATTICEFX_LIBRARIES
        latticefx_core
)

_addPluginINI( SimpleDownsample.ini )

set_property( TARGET ${_libName} PROPERTY FOLDER "Plugins")
