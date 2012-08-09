_addExecutable( Test test-vtk-vector
    test-vtk-vector.cpp
    LATTICEFX_LIBRARIES
        latticefx_core
        latticefx_utils_vtk
        latticefx_core_vtk
        ${_vtkLibraries}
)
