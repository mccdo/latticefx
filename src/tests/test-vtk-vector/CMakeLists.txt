_addExecutable( Test test-vtk-vector
    test-vtk-vector.cpp
    LATTICEFX_LIBRARIES
        latticefx
        vtk_utils
        ${_vtkLibraries}
)
