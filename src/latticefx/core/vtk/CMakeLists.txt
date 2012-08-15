set( _libName latticefx_core_vtk )

if( BUILD_SHARED_LIBS )
    add_definitions( -DLATTICEFX_CORE_VTK_LIBRARY )
endif()

file(GLOB_RECURSE _headerFiles ${CMAKE_CURRENT_SOURCE_DIR}/*.h )

file(GLOB_RECURSE _sourceFiles ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp )

set(INCLUDE_DIRECTORY_NAME "latticefx/core/vtk")

_addLibrary( Lib ${_libName}
    ${_headerFiles}
    ${_sourceFiles}
    LATTICEFX_LIBRARIES
        latticefx_core
        latticefx_utils_vtk
        latticefx_translators_vtk
        ${_vtkLibraries}
)
