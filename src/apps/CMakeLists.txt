if(VTK_FOUND)
    add_subdirectory( loaderToVTK )
    add_subdirectory( vti_texture_decompose )
    add_subdirectory( vtk-utils )
endif(VTK_FOUND)

add_subdirectory( raw2ive )
add_subdirectory( shapeCreator )
