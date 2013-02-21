if(VTK_FOUND)
    add_subdirectory( loaderToVTK )
    add_subdirectory( vti_texture_decompose )
endif(VTK_FOUND)

add_subdirectory( coneTextureCreator )
add_subdirectory( raw2ive )
add_subdirectory( shapeCreator )
