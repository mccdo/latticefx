_addExecutable( Test vtk-timeseries-points
    vtk-timeseries-points.cpp
    LATTICEFX_LIBRARIES
        latticefx_core
        latticefx_utils_vtk
        latticefx_core_vtk
        ${_vtkLibraries}
)
