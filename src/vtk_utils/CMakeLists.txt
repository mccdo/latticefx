
set( _libName lfx_vtk_utils )

if( BUILD_SHARED_LIBS )
    add_definitions( -DLATTICEFX_VTK_UTILSLIBRARY )
endif()

set( HEADER_PATH ${PROJECT_SOURCE_DIR}/src/vtk_utils )

file(GLOB_RECURSE _headerFiles ${HEADER_PATH}/*.h )

file(GLOB_RECURSE _sourceFiles ${HEADER_PATH}/*.cxx )

_addLibrary( Lib ${_libName}
    ${_headerFiles}
    ${_sourceFiles}
)
