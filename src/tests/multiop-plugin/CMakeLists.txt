set( _exeName multiop-plugin )
_addExecutable( CTest ${_exeName}
    multiop-plugin.cpp
    LATTICEFX_LIBRARIES
        latticefx_core
)

set_property( TARGET ${_exeName} PROPERTY FOLDER "Tests/Regression")

add_test(
    NAME ${_exeName}
    COMMAND ${_exeName}
)
