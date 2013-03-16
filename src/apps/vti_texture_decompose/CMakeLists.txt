set( _exeName vti_texture_decompose )
_addExecutable( App ${_exeName}
    vti_texture_decompose.cxx
    LATTICEFX_LIBRARIES
        latticefx_utils_vtk
        latticefx_translators_vtk
        ${_vtkLibraries}
)

set_property( TARGET ${_exeName} PROPERTY FOLDER "App/VTK")
