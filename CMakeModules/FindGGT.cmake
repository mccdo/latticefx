# Copyright

# Finds GGT
# This script defines the following:
#  GGT_FOUND (if false or zero, the GGT headers were not found)
#  GGT_INCLUDE_DIR
#
# The GGT_DIR environment variable can be set to
# the parent directory of gmtl/AABox.h


# Based on FindFreetype.cmake
# Created by Eric Wing.
# Modifications by Alexander Neundorf.
# GGT Modifications by Paul Martz.

FIND_PATH( GGT_INCLUDE_DIR gmtl/Xforms.h
  HINTS
    $ENV{GGT_DIR}
    ${GGT_DIR}
  PATHS
    /usr/local/X11R6/include
    /usr/local/X11/include
    /usr/X11/include
    /sw/include
    /opt/local/include
    /usr/freeware/include
)


# handle the QUIETLY and REQUIRED arguments and set
# GGT_FOUND to TRUE as appropriate
INCLUDE( FindPackageHandleStandardArgs )
FIND_PACKAGE_HANDLE_STANDARD_ARGS( GGT DEFAULT_MSG GGT_INCLUDE_DIR )

MARK_AS_ADVANCED(
    GGT_INCLUDE_DIR
)
