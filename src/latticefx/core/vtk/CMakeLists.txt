set( _libName latticefx_core_vtk )

if( BUILD_SHARED_LIBS )
    add_definitions( -DLATTICEFX_CORE_VTK_LIBRARY )
endif()

set( _headerFiles
    Export.h
    ChannelDatavtkDataObject.h
    ChannelDatavtkPolyData.h
    VTKVectorFieldRTP.h
    VTKVectorSliceRTP.h
    VTKVectorRenderer.h
    CuttingPlane.h
    VTKContourSliceRTP.h
    VTKIsosurfaceRTP.h
)

set( _sourceFiles
    ChannelDatavtkDataObject.cpp
    ChannelDatavtkPolyData.cpp
    VTKVectorFieldRTP.cpp
    VTKVectorSliceRTP.cpp
    VTKVectorRenderer.cpp
    CuttingPlane.cpp
    VTKContourSliceRTP.cpp
    VTKIsosurfaceRTP.cpp
)

_addLibrary( Lib ${_libName}
    ${_headerFiles}
    ${_sourceFiles}
    LATTICEFX_LIBRARIES
        latticefx
        latticefx_utils_vtk
        ${_vtkLibraries}
)

