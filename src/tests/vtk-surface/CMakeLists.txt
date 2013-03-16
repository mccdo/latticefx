set( _exeName vtk-surface )
_addExecutable( Test ${_exeName}
    vtk-surface.cpp
    LATTICEFX_LIBRARIES
        latticefx_core
        latticefx_core_vtk
)

set_property( TARGET ${_exeName} PROPERTY FOLDER "Tests/VTK")
