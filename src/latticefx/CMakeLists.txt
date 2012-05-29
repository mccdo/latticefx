
set( _libName latticefx )

if( BUILD_SHARED_LIBS )
    add_definitions( -DLATTICEFX_LIBRARY )
endif()


set( _headerFiles
    Export.h
    BoundUtils.h
    ChannelData.h
    ChannelDataComposite.h
    ChannelDataOSGArray.h
    ChannelDataOSGImage.h
    DataSet.h
    DBUtils.h
    OperationBase.h
    PageData.h
    PagingThread.h
    PlayControl.h
    PluginManager.h
    Preprocess.h
    RootCallback.h
    Renderer.h
    RTPOperation.h
    TextureUtils.h
    TransferFunctionUtils.h
    VectorRenderer.h
    VolumeRenderer.h
)

set( _sourceFiles
    BoundUtils.cpp
    ChannelData.cpp
    ChannelDataComposite.cpp
    ChannelDataOSGArray.cpp
    ChannelDataOSGImage.cpp
    DataSet.cpp
    DBUtils.cpp
    OperationBase.cpp
    PageData.cpp
    PagingThread.cpp
    PlayControl.cpp
    PluginManager.cpp
    Preprocess.cpp
    Renderer.cpp
    RootCallback.cpp
    RTPOperation.cpp
    TextureUtils.cpp
    TransferFunctionUtils.cpp
    VectorRenderer.cpp
    VolumeRenderer.cpp
)


_addLibrary( Lib ${_libName}
    ${_headerFiles}
    ${_sourceFiles}
)
