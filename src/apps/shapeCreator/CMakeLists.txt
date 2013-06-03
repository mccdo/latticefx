set( _vtkLibs )
if( VTK_FOUND )
    set( _vtkLibs
        latticefx_utils_vtk
        latticefx_core_vtk
        ${_vtkLibraries}
    )
endif()

set( _sharedSources
    shapeCreator.h
    shapeCreator.cpp
    vtkCreator.h
    vtkCreator.cpp
)

set( _exeName shapeCreator )
_addExecutable( App ${_exeName}
    ${_sharedSources}
    LATTICEFX_LIBRARIES
        latticefx_core
        ${_vtkLibs}
)

set_property( TARGET ${_exeName} PROPERTY FOLDER "App")


if( QT_FOUND )
    include( ${QT_USE_FILE} )

    # To be done: Add second executable,
    # using Qt GUI.

else()
    if( QT_QMAKE_EXECUTABLE AND NOT QT4_QGLOBAL_H_FILE )
        message( STATUS "Qt was not found because the qglobal.h file was not found." )
    endif()
endif()
