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

#include <latticefx/core/Log.h>
#include <latticefx/core/LogMacros.h>
#include <osg/ArgumentParser>
#include <boost/filesystem.hpp>
#include "CreateVolume.h"

#ifdef VTK_FOUND
#  include "VtkCreator.h"
#endif

const std::string logstr( "lfx.demo" );
const std::string loginfo( logstr+".info" );

using namespace lfx::core;

int main( int argc, char** argv )
{
	Log::instance()->setPriority( Log::PrioFatal, Log::Console );
	Log::instance()->setPriority( Log::PrioInfo, loginfo );

    // Please document all options using Doxygen at the bottom of this file.
    LFX_CRITICAL_STATIC( logstr, "With no command line args, write image data as files using DBDisk." );
    LFX_CRITICAL_STATIC( logstr, "-depth <d> Hierarchy levels. Default is 4." );
#ifdef LFX_USE_CRUNCHSTORE
    LFX_CRITICAL_STATIC( logstr, "-cs <dbFile> Write volume image data files using DBCrunchStore." );
#endif
    LFX_CRITICAL_STATIC( logstr, "-cube Generate a cube data set. This is the default if no other shape is specified." );
    LFX_CRITICAL_STATIC( logstr, "-scube Generate a soft cube data set." );
    LFX_CRITICAL_STATIC( logstr, "-cone Generate a cone data set." );
    LFX_CRITICAL_STATIC( logstr, "-sphere Generate a sphere data set." );
    LFX_CRITICAL_STATIC( logstr, "-ssphere Generate a soft sphere data set." );
#ifdef VTK_FOUND
    LFX_CRITICAL_STATIC( logstr, "-vtk <file> Generate a data set from a VTK volume data file." );
	LFX_CRITICAL_STATIC( logstr, "-s Generate a dataset for each scalar in a VTK volume data file." );
	LFX_CRITICAL_STATIC( logstr, "-v Generate a dataset for each vector in a VTK volume data file." );
	LFX_CRITICAL_STATIC( logstr, "-s0 Generate a dataset for scalar number 0 in a VTK volume data file (you can specify 0..(n-1)." );
	LFX_CRITICAL_STATIC( logstr, "-v0 Generate a dataset for vector number 0 in a VTK volume data file (you can specify 0..(n-1)." );
	LFX_CRITICAL_STATIC( logstr, "-threads number will specify the number of threads to use for VTK brick creation, if left out the default of 32 is used" );
	LFX_CRITICAL_STATIC( logstr, "-nocache will create VTK bricks with out storing or using a cache system. The cache system is much faster for mutliple scalars and vectors, but uses lots of memory" );
	LFX_CRITICAL_STATIC( logstr, "-hireslod will create a seperate VTK brick object for each level of detail. This should create a better quality rendering, but is slower to create." );
#endif
    LFX_CRITICAL_STATIC( logstr, "-prune Do not generate empty subvolumes." );

    osg::ArgumentParser arguments( &argc, argv );

	// validate database file
	std::string csFile;
#ifdef LFX_USE_CRUNCHSTORE
    arguments.read( "-cs", csFile );

	// make sure the directory exists if a file was specified
	if (csFile.size() > 0)
	{
		std::string dir = csFile.substr( 0, csFile.find_last_of( "/\\" ) );
		if ( dir.size() <= 0 )
		{
			LFX_FATAL_STATIC( logstr, "You must specify the full path of your database file." );
			return( 0 );
		}

		if ( !boost::filesystem::is_directory( dir ) )
		{
			std::ostringstream ss;
			ss << "Database folder doesn't exist: " << dir;
			LFX_FATAL_STATIC( logstr, ss.str().c_str() );
			return( 0 );
		}
	}
#endif

#ifdef VTK_FOUND
    if( arguments.find( "-vtk" ) > 0 )
    {
		boost::shared_ptr<VtkCreator> vtk ( new VtkCreator( logstr.c_str(), loginfo.c_str() ) );
		return vtk->create();//arguments, csFile);
    }
#endif

	CreateVolume createVolume( logstr.c_str(), loginfo.c_str() );
	createVolume.create( arguments, csFile );

    return( 0 );
}



/** \page AppShapeCreator Application  shapeCreator

shapeCreator generates sample volumetric data for use with the page-volume
and page-volume-rt test programs.

<h2>Hierarchy Depth Control</h2>

shapeCreator creates a hierarchy of volumetric levels of detail.
Larger values display more detail (if available in the source data)
at near viewing distances, but large values also create more data
files and increase shapeCreator runtime duration. Use the -depth command
line argument to control the hierarchy depth:

\li -depth <d> Hierarchy levels. Default is 4.

Valid values are >= 1.

<h2>Shape Creation</h2>

By default, shapeCreator generates a cube data set. This is the same as the \c -cube option.
\li -cube Generate a cube data set. This is the default if no other shape is specified.

Generate other shapes by specifying one of these options:
\li -scube Generate a soft cube with gradient scalar values.
\li -cone Generate a cone data set.
\li -sphere Generate a sphere data set.
\li -ssphere Generate a soft sphere with gradient scalar values.

If you've built LatticeFX with the optional VTK dependency, you can also
generate hierarchies for VTK folume data.
\li -vtk <file> Generate a data set from a VTK volume data file
\li -s Generate a dataset for each scalar in a VTK volume data file
\li -v Generate a dataset for each vector in a VTK volume data file
\li -s0 Generate a dataset for scalar number 0 in a VTK volume data file (you can specify 0..(n-1)
\li -v0 Generate a dataset for vector number 0 in a VTK volume data file (you can specify 0..(n-1)
\li if you do not specify any option then all scalars and vectors will be built.
\li -threads number will specify the number of threads to use for VTK brick creation, if left out the default of 32 is used
\li -nocache will create VTK bricks with out storing or using a cache system. The cache system is much faster for mutliple scalars and vectors, but uses lots of memory
\li -hireslod will create a seperate VTK brick object for each level of detail. This should create a better quality rendering, but is slower to create.

<h2>Database Usage</h2>

With no command line args, or if LatticeFX was built without the optional
crunchstore dependency, shapeCreator writes output image data as files using
DBDisk. Files are written to the current working directory. To view the data,
run page-volume or page-volume-rt with the -dp option to specify the directory.

If LatticeFX is built with crunchstore, use the \c -cs option to specify the
database file full path. The path must exist but if the file does not it will be created.
\li -cs <dbFile> Write volume image data files using DBCrunchStore.

To view the data, run page-volume or page-volume-rt with the -cs option to
specify the crunchstore database file.

<h2>Other Options</h2>
\li -prune Do not generate empty subvolumes.

*/
