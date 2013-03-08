set( _exeName dboperations )
_addExecutable( CTest ${_exeName}
    dboperations.cpp
    LATTICEFX_LIBRARIES
        latticefx_core
)

add_test(
    NAME ${_exeName}
    COMMAND ${_exeName}
)
