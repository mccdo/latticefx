set( _exeName vtk-timeseries-points )
_addExecutable( Test ${_exeName}
    vtk-timeseries-points.cpp
    LATTICEFX_LIBRARIES
        latticefx_core
        latticefx_utils_vtk
        latticefx_core_vtk
        ${_vtkLibraries}
)

set_property( TARGET ${_exeName} PROPERTY FOLDER "Tests/VTK")
