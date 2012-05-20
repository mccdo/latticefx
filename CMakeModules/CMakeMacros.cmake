if(WIN32)
    set( CMAKE_DEBUG_POSTFIX d )
endif()


# Given an input token _tokenIn, return all the elements
# in ARGN preceding _tokenIn in _list0out, and all the elements
# following _tokenIn in _list1out. If _tokenIn is not present in _listIn,
# _list0out will be identical to ARGN, and _list1out will be empty.
#
macro( _splitList _tokenIn _list0out _list1out )
    set( ${_list0out} )
    set( ${_list1out} )
    set( tokenFound 0 )

    foreach( element ${ARGN} )
        if( tokenFound )
            list( APPEND ${_list1out} ${element} )
        else()
            if( ${element} STREQUAL ${_tokenIn} )
                set( tokenFound 1 )
            else()
                list( APPEND ${_list0out} ${element} )
            endif()
        endif()
    endforeach()
endmacro()

#
#setup the VTK libraries that are needed for various libraries
set( _vtkLibraries )
if( VTK_FOUND )
    list( APPEND _vtkLibraries
        vtkCommon
        vtkIO
        vtkFiltering
        vtkRendering
        #used for text - may be able to remove later
        vtkHybrid
        )
endif()
#        vtkImaging
#        vtkRendering
#        vtkParallel
#        vtkGraphics
#        vtksys
#        vtkexoIIc
#        vtkftgl
#        vtkDICOMParser
#        vtkNetCDF
#        vtkmetaio
#        vtksqlite
#        vtkverdict
#        vtkfreetype
#        vtkNetCDF_cxx
#        vtkpng
#        vtkjpeg
#        vtktiff
#        vtkexpat
#        VPIC
#        Cosmo

set( _requiredDependencyIncludes
    ${POCO_INCLUDE_DIR}
    ${Boost_INCLUDE_DIR}
	${OSGWORKS_INCLUDE_DIR}
    ${OSG_INCLUDE_DIRS}
)
set( _projectIncludes
    ${PROJECT_SOURCE_DIR}/src
)

set( _requiredDependencyLibraries
    ${POCO_LIBRARIES}
    ${Boost_LIBRARIES}
	${OSGWORKS_LIBRARIES}
    ${OSG_LIBRARIES}
    ${_vtkLibraries}
)



# Usage:
#   _addLibrary( <category> <libraryName>
#      <sourceFile1>
#      <sourceFile2>
#      <sourceFile3> ...etc...
#      [ LATTICEFX_LIBRARIES
#         <lib1>
#         <lib2> ...etc... ]
#
# <category> is a project label such as "Lib" or "Plugin".
# <libraryName> is the library name.
# <sourceFileN> are the library source code files (.cpp/.h).
# <libN> are libraries to link with.
# 
# <libraryName> is always linked with ${_requiredDependencyLibraries}.
# Any additional libraries specified after the LATTICEFX_LIBRARIES keyword
# are added on the link line before ${_requiredDependencyLibraries}.
#
macro( _addLibrary _category _libName )
    include_directories(
        ${_projectIncludes}
        ${_requiredDependencyIncludes}
    )

    _splitList( LATTICEFX_LIBRARIES sources libs ${ARGN} )

    if( ( ${_category} STREQUAL "Plugin" ) OR BUILD_SHARED_LIBS )
        add_library( ${_libName} SHARED ${sources} )
    else()
        add_library( ${_libName} ${sources} )
    endif()

    target_link_libraries( ${_libName}
        ${libs}
        ${_requiredDependencyLibraries}
    )

    set_target_properties( ${_libName} PROPERTIES VERSION ${LATTICEFX_VERSION} )
    set_target_properties( ${_libName} PROPERTIES SOVERSION ${LATTICEFX_VERSION} )
    set_target_properties( ${_libName} PROPERTIES PROJECT_LABEL "${_category} ${_libName}" )

    include( ModuleInstall REQUIRED )
endmacro()


# Usage:
#   _addExecutable( <category> <executableName>
#      <sourceFile1>
#      <sourceFile2>
#      <sourceFile3> ...etc...
#      [ LATTICEFX_LIBRARIES
#         <lib1>
#         <lib2> ...etc... ]
#
# <category> is a project label such as "App", "Test", or "Example".
# <executableName> is the executable name.
# <sourceFileN> are the executable source code files (.cpp/.h).
# <libN> are libraries to link with.
# 
# <executableName> is always linked with ${_requiredDependencyLibraries}.
# Any additional libraries specified after the LATTICEFX_LIBRARIES keyword
# are added on the link line before ${_requiredDependencyLibraries}.
#
macro( _addExecutable _category _exeName )
    _splitList( LATTICEFX_LIBRARIES sources libs ${ARGN} )

    add_executable( ${_exeName}
        ${sources}
    )

    include_directories(
        ${_projectIncludes}
        ${_requiredDependencyIncludes}
    )

    target_link_libraries( ${_exeName}
        ${libs}
        ${_requiredDependencyLibraries}
    )
    
    if( _category STREQUAL "App" )
        install(
            TARGETS ${_exeName}
            RUNTIME DESTINATION bin COMPONENT latticefx
        )
    else()
        install(
            TARGETS ${_exeName}
            RUNTIME DESTINATION share/${CMAKE_PROJECT_NAME}/bin COMPONENT latticefx
        )
    endif()

    set_target_properties( ${_exeName} PROPERTIES PROJECT_LABEL "${_category} ${_exeName}" )
endmacro()
