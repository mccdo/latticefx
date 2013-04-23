set( _vtkLibs )
if( VTK_FOUND )
    set( _vtkLibs
        latticefx_utils_vtk
        latticefx_core_vtk
        ${_vtkLibraries}
    )
endif()

set( _exeName shapeCreator )
_addExecutable( App ${_exeName}
    shapeCreator.h
    shapeCreator.cpp
    vtkCreator.h
    vtkCreator.cpp
    LATTICEFX_LIBRARIES
        latticefx_core
        ${_vtkLibs}
)

set_property( TARGET ${_exeName} PROPERTY FOLDER "App")
