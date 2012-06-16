
set( _libName vtk_utils )

if( BUILD_SHARED_LIBS )
    add_definitions( -DLATTICEFX_LIBRARY )
endif()

set( HEADER_PATH ${PROJECT_SOURCE_DIR}/src/vtk_utils )

file(GLOB_RECURSE _headerFiles ${HEADER_PATH}/*.h )

file(GLOB_RECURSE _sourceFiles ${HEADER_PATH}/*.cpp )

_addLibrary( Lib ${_libName}
    ${_headerFiles}
    ${_sourceFiles}
    LATTICEFX_LIBRARIES
        ${_vtkLibraries}
)
