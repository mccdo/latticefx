set( _vtkLibs )
if( VTK_FOUND )
    set( _vtkLibs
        latticefx_utils_vtk
        latticefx_core_vtk
        ${_vtkLibraries}
    )
endif()

_addExecutable( App shapeCreator
    shapeCreator.cpp
    LATTICEFX_LIBRARIES
        latticefx_core
        ${_vtkLibs}
)
