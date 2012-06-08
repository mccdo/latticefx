_addExecutable( Test test-vtk-vector
    OSGVectorStage.cxx
    VectorFieldData.cxx
    VTKTextureCreator.cxx
    test-vtk-vector.cpp
    LATTICEFX_LIBRARIES
        latticefx
        vtk_utils
        ${_vtkLibraries}
)
