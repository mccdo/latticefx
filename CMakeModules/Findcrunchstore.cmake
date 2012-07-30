# - Find a crunchstore installation or build tree.
# The following variables are set if crunchstore is found.  If crunchstore is not
# found, crunchstore_FOUND is set to false.
#  crunchstore_FOUND         - Set to true when crunchstore is found.
#  crunchstore_USE_FILE      - CMake file to use crunchstore.
#  crunchstore_MAJOR_VERSION - The crunchstore major version number.
#  crunchstore_MINOR_VERSION - The crunchstore minor version number 
#                       (odd non-release).
#  crunchstore_BUILD_VERSION - The crunchstore patch level 
#                       (meaningless for odd minor).
#  crunchstore_INCLUDE_DIRS  - Include directories for crunchstore
#  crunchstore_LIBRARY_DIRS  - Link directories for crunchstore libraries

# The following cache entries must be set by the user to locate crunchstore:
#  crunchstore_DIR  - The directory containing crunchstoreConfig.cmake.  
#             This is either the root of the build tree,
#             or the lib directory.  This is the 
#             only cache entry.


# Assume not found.
SET(crunchstore_FOUND 0)

# Construct consitent error messages for use below.
SET(crunchstore_DIR_DESCRIPTION "directory containing crunchstoreConfig.cmake.  This is either the root of the build tree, or PREFIX/lib for an installation.")
SET(crunchstore_DIR_MESSAGE "crunchstore not found.  Set the crunchstore_DIR cmake cache entry to the ${crunchstore_DIR_DESCRIPTION}")

# Use the Config mode of the find_package() command to find crunchstoreConfig.
# If this succeeds (possibly because crunchstore_DIR is already set), the
# command will have already loaded crunchstoreConfig.cmake and set crunchstore_FOUND.
IF(NOT crunchstore_FOUND)
  FIND_PACKAGE(crunchstore QUIET NO_MODULE)
ENDIF(NOT crunchstore_FOUND)

#-----------------------------------------------------------------------------
IF(NOT crunchstore_FOUND)
  # crunchstore not found, explain to the user how to specify its location.
  IF(crunchstore_FIND_REQUIRED)
    MESSAGE(FATAL_ERROR ${crunchstore_DIR_MESSAGE})
  ELSE(crunchstore_FIND_REQUIRED)
    IF(NOT crunchstore_FIND_QUIETLY)
      MESSAGE(STATUS ${crunchstore_DIR_MESSAGE})
    ENDIF(NOT crunchstore_FIND_QUIETLY)
  ENDIF(crunchstore_FIND_REQUIRED)
ENDIF(NOT crunchstore_FOUND)
