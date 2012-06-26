
set( _libName latticefx_utils_vtk )

if( BUILD_SHARED_LIBS )
    add_definitions( -DLATTICEFX_UTILS_VTK_LIBRARY )
endif()

set( HEADER_PATH ${PROJECT_SOURCE_DIR}/src/latticefx/utils/vtk )

file(GLOB_RECURSE _headerFiles ${HEADER_PATH}/*.h )

file(GLOB_RECURSE _sourceFiles ${HEADER_PATH}/*.cpp )

set(INCLUDE_DIRECTORY_NAME "latticefx/utils/vtk")

_addLibrary( Lib ${_libName}
    ${_headerFiles}
    ${_sourceFiles}
    LATTICEFX_LIBRARIES
        ${_vtkLibraries}
)
