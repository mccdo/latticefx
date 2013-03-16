set( _exeName timeseries-vector )
_addExecutable( Test ${_exeName}
    timeseries-vector.cpp
    LATTICEFX_LIBRARIES
        latticefx_core
)

set_property( TARGET ${_exeName} PROPERTY FOLDER "Tests")
