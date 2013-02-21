_addExecutable( App vti_texture_decompose
    vti_texture_decompose.cxx
    LATTICEFX_LIBRARIES
        latticefx_utils_vtk
        latticefx_translators_vtk
        ${_vtkLibraries}
)
