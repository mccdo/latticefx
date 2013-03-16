set( _exeName volume )
_addExecutable( Test ${_exeName}
    volume.cpp
    LATTICEFX_LIBRARIES
        latticefx_core
)

set_property( TARGET ${_exeName} PROPERTY FOLDER "Tests")
