_addExecutable( Test test-vtk-vector
    test-vtk-vector.cpp
    LATTICEFX_LIBRARIES
        latticefx
        latticefx_utils_vtk
        latticefx_core_vtk
        ${_vtkLibraries}
)
