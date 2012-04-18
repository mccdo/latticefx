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

# Macro to force the FindGgt.cmake script to
# search again for Ggt.
macro( unFindGgt )
#    message( STATUS "In unFindGgt" )

    unset( "GGT_INCLUDE_DIR" CACHE )
    unset( "GGT_FOUND" CACHE )
endmacro( unFindGgt )


macro( GGTMenuSetup )
    # What type of Ggt should we look for? This is a combo box
    # supporting three types of Ggt installations:
    #  * Default installation (in which the stock FindGgt.cmake script does all the work)
    #  * Alternate Install Location (user must set the GGTInstallLocation variable)
    #  * Source And Build Tree (user must supply both the GGTSourceRoot and GGTBuildRoot variables)
    set( GGTInstallType "Default Installation" CACHE STRING "Type of Ggt install: 'Default Installation', 'Alternate Install Location', or 'Source And Build Tree'." )
    set_property( CACHE GGTInstallType PROPERTY STRINGS "Default Installation" "Alternate Install Location" "Source And Build Tree" )

    # We need to detect when the user changes the Ggt install type
    # or any of the related directory variables, so that we'll know
    # to call unFindGgt() and force the stock Ggt script to search
    # again. To do this, we save the last set value of these variables
    # in the CMake cache as internal (hidden) variables.
    if( NOT DEFINED _lastGGTInstallType )
        set( _lastGGTInstallType "empty" CACHE INTERNAL "" )
    endif()
    if( NOT DEFINED _lastGGTInstallLocation )
        set( _lastGGTInstallLocation "empty" CACHE INTERNAL "" )
    endif()
    if( NOT DEFINED _lastGGTSourceRoot )
        set( _lastGGTSourceRoot "empty" CACHE INTERNAL "" )
    endif()
    if( NOT DEFINED _lastGGTBuildRoot )
        set( _lastGGTBuildRoot "empty" CACHE INTERNAL "" )
    endif()

    if( NOT DEFINED GGTInstallLocation )
        set( GGTInstallLocation "Please specify" )
    endif()
    if( NOT DEFINED GGTSourceRoot )
        set( GGTSourceRoot "Please specify" )
    endif()
    if( NOT DEFINED GGTBuildRoot )
        set( GGTBuildRoot "Please specify" )
    endif()
endmacro()



macro( GGTFinder )
    # If the user has changed the Ggt install type combo box
    # (or it's a clean cache), then set or unset our related
    # Ggt directory search variables.
    if( NOT ( ${GGTInstallType} STREQUAL ${_lastGGTInstallType} ) )
    #    message( STATUS "NOT ( ${GGTInstallType} STREQUAL ${_lastGGTInstallType} )" )

        if( GGTInstallType STREQUAL "Default Installation" )
            # Remove our helper variables and tell the stock script to search again.
            unset( GGTInstallLocation CACHE )
            unset( GGTSourceRoot CACHE )
            unset( GGTBuildRoot CACHE )
        elseif( GGTInstallType STREQUAL "Alternate Install Location" )
            # Enable just the GGTInstallLocation helper variable.
            set( GGTInstallLocation "Please specify" CACHE PATH "Root directory where Ggt is installed" )
            unset( GGTSourceRoot CACHE )
            unset( GGTBuildRoot CACHE )
        elseif( GGTInstallType STREQUAL "Source And Build Tree" )
            # Enable the GGTSourceRoot and GGTBuildRoot helper variables.
            unset( GGTInstallLocation CACHE )
            set( GGTSourceRoot "Please specify" CACHE PATH "Root directory of Ggt source tree" )
            set( GGTBuildRoot "Please specify" CACHE PATH "Root directory of Ggt build tree" )
        endif()
    endif()

    # Look for conditions that require us to find Ggt again.
    set( _needToFindGgt FALSE )
    if( GGTInstallType STREQUAL "Default Installation" )
        if( NOT ( ${GGTInstallType} STREQUAL ${_lastGGTInstallType} ) )
    #        message( STATUS "Need to find: case A" )
            set( _needToFindGgt TRUE )
        endif()
    elseif( GGTInstallType STREQUAL "Alternate Install Location" )
        if( NOT ( "${GGTInstallLocation}" STREQUAL "${_lastGGTInstallLocation}" ) )
    #        message( STATUS "Need to find: case B" )
            set( _needToFindGgt TRUE )
        endif()
    elseif( GGTInstallType STREQUAL "Source And Build Tree" )
        if( ( NOT ( "${GGTSourceRoot}" STREQUAL "${_lastGGTSourceRoot}" ) ) OR
            ( NOT ( "${GGTBuildRoot}" STREQUAL "${_lastGGTBuildRoot}" ) ) )
    #        message( STATUS "Need to find: cade C" )
            set( _needToFindGgt TRUE )
        endif()
    endif()
    if( _needToFindGgt )
        unFindGgt()
        set( _lastGGTInstallType ${GGTInstallType} CACHE INTERNAL "" FORCE )
        set( _lastGGTInstallLocation ${GGTInstallLocation} CACHE INTERNAL "" FORCE )
        set( _lastGGTSourceRoot ${GGTSourceRoot} CACHE INTERNAL "" FORCE )
        set( _lastGGTBuildRoot ${GGTBuildRoot} CACHE INTERNAL "" FORCE )
    endif()



    # Save internal variables for later restoration
    set( CMAKE_PREFIX_PATH_SAVE ${CMAKE_PREFIX_PATH} )
    set( CMAKE_LIBRARY_PATH_SAVE ${CMAKE_LIBRARY_PATH} )

    set( CMAKE_PREFIX_PATH
        ${GGTInstallLocation}
        ${GGTSourceRoot}
        ${GGTBuildRoot} )
    set( CMAKE_LIBRARY_PATH
        ${GGTInstallLocation}
        ${GGTBuildRoot} )


    find_package( GGT REQUIRED )


    # Restore internal variables
    set( CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH_SAVE} )
    set( CMAKE_LIBRARY_PATH ${CMAKE_LIBRARY_PATH_SAVE} )
endmacro()
