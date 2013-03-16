set( _exeName hierarchyutils )
_addExecutable( CTest ${_exeName}
    hierarchyutils.cpp
    LATTICEFX_LIBRARIES
        latticefx_core
)

set_property( TARGET ${_exeName} PROPERTY FOLDER "Tests/Regression")

add_test(
    NAME ${_exeName}
    COMMAND ${_exeName}
)
