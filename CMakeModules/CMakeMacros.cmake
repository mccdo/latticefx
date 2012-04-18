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
)
set( _projectLibraries
    latticefx
)



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


macro( _addExecutable _category _exeName )
    add_executable( ${_exeName}
        ${ARGN}
    )

    include_directories(
        ${_projectIncludes}
        ${_requiredDependencyIncludes}
    )

    target_link_libraries( ${_exeName}
        ${_projectLibraries}
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
