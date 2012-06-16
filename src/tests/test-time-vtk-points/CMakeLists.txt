_addExecutable( Test test-time-vtk-points
    test-time-vtk-points.cpp
    LATTICEFX_LIBRARIES
        latticefx
        vtk_utils
        ${_vtkLibraries}
)
