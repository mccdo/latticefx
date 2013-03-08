/*************** <auto-copyright.rb BEGIN do not edit this line> **************
 *
 * Copyright 2012-2012 by Ames Laboratory
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 *************** <auto-copyright.rb END do not edit this line> ***************/

#include <latticefx/core/DBOperations.h>
#include <latticefx/core/DataSet.h>
#include <latticefx/core/ChannelDataOSGArray.h>
#include <latticefx/core/ChannelDataOSGImage.h>

#include <latticefx/core/Log.h>
#include <latticefx/core/LogMacros.h>

static std::string logstr( "lfx.ctest.dbops" );


using namespace lfx::core;

int main( int argc, char** argv )
{
    Log::instance()->setPriority( Log::PrioInfo, Log::Console );

    LFX_CRITICAL_STATIC( logstr, "This is a CTest regression test. To launch under Visual Studio, build the" );
    LFX_CRITICAL_STATIC( logstr, "RUN_TESTS target. Under Linux, enter 'make test' at a shell prompty.\n" );


    /*
    NameStringGenerator nsg( osg::Vec3s( 128, 128, 128 ) );

    std::string result( nsg.getNameString( osg::Vec3s( 128, 128, 128 ), osg::Vec3s( 0, 0, 0 ) ) );
    std::string correct( "" );
    if( result != correct )
    {
        LFX_ERROR_STATIC( logstr, "Failure: Root node has non-null name string: " + result );
        return( 1 );
    }
    */


    LFX_CRITICAL_STATIC( logstr, "Pass." );
    return( 0 );
}
