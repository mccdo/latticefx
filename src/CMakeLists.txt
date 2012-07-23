add_subdirectory( latticefx )

if( LFX_BUILD_APPS )
    add_subdirectory( apps )
endif()

if( LFX_BUILD_EXAMPLES )
    add_subdirectory( examples )
endif()

if( LFX_BUILD_TESTS )
    add_subdirectory( tests )
endif()
