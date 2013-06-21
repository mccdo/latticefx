if(VTK_FOUND)
    add_subdirectory(vtk)
endif(VTK_FOUND)

set( _libName latticefx_core )

if( BUILD_SHARED_LIBS )
    add_definitions( -DLATTICEFX_LIBRARY )
endif()


set( _headerFiles
    Export.h
    BoundUtils.h
	CallbackProgress.h
    ChannelData.h
    ChannelDataComposite.h
    ChannelDataImageSet.h
    ChannelDataLOD.h
    ChannelDataOSGArray.h
    ChannelDataOSGImage.h
    DataSet.h
    DataSetPtr.h
    DBBase.h
    DBDisk.h
#    DBUtils.h
    DBOperations.h
    HierarchyUtils.h
    LoadRequest.h
    Log.h
    LogBase.h
    LogMacros.h
    OctreeGroup.h
    OperationBase.h
    OperationValue.h
    PageData.h
    PagingCallback.h
    PagingThread.h
    PlayControl.h
    PlayControlPtr.h
    PluginManager.h
    Preprocess.h
    Renderer.h
    RTPOperation.h
    SurfaceRenderer.h
    TextureUtils.h
    TransferFunctionUtils.h
    types.h
    VectorRenderer.h
    VolumeRenderer.h
)

set( _sourceFiles
    BoundUtils.cpp
	CallbackProgress.cpp
    ChannelData.cpp
    ChannelDataComposite.cpp
    ChannelDataImageSet.cpp
    ChannelDataLOD.cpp
    ChannelDataOSGArray.cpp
    ChannelDataOSGImage.cpp
    DataSet.cpp
    DBBase.cpp
    DBDisk.cpp
#    DBUtils.cpp
    DBOperations.cpp
    HierarchyUtils.cpp
    LoadRequest.cpp
    Log.cpp
    OctreeGroup.cpp
    OperationBase.cpp
    OperationValue.cpp
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

if( crunchstore_FOUND )
    list( APPEND _sourceFiles DBCrunchStore.cpp )
    list( APPEND _headerFiles DBCrunchStore.h )
endif()

set(INCLUDE_DIRECTORY_NAME "latticefx/core")

_addLibrary( Lib ${_libName}
    ${_headerFiles}
    ${_sourceFiles}
    LATTICEFX_LIBRARIES
        ${_vtkLibraries}
)