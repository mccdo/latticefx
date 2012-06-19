
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
    TextureUtils.h
    TransferFunctionUtils.h
    VectorRenderer.h
    VolumeRenderer.h
)

if(VTK_FOUND)
    list(APPEND _headerFiles ChannelDatavtkPolyData.h ChannelDatavtkDataObject.h)
endif(VTK_FOUND)

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
    TextureUtils.cpp
    TransferFunctionUtils.cpp
    VectorRenderer.cpp
    VolumeRenderer.cpp
)

if(VTK_FOUND)
    list(APPEND _sourceFiles ChannelDatavtkPolyData.cpp ChannelDatavtkDataObject.cpp)
endif(VTK_FOUND)

_addLibrary( Lib ${_libName}
    ${_headerFiles}
    ${_sourceFiles}
    LATTICEFX_LIBRARIES
        ${_vtkLibraries}
)

