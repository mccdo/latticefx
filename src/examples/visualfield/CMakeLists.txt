set( _exeName visualfield )
_addExecutable( Example ${_exeName}
    visualfield.cpp
    LATTICEFX_LIBRARIES
        latticefx_core
)

set_property( TARGET ${_exeName} PROPERTY FOLDER "Examples")
