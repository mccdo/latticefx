set( _exeName hierarchyutils )
_addExecutable( CTest ${_exeName}
    hierarchyutils.cpp
    LATTICEFX_LIBRARIES
        latticefx_core
)

add_test(
    NAME ${_exeName}
    COMMAND ${_exeName}
)
