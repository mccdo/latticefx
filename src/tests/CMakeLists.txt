# CTest regression tests
add_subdirectory( dboperations )
add_subdirectory( hierarchyutils )
add_subdirectory( multiop-plugin )

# Run-time / interactive tests
add_subdirectory( page-volume )
add_subdirectory( page-volume-rt )
add_subdirectory( surface )
add_subdirectory( test0 )
add_subdirectory( timeseries-vector )
add_subdirectory( volume )
add_subdirectory( volume-rt )

if( VTK_FOUND )
    add_subdirectory( vtk-surface )
    add_subdirectory( vtk-timeseries-points )
    add_subdirectory( vtk-vector )
endif()

