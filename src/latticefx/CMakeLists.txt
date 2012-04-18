
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
    PluginInterface.h
    PluginManager.h
    Preprocess.h
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
    PluginInterface.cpp
    PluginManager.cpp
    Preprocess.cpp
    Renderer.cpp
    RTPOperation.cpp
    TextureUtils.cpp
)


_addLibrary( Lib ${_libName}
    ${_headerFiles}
    ${_sourceFiles}
)
