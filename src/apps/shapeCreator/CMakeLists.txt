include_directories(${CMAKE_CURRENT_BINARY_DIR})
if(QT_FOUND)
    include(${QT_USE_FILE})
endif(QT_FOUND)

set( _vtkLibs )
if( VTK_FOUND )
    set( _vtkLibs
        latticefx_utils_vtk
        latticefx_core_vtk
        ${_vtkLibraries}
    )
endif()

set( _sharedSources
    CreateVolume.cpp
    CreateVolume.h
    shapeVolumes.cpp
    shapeVolumes.h
    vtkCreator.cpp
    vtkCreator.h
)


#
# Always build the command line executable
#

set( _exeName shapeCreator-cl )
_addExecutable( App ${_exeName}
    shapeCreator.cpp
    ${_sharedSources}
    LATTICEFX_LIBRARIES
        latticefx_core
        ${_vtkLibs}
)
set_property( TARGET ${_exeName} PROPERTY FOLDER "App")


#
# Optionally build the Qt GUI executable
#

if( QT_FOUND )
    set(qt_hdrs
        MainWindow.h
        creatorThread.h)
    # All headers go with widgets, so must be processed with Qt's MOC.
    qt4_wrap_cpp(MOCCED ${qt_hdrs}  OPTIONS -DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED)

    file(GLOB uis "*.ui")
    qt4_wrap_ui(UICED ${uis})

    set( _exeName shapeCreator )

    _addExecutable( App ${_exeName}
        main.cpp
        MainWindow.cpp
        UtlSettings.cpp
        UtlSettings.h
        creatorThread.cpp
        ${MOCCED}
        ${UICED}
        ${_sharedSources}
        LATTICEFX_LIBRARIES
            latticefx_core
            ${_vtkLibs}
    )

    target_link_libraries(${_exeName} ${QT_LIBRARIES})

    set_property( TARGET ${_exeName} PROPERTY FOLDER "App")

else()
    message( STATUS "shapeCreator build: Qt was not found." )
endif()
