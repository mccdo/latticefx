#*************** <auto-copyright.pl BEGIN do not edit this line> **************
#
# jag3d is (C) Copyright 2011-2012 by Kenneth Mark Bryden and Paul Martz
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License version 2.1 as published by the Free Software Foundation.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the
# Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.
#
#************** <auto-copyright.pl END do not edit this line> ***************

# If you are using a static POCO build, add the POCO_STATIC entry
# in CMake and select it. Otherwise, glewInit() will come up as an
# undefined symbol.
if( POCO_STATIC )
    add_definitions( -DPOCO_STATIC )
endif()


# Macro to force the FindPOCO.cmake script to
# search again for POCO.
macro( unFindPOCO )
#    message( STATUS "In unFindPOCO" )

    unset( "POCO_INCLUDE_DIR" CACHE )
    unset( "POCO_LIBRARIES" CACHE )
    unset( "POCO_FOUND" CACHE )
endmacro( unFindPOCO )


macro( POCOMenuSetup )
    # What type of POCO should we look for? This is a combo box
    # supporting three types of POCO installations:
    #  * Default installation (in which the stock FindPOCO.cmake script does all the work)
    #  * Alternate Install Location (user must set the POCOInstallLocation variable)
    #  * Source And Build Tree (user must supply both the POCOSourceRoot and POCOBuildRoot variables)
    set( POCOInstallType "Default Installation" CACHE STRING "Type of POCO install: 'Default Installation', 'Alternate Install Location', or 'Source And Build Tree'." )
    set_property( CACHE POCOInstallType PROPERTY STRINGS "Default Installation" "Alternate Install Location" "Source And Build Tree" )

    # We need to detect when the user changes the POCO install type
    # or any of the related directory variables, so that we'll know
    # to call unFindPOCO() and force the stock POCO script to search
    # again. To do this, we save the last set value of these variables
    # in the CMake cache as internal (hidden) variables.
    if( NOT DEFINED _lastPOCOInstallType )
        set( _lastPOCOInstallType "empty" CACHE INTERNAL "" )
    endif()
    if( NOT DEFINED _lastPOCOInstallLocation )
        set( _lastPOCOInstallLocation "empty" CACHE INTERNAL "" )
    endif()
    if( NOT DEFINED _lastPOCOSourceRoot )
        set( _lastPOCOSourceRoot "empty" CACHE INTERNAL "" )
    endif()
    if( NOT DEFINED _lastPOCOBuildRoot )
        set( _lastPOCOBuildRoot "empty" CACHE INTERNAL "" )
    endif()

    if( NOT DEFINED POCOInstallLocation )
        set( POCOInstallLocation "Please specify" )
    endif()
    if( NOT DEFINED POCOSourceRoot )
        set( POCOSourceRoot "Please specify" )
    endif()
    if( NOT DEFINED POCOBuildRoot )
        set( POCOBuildRoot "Please specify" )
    endif()
endmacro()



macro( POCOFinder )
    # If the user has changed the POCO install type combo box
    # (or it's a clean cache), then set or unset our related
    # POCO directory search variables.
    if( NOT ( ${POCOInstallType} STREQUAL ${_lastPOCOInstallType} ) )
    #    message( STATUS "NOT ( ${POCOInstallType} STREQUAL ${_lastPOCOInstallType} )" )

        if( POCOInstallType STREQUAL "Default Installation" )
            # Remove our helper variables and tell the stock script to search again.
            unset( POCOInstallLocation CACHE )
            unset( POCOSourceRoot CACHE )
            unset( POCOBuildRoot CACHE )
        elseif( POCOInstallType STREQUAL "Alternate Install Location" )
            # Enable just the POCOInstallLocation helper variable.
            set( POCOInstallLocation "Please specify" CACHE PATH "Root directory where POCO is installed" )
            unset( POCOSourceRoot CACHE )
            unset( POCOBuildRoot CACHE )
        elseif( POCOInstallType STREQUAL "Source And Build Tree" )
            # Enable the POCOSourceRoot and POCOBuildRoot helper variables.
            unset( POCOInstallLocation CACHE )
            set( POCOSourceRoot "Please specify" CACHE PATH "Root directory of POCO source tree" )
            set( POCOBuildRoot "Please specify" CACHE PATH "Root directory of POCO build tree" )
        endif()
    endif()

    # Look for conditions that require us to find POCO again.
    set( _needToFindPOCO FALSE )
    if( POCOInstallType STREQUAL "Default Installation" )
        if( NOT ( ${POCOInstallType} STREQUAL ${_lastPOCOInstallType} ) )
    #        message( STATUS "Need to find: case A" )
            set( _needToFindPOCO TRUE )
        endif()
    elseif( POCOInstallType STREQUAL "Alternate Install Location" )
        if( NOT ( "${POCOInstallLocation}" STREQUAL "${_lastPOCOInstallLocation}" ) )
    #        message( STATUS "Need to find: case B" )
            set( _needToFindPOCO TRUE )
        endif()
    elseif( POCOInstallType STREQUAL "Source And Build Tree" )
        if( ( NOT ( "${POCOSourceRoot}" STREQUAL "${_lastPOCOSourceRoot}" ) ) OR
            ( NOT ( "${POCOBuildRoot}" STREQUAL "${_lastPOCOBuildRoot}" ) ) )
    #        message( STATUS "Need to find: cade C" )
            set( _needToFindPOCO TRUE )
        endif()
    endif()
    if( _needToFindPOCO )
        unFindPOCO()
        set( _lastPOCOInstallType ${POCOInstallType} CACHE INTERNAL "" FORCE )
        set( _lastPOCOInstallLocation ${POCOInstallLocation} CACHE INTERNAL "" FORCE )
        set( _lastPOCOSourceRoot ${POCOSourceRoot} CACHE INTERNAL "" FORCE )
        set( _lastPOCOBuildRoot ${POCOBuildRoot} CACHE INTERNAL "" FORCE )
    endif()



    # Save internal variables for later restoration
    set( CMAKE_PREFIX_PATH_SAVE ${CMAKE_PREFIX_PATH} )
    set( CMAKE_LIBRARY_PATH_SAVE ${CMAKE_LIBRARY_PATH} )

    set( CMAKE_PREFIX_PATH
        ${POCOInstallLocation}
        ${POCOSourceRoot}
        ${POCOBuildRoot} )
    set( CMAKE_LIBRARY_PATH
        ${POCOInstallLocation}
        ${POCOBuildRoot} )


    find_package( POCO 1.4.0 REQUIRED Util )


    # Restore internal variables
    set( CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH_SAVE} )
    set( CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH_SAVE} )
endmacro()
