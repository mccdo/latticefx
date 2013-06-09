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

#ifdef LFX_USE_CRUNCHSTORE
#  include <crunchstore/SQLiteStore.h>
#  include <latticefx/core/DBCrunchStore.h>
#  include <crunchstore/DataManager.h>
#  include <crunchstore/NullCache.h>
#  include <crunchstore/NullBuffer.h>
#endif

#include <latticefx/core/HierarchyUtils.h>
#include <latticefx/core/DBDisk.h>
#include <latticefx/core/Log.h>
#include <latticefx/core/LogMacros.h>

#include <osg/ArgumentParser>
#include <osgDB/FileUtils>
#include <osg/io_utils>

#ifdef VTK_FOUND
#  include "vtkCreator.h"
#endif

#include <latticefx/utils/CompilerGuards.h>
DIAG_OFF(unused-parameter)
#include <boost/date_time/posix_time/posix_time.hpp>
DIAG_ON(unused-parameter)

const std::string logstr( "lfx.demo" );
const std::string loginfo( logstr+".info" );

using namespace lfx::core;

void createDataSet( const std::string& csFile, VolumeBrickDataPtr shapeGen, const std::string &baseName );


class CubeVolumeBrickData : public VolumeBrickData
{
public:
    CubeVolumeBrickData( const bool prune, const bool soft )
        : VolumeBrickData( prune ),
          _brickRes( 32, 32, 32 ),
          _cubeMin( .2, .2, .2 ),
          _cubeMax( .8, .8, .8 ),
          _soft( soft )
    {}
    virtual ~CubeVolumeBrickData()
    {}

    virtual osg::Image* getBrick( const osg::Vec3s& brickNum ) const
    {
        const int idx( brickIndex( brickNum ) );
        if( ( idx < 0 ) || ( idx >= _images.size() ) )
        {
            return( NULL );
        }


        const osg::Vec3f brick( brickNum[0], brickNum[1], brickNum[2] );
        const osg::Vec3f invNumBricks( 1. / _numBricks[0], 1. / _numBricks[1], 1. / _numBricks[2] );

        // Compute brick min, max, and extents in normalized 0 <-> 1 volume coordinates.
        const osg::Vec3f brickMin( brick[0] * invNumBricks[0], brick[1] * invNumBricks[1],
                                   brick[2] * invNumBricks[2] );
        const osg::Vec3f brickMax( brickMin + invNumBricks );
        const osg::Vec3f extent( brickMax - brickMin );

        if( _prune && pruneTest( brickMin, brickMax ) )
        {
            return( NULL );
        }

        unsigned char* data( new unsigned char[ _brickRes[0] * _brickRes[1] * _brickRes[2] ] );
        unsigned char* ptr( data );
        for( int rIdx = 0; rIdx < _brickRes[2]; ++rIdx )
        {
            const float rVal( ( float )rIdx / ( float )_brickRes[2] * extent[2] + brickMin[2] );
            for( int tIdx = 0; tIdx < _brickRes[1]; ++tIdx )
            {
                const float tVal( ( float )tIdx / ( float )_brickRes[1] * extent[1] + brickMin[1] );
                for( int sIdx = 0; sIdx < _brickRes[0]; ++sIdx )
                {
                    const float sVal( ( float )sIdx / ( float )_brickRes[0] * extent[0] + brickMin[0] );

                    if( ( sVal >= _cubeMin[0] ) && ( sVal <= _cubeMax[0] ) &&
                            ( tVal >= _cubeMin[1] ) && ( tVal <= _cubeMax[1] ) &&
                            ( rVal >= _cubeMin[2] ) && ( rVal <= _cubeMax[2] ) )
                    {
                        if( !_soft )
                        {
                            *ptr++ = 255;
                        }
                        else
                        {
                            float val( sVal + tVal );
                            if( val > 1. )
                            {
                                val = 2. - val;
                            }
                            *ptr++ = ( unsigned char )( val * 255 );
                        }
                    }
                    else
                    {
                        *ptr++ = 0;
                    }
                }
            }
        }

        osg::ref_ptr< osg::Image > image( new osg::Image() );
        image->setImage( _brickRes[0], _brickRes[1], _brickRes[2],
                         GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE,
                         ( unsigned char* ) data, osg::Image::USE_NEW_DELETE );
        return( image.release() );
    }

protected:
    bool pruneTest( const osg::Vec3f& bMin, const osg::Vec3f& bMax ) const
    {
        return( ( bMin[0] > _cubeMax[0] ) ||
                ( bMax[0] < _cubeMin[0] ) ||
                ( bMin[1] > _cubeMax[1] ) ||
                ( bMax[1] < _cubeMin[1] ) ||
                ( bMin[2] > _cubeMax[2] ) ||
                ( bMax[2] < _cubeMin[2] )
              );
    }

    osg::Vec3s _brickRes;
    osg::Vec3f _cubeMin, _cubeMax;
    bool _soft;
};

class SphereVolumeBrickData : public VolumeBrickData
{
public:
    SphereVolumeBrickData( const bool prune, const bool soft )
        : VolumeBrickData( prune ),
          _brickRes( 32, 32, 32 ),
          _center( .5, .5, .5 ),
          _minRad( soft ? .1 : .45 ),
          _maxRad( .45 )
    {}
    virtual ~SphereVolumeBrickData()
    {}

    virtual osg::Image* getBrick( const osg::Vec3s& brickNum ) const
    {
        const int idx( brickIndex( brickNum ) );
        if( ( idx < 0 ) || ( idx >= _images.size() ) )
        {
            return( NULL );
        }


        const osg::Vec3f brick( brickNum[0], brickNum[1], brickNum[2] );
        const osg::Vec3f invNumBricks( 1. / _numBricks[0], 1. / _numBricks[1], 1. / _numBricks[2] );

        // Compute brick min, max, and extents in normalized 0 <-> 1 volume coordinates.
        const osg::Vec3f brickMin( brick[0] * invNumBricks[0], brick[1] * invNumBricks[1],
                                   brick[2] * invNumBricks[2] );
        const osg::Vec3f brickMax( brickMin + invNumBricks );
        const osg::Vec3f extent( brickMax - brickMin );

        if( _prune && pruneTest( brickMin, brickMax ) )
        {
            return( NULL );
        }

        const float transition( _maxRad - _minRad );

        unsigned char* data( new unsigned char[ _brickRes[0] * _brickRes[1] * _brickRes[2] ] );
        unsigned char* ptr( data );
        for( int rIdx = 0; rIdx < _brickRes[2]; ++rIdx )
        {
            const float rVal( ( float )rIdx / ( float )_brickRes[2] * extent[2] + brickMin[2] );
            for( int tIdx = 0; tIdx < _brickRes[1]; ++tIdx )
            {
                const float tVal( ( float )tIdx / ( float )_brickRes[1] * extent[1] + brickMin[1] );
                for( int sIdx = 0; sIdx < _brickRes[0]; ++sIdx )
                {
                    const float sVal( ( float )sIdx / ( float )_brickRes[0] * extent[0] + brickMin[0] );

                    const osg::Vec3f pt( sVal - _center[0], tVal - _center[1], rVal - _center[2] );
                    const float length( pt.length() );

                    if( length <= _minRad )
                    {
                        *ptr++ = 255;
                    }
                    else if( length > _maxRad )
                    {
                        *ptr++ = 0;
                    }
                    else
                    {
                        float norm( ( length - _minRad ) / transition );
                        *ptr++ = ( unsigned char )( ( 1.f - norm ) * 255.f );
                    }
                }
            }
        }

        osg::ref_ptr< osg::Image > image( new osg::Image() );
        image->setImage( _brickRes[0], _brickRes[1], _brickRes[2],
                         GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE,
                         ( unsigned char* ) data, osg::Image::USE_NEW_DELETE );
        return( image.release() );
    }

protected:
    bool pruneTest( const osg::Vec3f& bMin, const osg::Vec3f& bMax ) const
    {
        const osg::Vec3f p0( bMin[0] - .5f, bMin[1] - .5f, bMin[2] - .5f );
        const osg::Vec3f p1( bMax[0] - .5f, bMin[1] - .5f, bMin[2] - .5f );
        const osg::Vec3f p2( bMin[0] - .5f, bMax[1] - .5f, bMin[2] - .5f );
        const osg::Vec3f p3( bMax[0] - .5f, bMax[1] - .5f, bMin[2] - .5f );
        const osg::Vec3f p4( bMin[0] - .5f, bMin[1] - .5f, bMax[2] - .5f );
        const osg::Vec3f p5( bMax[0] - .5f, bMin[1] - .5f, bMax[2] - .5f );
        const osg::Vec3f p6( bMin[0] - .5f, bMax[1] - .5f, bMax[2] - .5f );
        const osg::Vec3f p7( bMax[0] - .5f, bMax[1] - .5f, bMax[2] - .5f );

        return( ( p0.length() > _maxRad ) &&
                ( p1.length() > _maxRad ) &&
                ( p2.length() > _maxRad ) &&
                ( p3.length() > _maxRad ) &&
                ( p4.length() > _maxRad ) &&
                ( p5.length() > _maxRad ) &&
                ( p6.length() > _maxRad ) &&
                ( p7.length() > _maxRad )
              );
    }

    osg::Vec3s _brickRes;
    osg::Vec3f _center;
    float _minRad, _maxRad;
};

class ConeVolumeBrickData : public VolumeBrickData
{
public:
    ConeVolumeBrickData( const bool prune )
        : VolumeBrickData( prune ),
          _brickRes( 32, 32, 32 ),
          _baseRad( .475 )
    {}
    virtual ~ConeVolumeBrickData()
    {}

    virtual osg::Image* getBrick( const osg::Vec3s& brickNum ) const
    {
        const int idx( brickIndex( brickNum ) );
        if( ( idx < 0 ) || ( idx >= _images.size() ) )
        {
            return( NULL );
        }


        const osg::Vec3f brick( brickNum[0], brickNum[1], brickNum[2] );
        const osg::Vec3f invNumBricks( 1. / _numBricks[0], 1. / _numBricks[1], 1. / _numBricks[2] );

        // Compute brick min, max, and extents in normalized 0 <-> 1 volume coordinates.
        const osg::Vec3f brickMin( brick[0] * invNumBricks[0], brick[1] * invNumBricks[1],
                                   brick[2] * invNumBricks[2] );
        const osg::Vec3f brickMax( brickMin + invNumBricks );
        const osg::Vec3f extent( brickMax - brickMin );

        if( _prune && pruneTest( brickMin, brickMax ) )
        {
            //std::cout << "Pruning " << brickNum << std::endl;
            return( NULL );
        }

        unsigned char* data( new unsigned char[ _brickRes[0] * _brickRes[1] * _brickRes[2] ] );
        unsigned char* ptr( data );
        for( int rIdx = 0; rIdx < _brickRes[2]; ++rIdx )
        {
            const float rVal( ( float )rIdx / ( float )_brickRes[2] * extent[2] + brickMin[2] );
            const float scaledRad( ( 1.f - rVal ) * _baseRad );
            for( int tIdx = 0; tIdx < _brickRes[1]; ++tIdx )
            {
                const float tVal( ( float )tIdx / ( float )_brickRes[1] * extent[1] + brickMin[1] );
                for( int sIdx = 0; sIdx < _brickRes[0]; ++sIdx )
                {
                    const float sVal( ( float )sIdx / ( float )_brickRes[0] * extent[0] + brickMin[0] );

                    const osg::Vec2f pt( sVal - .5f, tVal - .5f );
                    const float length( pt.length() );

                    if( length <= scaledRad )
                    {
                        *ptr++ = 255;
                    }
                    else
                    {
                        *ptr++ = 0;
                    }
                }
            }
        }

        osg::ref_ptr< osg::Image > image( new osg::Image() );
        image->setImage( _brickRes[0], _brickRes[1], _brickRes[2],
                         GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE,
                         ( unsigned char* ) data, osg::Image::USE_NEW_DELETE );
        return( image.release() );
    }

protected:
    bool pruneTest( const osg::Vec3f& bMin, const osg::Vec3f& bMax ) const
    {
        const osg::Vec2f p0( bMin[0] - .5f, bMin[1] - .5f );
        const osg::Vec2f p1( bMax[0] - .5f, bMin[1] - .5f );
        const osg::Vec2f p2( bMin[0] - .5f, bMax[1] - .5f );
        const osg::Vec2f p3( bMax[0] - .5f, bMax[1] - .5f );
        const float scaledRad( ( 1.f - bMin[2] ) * _baseRad );

        return( ( p0.length() > scaledRad ) &&
                ( p1.length() > scaledRad ) &&
                ( p2.length() > scaledRad ) &&
                ( p3.length() > scaledRad ) );
    }

    osg::Vec3s _brickRes;
    float _baseRad;
};

void createDataSet( const std::string& csFile, SaveHierarchy* saver )
{
	 // Configure database to use
#ifdef LFX_USE_CRUNCHSTORE
    if( !( csFile.empty() ) )
    {
        DBCrunchStorePtr cs( DBCrunchStorePtr( new DBCrunchStore() ) );

        crunchstore::DataManagerPtr manager( crunchstore::DataManagerPtr( new crunchstore::DataManager() ) );
        crunchstore::DataAbstractionLayerPtr cache( new crunchstore::NullCache );
        crunchstore::DataAbstractionLayerPtr buffer( new crunchstore::NullBuffer );
        manager->SetCache( cache );
        manager->SetBuffer( buffer );
        crunchstore::SQLiteStorePtr sqstore( new crunchstore::SQLiteStore );
        sqstore->SetStorePath( csFile );
        manager->AttachStore( sqstore, crunchstore::Store::BACKINGSTORE_ROLE );
        try
        {
            cs->setDataManager( manager );
        }
        catch( std::exception exc )
        {
            LFX_FATAL_STATIC( logstr, std::string( exc.what() ) );
            LFX_FATAL_STATIC( logstr, "Unable to set DataManager." );
            exit( 1 );
        }

        saver->save( ( DBBasePtr )cs );
    }
#endif
    if( csFile.empty() )
    {
        DBDiskPtr disk( DBDiskPtr( new DBDisk() ) );
        saver->save( ( DBBasePtr )disk );
    }
}

void createDataSet( const std::string& csFile, VolumeBrickDataPtr shapeGen, const std::string &baseName )
{
    //SaveHierarchy* saver( new SaveHierarchy( shapeGen, "shapevolume" ) );
	SaveHierarchy* saver( new SaveHierarchy( baseName ) );
	saver->addLevel( shapeGen );
	createDataSet( csFile, saver );
}


int main( int argc, char** argv )
{
    Log::instance()->setPriority( Log::PrioSilent, Log::Console );
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

    const bool prune( arguments.find( "-prune" ) > 0 );

    std::string csFile;
#ifdef LFX_USE_CRUNCHSTORE
    arguments.read( "-cs", csFile );
#endif

    bool softCube( arguments.find( "-scube" ) > 0 );
    bool softSphere( arguments.find( "-ssphere" ) > 0 );

    VolumeBrickDataPtr shapeGen;
    if( ( arguments.find( "-sphere" ) > 0 ) || softSphere )
    {
        shapeGen.reset(new SphereVolumeBrickData( prune, softSphere ));
    }
    else if( arguments.find( "-cone" ) > 0 )
    {
        shapeGen.reset(new ConeVolumeBrickData( prune ));
    }
#ifdef VTK_FOUND
    else if( arguments.find( "-vtk" ) > 0 )
    {
		VtkCreator vtk(logstr.c_str(), loginfo.c_str());
        return vtk.create(arguments, csFile);
    }
#endif
    else
    {
        shapeGen.reset(new CubeVolumeBrickData( prune, softCube ));
    }

    int depth( 4 );
    arguments.read( "-depth", depth );
    if( depth < 1 ) depth = 1;
    shapeGen->setDepth( depth );

    boost::posix_time::ptime start_time( boost::posix_time::microsec_clock::local_time() );
	createDataSet( csFile, shapeGen, std::string("shapevolume") );
    boost::posix_time::ptime end_time( boost::posix_time::microsec_clock::local_time() );
    boost::posix_time::time_duration diff = end_time - start_time;
    
    double createTime = diff.total_milliseconds() * 0.001;

    std::ostringstream ss;
    ss << "Time to create data set = " << createTime << " secs" << std::endl;
    LFX_CRITICAL_STATIC( logstr, ss.str().c_str() );

    return( 0 );
}



/** \page AppShapeCreator Application shapeCreator

shapeCreator generates sample volumetric data for use with the page-volume
and page-volume-rt test programs.

<h2>Hierarchy Depth Control</h2>

shapeCreator creates a hierarchy of volumetric levels of detail.
Larger values display more detail (if available in the source data)
at near viewing distances, but large values also create more data
files and increase shapeCreator runtime duration. Use the -depth command
line argument to control the hierarchy depth:

\li-depth <d> Hierarchy levels. Default is 4.

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
database file name.
\li -cs <dbFile> Write volume image data files using DBCrunchStore.

To view the data, run page-volume or page-volume-rt with the -cs option to
specify the crunchstore database file.

<h2>Other Options</h2>
\li -prune Do not generate empty subvolumes.

*/
