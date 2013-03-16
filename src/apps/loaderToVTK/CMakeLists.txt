set( _exeName loaderToVTK )
_addExecutable( App ${_exeName}
    main.cpp
    LATTICEFX_LIBRARIES
        latticefx_utils_vtk
        latticefx_translators_vtk
        ${_vtkLibraries}
)

set_property( TARGET ${_exeName} PROPERTY FOLDER "App/VTK")
