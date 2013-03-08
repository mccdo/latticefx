# Inputs. If we rewrite this as a function or macro later,
# we need these as parameters:
#   _srcDir - Directory of files to install.
#   _destDir - Relative destincation install directory.
set( _srcDir ${PROJECT_SOURCE_DIR}/data/ )
set( _destDir share/latticefx/data )

install(
    DIRECTORY ${_srcDir} 
    DESTINATION ${_destDir}
    USE_SOURCE_PERMISSIONS
    COMPONENT latticefx-dev
    FILES_MATCHING PATTERN "*"
    PATTERN "CMakeLists.txt" EXCLUDE
    PATTERN ".svn" EXCLUDE
    PATTERN ".DS_Store" EXCLUDE
)
