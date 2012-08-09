_addExecutable( Test test-time-vtk-points
    test-time-vtk-points.cpp
    LATTICEFX_LIBRARIES
        latticefx_core
        latticefx_utils_vtk
        latticefx_core_vtk
        ${_vtkLibraries}
)
