set( _exeName page-volume-rt )
_addExecutable( Test ${_exeName}
    page-volume-rt.cpp
    LATTICEFX_LIBRARIES
        latticefx_core
)

set_property( TARGET ${_exeName} PROPERTY FOLDER "Tests")
