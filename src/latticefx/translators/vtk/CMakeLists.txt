
set( _libName latticefx_translators_vtk )

if( BUILD_SHARED_LIBS )
    add_definitions( -DLATTICEFX_LIBRARY )
endif()

set( HEADER_PATH ${PROJECT_SOURCE_DIR}/src/latticefx/translators/vtk )

file(GLOB_RECURSE _headerFiles ${HEADER_PATH}/*.h )

file(GLOB_RECURSE _sourceFiles ${HEADER_PATH}/*.cpp )

_addLibrary( Lib ${_libName}
    ${_headerFiles}
    ${_sourceFiles}
    LATTICEFX_LIBRARIES
        vtk_utils
        ${_vtkLibraries}
)
