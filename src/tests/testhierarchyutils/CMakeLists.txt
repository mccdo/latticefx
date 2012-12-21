set( _exeName testhierarchyutils )
_addExecutable( Test ${_exeName}
    testhierarchyutils.cpp
    LATTICEFX_LIBRARIES
        latticefx_core
)

add_test(
    NAME ${_exeName}
    COMMAND ${_exeName}
)
