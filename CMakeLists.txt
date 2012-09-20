cmake_minimum_required( VERSION 2.8.8 )
project( latticefx )
set( LATTICEFX_VERSION 0 )

set( CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/CMakeModules;${CMAKE_MODULE_PATH}" )


set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib )


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

find_package( Boost REQUIRED thread date_time filesystem system chrono program_options)

find_package( osgWorks 2.0.0 REQUIRED )
if( osgWorks_FOUND )
    include( ${osgWorks_USE_FILE} )
endif()

find_package( VTK )
if( VTK_FOUND )
    include( ${VTK_USE_FILE} )
endif()

find_package( crunchstore 0.2.0 )
if( crunchstore_FOUND )
    include( ${crunchstore_USE_FILE} )
    add_definitions( -DLFX_USE_CRUNCHSTORE )
endif()

include( CMakeMacros )

option( LFX_DISABLE_LOGGING "If set, Lfx disables all console and log file writes." OFF )
if( LFX_DISABLE_LOGGING )
    add_definitions( -DLFX_DISABLE_LOGGING )
    add_definitions( -DLFX_LOG=false )
else()
    add_definitions( -DLFX_LOG=true )
endif()

option( BOOST_ALL_DYN_LINK "If set, use Boost dynamic libraries (boost_*); otherwise static (libboost_*)." OFF )
if( BOOST_ALL_DYN_LINK )
    add_definitions( -DBOOST_ALL_DYN_LINK )
endif()

option( BUILD_SHARED_LIBS "Set to ON to build libraries for dynamic linking. Use OFF for static." ON )
if( NOT BUILD_SHARED_LIBS )
    add_definitions( -DLATTICEFX_STATIC )
endif()


option( LFX_BUILD_APPS "Build applications." ON )
option( LFX_BUILD_EXAMPLES "Build examples." ON )
option( LFX_BUILD_TESTS "Build tests." ON )


#
# Add all of the libraries to the build system
add_subdirectory( data )
add_subdirectory( src )

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

#
# Create and install all of the auto find tools
# =============================================

# Add all targets to the build-tree export set
set( _targets latticefx_core )
if( VTK_FOUND )
    list( APPEND _targets latticefx_core_vtk latticefx_translators_vtk latticefx_utils_vtk )
endif()
export(TARGETS ${_targets}
  FILE "${PROJECT_BINARY_DIR}/lib/latticefxLibraryDepends.cmake")

# Export the package for use from the build-tree
# (this registers the build-tree with a global CMake-registry)
export(PACKAGE latticefx)

# Create a latticefxBuildTreeSettings.cmake file for the use from the build tree
configure_file(latticefxBuildTreeSettings.cmake.in
  "${PROJECT_BINARY_DIR}/lib/latticefxBuildTreeSettings.cmake" @ONLY)

#configure_file(latticefxConfig.cmake.in
#  "${PROJECT_BINARY_DIR}/lib/latticefxConfig.cmake" @ONLY)
configure_file(latticefxConfigVersion.cmake.in
  "${PROJECT_BINARY_DIR}/lib/latticefxConfigVersion.cmake" @ONLY)
configure_file(Uselatticefx.cmake.in
  "${PROJECT_BINARY_DIR}/lib/Uselatticefx.cmake" @ONLY)

# Install the export set for use with the install-tree
install(EXPORT latticefx-targets DESTINATION lib)

include( CMakePackageConfigHelpers )
# Create the latticefxConfig.cmake and latticefxConfigVersion files
file(RELATIVE_PATH CONF_REL_INCLUDE_DIR "${CMAKE_INSTALL_PREFIX}/lib"
   "${CMAKE_INSTALL_PREFIX}/include")

configure_package_config_file( 
    latticefxConfig.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/lib/latticefxConfig.cmake
    INSTALL_DESTINATION lib
    PATH_VARS CONF_REL_INCLUDE_DIR )

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lib/latticefxConfig.cmake 
              ${CMAKE_CURRENT_BINARY_DIR}/lib/latticefxConfigVersion.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/lib/Uselatticefx.cmake
        DESTINATION lib )

#
# Setup the fpc auto detection files
if(WIN32)
    set(latticefx_FPC_LIBRARIES "/libpath:\"\${libdir}\" latticefx_core.lib latticefx_core_vtk.lib latticefx_translators_vtk.lib latticefx_utils_vtk.lib")
    set(latticefx_FPC_INCLUDES "/I\"\${includedir}\"")
else(WIN32)
    set(latticefx_FPC_INCLUDES "-I\${includedir}")
    set(latticefx_FPC_LIBRARIES "-L\${libdir} -llatticefx_core -llatticefx_core_vtk -llatticefx_translators_vtk -llatticefx_utils_vtk")
endif(WIN32)

configure_file(extra/latticefx.fpc.in
  "${PROJECT_BINARY_DIR}/lib/flagpoll/latticefx.fpc" @ONLY)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/lib/flagpoll/latticefx.fpc
        DESTINATION lib/flagpoll )

