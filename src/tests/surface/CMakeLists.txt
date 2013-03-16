set( _exeName surface )
_addExecutable( Test ${_exeName}
    surface.cpp
    LATTICEFX_LIBRARIES
        latticefx_core
)

set_property( TARGET ${_exeName} PROPERTY FOLDER "Tests")
