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
#message( ${_vtkLibraries} )

set( _projectIncludes
    ${PROJECT_SOURCE_DIR}/src
)
set( _requiredDependencyIncludes
    ${POCO_INCLUDE_DIR}
    ${Boost_INCLUDE_DIR}
	#${OSGWORKS_INCLUDE_DIR}
    ${OSG_INCLUDE_DIRS}
)
set( _optionalDependencyIncludes )
if( crunchstore_FOUND )
    list( APPEND _optionalDependencyIncludes ${crunchstore_INCLUDE_DIRS} )
endif()

set(_osgWorksLibraries
    osgwTools
)

set( _requiredDependencyLibraries
    ${POCO_LIBRARIES}
    ${Boost_LIBRARIES}
	${_osgWorksLibraries}
    ${OSG_LIBRARIES}
)
set( _optionalDependencyLibraries )
if( crunchstore_FOUND )
    list( APPEND _optionalDependencyLibraries ${crunchstore_LIBRARIES} )
endif()
# if( VTK_FOUND )
#     list( APPEND _optionalDependencyLibraries ${_vtkLibraries} )
# endif()


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
        ${_optionalDependencyIncludes}
    )

    _splitList( LATTICEFX_LIBRARIES sources libs ${ARGN} )

    if( ${_category} STREQUAL "Plugin" )
        add_library( ${_libName} MODULE ${sources} )
    elseif( BUILD_SHARED_LIBS )
        add_library( ${_libName} SHARED ${sources} )
        set_target_properties( ${_libName} PROPERTIES VERSION ${LATTICEFX_VERSION} )
        set_target_properties( ${_libName} PROPERTIES SOVERSION ${LATTICEFX_VERSION} )
    else()
        add_library( ${_libName} ${sources} )
        set_target_properties( ${_libName} PROPERTIES VERSION ${LATTICEFX_VERSION} )
        set_target_properties( ${_libName} PROPERTIES SOVERSION ${LATTICEFX_VERSION} )
    endif()

    target_link_libraries( ${_libName}
        ${libs}
        ${_requiredDependencyLibraries}
        ${_optionalDependencyLibraries}
    )

    set_target_properties( ${_libName} PROPERTIES PROJECT_LABEL "${_category} ${_libName}" )

    include( ModuleInstall REQUIRED )
endmacro()

# Supports LatticeFX plugin INI files, which must reside in the
# same directory as the plugin shared libraries / DLLs. This means
# they must be installed into the bin directory, but also they
# must be copied into the CMake output build tree per config type.
#
macro( _addPluginINI _iniName )
    # Copy .ini file to development bin directories.
    # NOTE this is done during CMake config.
    foreach( _configDir ${CMAKE_CONFIGURATION_TYPES} )
        configure_file( ${_iniName}
            ${PROJECT_BINARY_DIR}/bin/${_configDir}/${_iniName} COPYONLY )
    endforeach()

    # Install .ini file to bin directory.
    install( FILES ${_iniName}
        DESTINATION bin
        COMPONENT latticefx-dev
    )
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
        ${_optionalDependencyIncludes}
    )

    target_link_libraries( ${_exeName}
        ${libs}
        ${_requiredDependencyLibraries}
        ${_optionalDependencyLibraries}
    )

    if( ${_category} STREQUAL "App" )
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
