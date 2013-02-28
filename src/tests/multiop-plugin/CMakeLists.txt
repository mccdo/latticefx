set( _exeName multiop-plugin )
_addExecutable( CTest ${_exeName}
    multiop-plugin.cpp
    LATTICEFX_LIBRARIES
        latticefx_core
)

add_test(
    NAME ${_exeName}
    COMMAND ${_exeName}
)
