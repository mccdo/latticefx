# - Find a latticefx installation or build tree.
# The following variables are set if latticefx is found.  If latticefx is not
# found, latticefx_FOUND is set to false.
#  latticefx_FOUND         - Set to true when latticefx is found.
#  latticefx_USE_FILE      - CMake file to use latticefx.
#  latticefx_MAJOR_VERSION - The latticefx major version number.
#  latticefx_MINOR_VERSION - The latticefx minor version number 
#                       (odd non-release).
#  latticefx_BUILD_VERSION - The latticefx patch level 
#                       (meaningless for odd minor).
#  latticefx_INCLUDE_DIRS  - Include directories for latticefx
#  latticefx_LIBRARY_DIRS  - Link directories for latticefx libraries

# The following cache entries must be set by the user to locate latticefx:
#  latticefx_DIR  - The directory containing latticefxConfig.cmake.  
#             This is either the root of the build tree,
#             or the lib directory.  This is the 
#             only cache entry.


# Assume not found.
SET(latticefx_FOUND 0)

# Construct consitent error messages for use below.
SET(latticefx_DIR_DESCRIPTION "directory containing latticefxConfig.cmake.  This is either the root of the build tree, or PREFIX/lib for an installation.")
SET(latticefx_DIR_MESSAGE "latticefx not found.  Set the latticefx_DIR cmake cache entry to the ${latticefx_DIR_DESCRIPTION}")

# Use the Config mode of the find_package() command to find latticefxConfig.
# If this succeeds (possibly because latticefx_DIR is already set), the
# command will have already loaded latticefxConfig.cmake and set latticefx_FOUND.
IF(NOT latticefx_FOUND)
  FIND_PACKAGE(latticefx QUIET NO_MODULE)
ENDIF(NOT latticefx_FOUND)

#-----------------------------------------------------------------------------
IF(NOT latticefx_FOUND)
  # latticefx not found, explain to the user how to specify its location.
  IF(latticefx_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR ${latticefx_DIR_MESSAGE})
  ELSE(latticefx_FIND_REQUIRED)
    IF(NOT latticefx_FIND_QUIETLY)
      MESSAGE(STATUS ${latticefx_DIR_MESSAGE})
    ENDIF(NOT latticefx_FIND_QUIETLY)
  ENDIF(latticefx_FIND_REQUIRED)
ENDIF(NOT latticefx_FOUND)
