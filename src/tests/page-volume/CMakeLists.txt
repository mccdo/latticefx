set( _exeName page-volume )
_addExecutable( Test ${_exeName}
    page-volume.cpp
    LATTICEFX_LIBRARIES
        latticefx_core
)

set_property( TARGET ${_exeName} PROPERTY FOLDER "Tests")
