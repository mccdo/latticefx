add_subdirectory( latticefx )
add_subdirectory( plugins )

if(VTK_FOUND)
    add_subdirectory( vtk_utils )
    add_subdirectory( vtk_translator )
endif(VTK_FOUND)

add_subdirectory( tests )
add_subdirectory( apps )

