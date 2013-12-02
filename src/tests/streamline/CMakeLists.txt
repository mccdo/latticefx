set( _exeName streamline )
_addExecutable( Test ${_exeName}
    streamline.cpp
    LATTICEFX_LIBRARIES
        latticefx_core
)

set_property( TARGET ${_exeName} PROPERTY FOLDER "Tests")
