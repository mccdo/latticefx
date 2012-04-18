cmake_minimum_required( VERSION 2.8 )
project( latticefx )
set( LATTICEFX_VERSION 0 )

set( CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMakeModules;${CMAKE_MODULE_PATH}" )


set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib )
if( WIN32 )
    set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} )
else()
    set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY} )
endif()


if( WIN32 AND MSVC )
    # Disable warnings
    # Use foreach in case we want to disable more than 1 warning in the future.
    foreach( warning 4251 )
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /wd${warning}" )
    endforeach()

    option( LATTICEFX_WIN32_USE_MP "Build with multiple processes (/MP option; VS 2005 and above)." ON )
    mark_as_advanced( LATTICEFX_WIN32_USE_MP )
    if( LATTICEFX_WIN32_USE_MP )
        set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP" )
    endif()
endif()


include( FindPocoHelper )
PocoMenuSetup()

include( FindOSGHelper )
PocoFinder()
find_package( Boost REQUIRED )
find_package( osgWorks REQUIRED )

find_package( VTK )
if( VTK_FOUND )
    include( ${VTK_USE_FILE} )
endif()


include( CMakeMacros )


option( BOOST_ALL_DYN_LINK "If set, use Boost dynamic libraries (boost_*); otherwise static (libboost_*)." OFF )
if( BOOST_ALL_DYN_LINK )
    add_definitions( -DBOOST_ALL_DYN_LINK )
endif()

option( BUILD_SHARED_LIBS "Set to ON to build libraries for dynamic linking. Use OFF for static." ON )
if( NOT BUILD_SHARED_LIBS )
    add_definitions( -DLATTICEFX_STATIC )
endif()



add_subdirectory( src/latticefx )
add_subdirectory( src/plugins )
add_subdirectory( src/tests )



#
# Doxygen
find_package( Doxygen )
if( DOXYGEN_FOUND )
    set( HAVE_DOT "NO" )
    if( DOXYGEN_DOT_PATH )
        set( HAVE_DOT "YES" )
    endif()

    configure_file( ${PROJECT_SOURCE_DIR}/doc/doxyfile.cmake
        ${PROJECT_BINARY_DIR}/doc/doxyfile
    )
    add_custom_target( Documentation ${DOXYGEN_EXECUTABLE}
        ${PROJECT_BINARY_DIR}/doc/doxyfile
    )
endif()

if( MSVC )
    install(
        DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/\${CMAKE_INSTALL_CONFIG_NAME}/
        DESTINATION lib
        USE_SOURCE_PERMISSIONS
        COMPONENT latticefx-dev
        FILES_MATCHING PATTERN "*.pdb"
    )
endif()
