set( _exeName test0 )
_addExecutable( Test ${_exeName}
    test0.cpp
    LATTICEFX_LIBRARIES
        latticefx_core
)

set_property( TARGET ${_exeName} PROPERTY FOLDER "Tests")
