
set( _libName latticefx )

if( BUILD_SHARED_LIBS )
    add_definitions( -DLATTICEFX_LIBRARY )
endif()


set( _headerFiles
    Export.h
    ChannelData.h
    ChannelDataOSGArray.h
    DataSet.h
    MaskUtils.h
    OperationBase.h
    PageData.h
    PagingThread.h
    PluginManager.h
    Preprocess.h
    RootCallback.h
    Renderer.h
    RTPOperation.h
    TextureUtils.h
)

set( _sourceFiles
    ChannelData.cpp
    ChannelDataOSGArray.cpp
    DataSet.cpp
    MaskUtils.cpp
    OperationBase.cpp
    PageData.cpp
    PagingThread.cpp
    PluginManager.cpp
    Preprocess.cpp
    Renderer.cpp
    RootCallback.cpp
    RTPOperation.cpp
    TextureUtils.cpp
)


_addLibrary( Lib ${_libName}
    ${_headerFiles}
    ${_sourceFiles}
)
