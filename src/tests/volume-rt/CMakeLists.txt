set( _exeName volume-rt )
_addExecutable( Test ${_exeName}
    volume-rt.cpp
    LATTICEFX_LIBRARIES
        latticefx_core
)

set_property( TARGET ${_exeName} PROPERTY FOLDER "Tests")
