if(VTK_FOUND)
    add_subdirectory(vtk)
endif(VTK_FOUND)

set( _libName latticefx )

if( BUILD_SHARED_LIBS )
    add_definitions( -DLATTICEFX_LIBRARY )
endif()


set( _headerFiles
    Export.h
    BoundUtils.h
    ChannelData.h
    ChannelDataComposite.h
    ChannelDataImageSet.h
    ChannelDataLOD.h
    ChannelDataOSGArray.h
    ChannelDataOSGImage.h
    DataSet.h
    DBUtils.h
    LoadRequest.h
    OperationBase.h
    PageData.h
    PagingCallback.h
    PagingThread.h
    PlayControl.h
    PluginManager.h
    Preprocess.h
    Renderer.h
    RTPOperation.h
    SurfaceRenderer.h
    TextureUtils.h
    TransferFunctionUtils.h
    VectorRenderer.h
    VolumeRenderer.h
)

set( _sourceFiles
    BoundUtils.cpp
    ChannelData.cpp
    ChannelDataComposite.cpp
    ChannelDataImageSet.cpp
    ChannelDataLOD.cpp
    ChannelDataOSGArray.cpp
    ChannelDataOSGImage.cpp
    DataSet.cpp
    DBUtils.cpp
    LoadRequest.cpp
    OperationBase.cpp
    PageData.cpp
    PagingCallback.cpp
    PagingThread.cpp
    PlayControl.cpp
    PluginManager.cpp
    Preprocess.cpp
    Renderer.cpp
    RTPOperation.cpp
    SurfaceRenderer.cpp
    TextureUtils.cpp
    TransferFunctionUtils.cpp
    VectorRenderer.cpp
    VolumeRenderer.cpp
)

set(INCLUDE_DIRECTORY_NAME "latticefx/core")

_addLibrary( Lib ${_libName}
    ${_headerFiles}
    ${_sourceFiles}
    LATTICEFX_LIBRARIES
        ${_vtkLibraries}
)


