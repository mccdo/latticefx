set( _libName latticefx_core_vtk )

if( BUILD_SHARED_LIBS )
    add_definitions( -DLATTICEFX_CORE_VTK_LIBRARY )
endif()

set( _headerFiles
    Export.h
    ChannelDatavtkDataObject.h
    ChannelDatavtkPolyData.h
)

set( _sourceFiles
    ChannelDatavtkDataObject.cpp
    ChannelDatavtkPolyData.cpp
)

_addLibrary( Lib ${_libName}
    ${_headerFiles}
    ${_sourceFiles}
    LATTICEFX_LIBRARIES
        latticefx
        ${_vtkLibraries}
)

