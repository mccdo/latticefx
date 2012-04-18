
set( _libName multiop )


set( _sourceFiles
    multiop.cpp
)


_addLibrary( Plugin ${_libName}
    ${_sourceFiles}
    LATTICEFX_LIBRARIES
        latticefx
)
