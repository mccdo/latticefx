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
    vtkCreator.h
)

file(GLOB srcs "*.cpp")
list( REMOVE_ITEM srcs 
    "${CMAKE_SOURCE_DIR}/src/apps/shapeCreator/main.cpp")
list(APPEND _sharedSources ${srcs})

set(qt_hdrs
    MainWindow.h
    creatorThread.h)

# All headers in this dir go with widgets, so must be processed with Qt's MOC.
qt4_wrap_cpp(MOCCED ${qt_hdrs}  OPTIONS -DBOOST_TT_HAS_OPERATOR_HPP_INCLUDED)

file(GLOB uis "*.ui")
qt4_wrap_ui(UICED ${uis})

list(APPEND _sharedSources ${MOCCED})
list(APPEND _sharedSources ${UICED})

set( _exeName shapeCreator )

_addExecutable( App ${_exeName}
    ${_sharedSources}
    LATTICEFX_LIBRARIES
        latticefx_core
        ${_vtkLibs}
)

target_link_libraries(${_exeName} ${QT_LIBRARIES})

set_property( TARGET ${_exeName} PROPERTY FOLDER "App")

if( QT_FOUND )
    #include( ${QT_USE_FILE} )

    # To be done: Add second executable,
    # using Qt GUI.

else()
    if( QT_QMAKE_EXECUTABLE AND NOT QT4_QGLOBAL_H_FILE )
        message( STATUS "Qt was not found because the qglobal.h file was not found." )
    endif()
endif()
