set( _exeName raw2ive )
_addExecutable( App ${_exeName}
    main.cpp
    LATTICEFX_LIBRARIES
)

set_property( TARGET ${_exeName} PROPERTY FOLDER "App")
