set( _libName osgdb_latticefx_core )

_addLibrary( Plugin ${_libName}
    OctreeGroup-dotOSG.cpp
    PageData-dotOSG.cpp
    LATTICEFX_LIBRARIES
        latticefx_core
)

set_property( TARGET ${_libName} PROPERTY FOLDER "Plugins")
