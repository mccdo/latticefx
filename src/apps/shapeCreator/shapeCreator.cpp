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

#include <latticefx/core/HierarchyUtils.h>

#include <latticefx/core/DBDisk.h>
#ifdef LFX_USE_CRUNCHSTORE
#  include <latticefx/core/DBCrunchStore.h>
#  include <crunchstore/DataManager.h>
#  include <crunchstore/NullCache.h>
#  include <crunchstore/NullBuffer.h>
#  include <crunchstore/SQLiteStore.h>
#endif

#include <latticefx/core/Log.h>
#include <latticefx/core/LogMacros.h>

#include <osg/ArgumentParser>
#include <osgViewer/Viewer>



const std::string logstr( "lfx.demo" );

using namespace lfx::core;



class CubeVolumeBrickData : public VolumeBrickData
{
public:
    CubeVolumeBrickData( const bool prune )
      : VolumeBrickData( prune ),
        _brickRes( 32, 32, 32 ),
        _cubeMin( .1, .1, .1 ),
        _cubeMax( .9, .9, .9 )
    {
        setNumBricks( osg::Vec3s( 4, 4, 4 ) );
    }
    ~CubeVolumeBrickData()
    {}

    virtual osg::Image* getBrick( const osg::Vec3s& brickNum ) const
    {
        const int idx( brickIndex( brickNum ) );
        if( ( idx < 0 ) || ( idx >= _images.size() ) )
            return( NULL );


        const osg::Vec3f brick( brickNum[0], brickNum[1], brickNum[2] );
        const osg::Vec3f invNumBricks( 1. / _numBricks[0], 1. / _numBricks[1], 1. / _numBricks[2] );

        // Compute brick min, max, and extents in normalized 0 <-> 1 volume coordinates.
        const osg::Vec3f brickMin( brick[0] * invNumBricks[0], brick[1] * invNumBricks[1], 
            brick[2] * invNumBricks[2] );
        const osg::Vec3f brickMax( brickMin + invNumBricks );
        const osg::Vec3f extent( brickMax - brickMin );

        unsigned char* data( new unsigned char[ _brickRes[0] * _brickRes[1] * _brickRes[2] ] );
        unsigned char* ptr( data );
        for( int rIdx=0; rIdx<_brickRes[2]; ++rIdx )
        {
            const float rVal( (float)rIdx/(float)_brickRes[2] * extent[2] + brickMin[2] );
            for( int tIdx=0; tIdx<_brickRes[1]; ++tIdx )
            {
                const float tVal( (float)tIdx/(float)_brickRes[1] * extent[1] + brickMin[1] );
                for( int sIdx=0; sIdx<_brickRes[0]; ++sIdx )
                {
                    const float sVal( (float)sIdx / (float)_brickRes[0] * extent[0] + brickMin[0] );

                    if( ( sVal >= _cubeMin[0] ) && ( sVal <= _cubeMax[0] ) &&
                        ( tVal >= _cubeMin[1] ) && ( tVal <= _cubeMax[1] ) &&
                        ( rVal >= _cubeMin[2] ) && ( rVal <= _cubeMax[2] ) )
                            *ptr++ = 255;
                    else
                            *ptr++ = 0;
                }
            }
        }

        osg::ref_ptr< osg::Image > image( new osg::Image() );
        image->setImage( _brickRes[0], _brickRes[1], _brickRes[2],
            GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE,
            (unsigned char*) data, osg::Image::USE_NEW_DELETE );
        return( image.release() );
    }

protected:
    osg::Vec3s _brickRes;
    osg::Vec3f _cubeMin, _cubeMax;
};

class SphereVolumeBrickData : public VolumeBrickData
{
public:
    SphereVolumeBrickData( const bool prune, const bool soft )
      : VolumeBrickData( prune ),
        _brickRes( 32, 32, 32 ),
        _center( .5, .5, .5 ),
        _minRad( soft ? .1 : .4 ),
        _maxRad( .45 )
    {
        setNumBricks( osg::Vec3s( 4, 4, 4 ) );
    }
    ~SphereVolumeBrickData()
    {}

    virtual osg::Image* getBrick( const osg::Vec3s& brickNum ) const
    {
        const int idx( brickIndex( brickNum ) );
        if( ( idx < 0 ) || ( idx >= _images.size() ) )
            return( NULL );


        const osg::Vec3f brick( brickNum[0], brickNum[1], brickNum[2] );
        const osg::Vec3f invNumBricks( 1. / _numBricks[0], 1. / _numBricks[1], 1. / _numBricks[2] );

        // Compute brick min, max, and extents in normalized 0 <-> 1 volume coordinates.
        const osg::Vec3f brickMin( brick[0] * invNumBricks[0], brick[1] * invNumBricks[1], 
            brick[2] * invNumBricks[2] );
        const osg::Vec3f brickMax( brickMin + invNumBricks );
        const osg::Vec3f extent( brickMax - brickMin );

        const float transition( _maxRad - _minRad );

        unsigned char* data( new unsigned char[ _brickRes[0] * _brickRes[1] * _brickRes[2] ] );
        unsigned char* ptr( data );
        for( int rIdx=0; rIdx<_brickRes[2]; ++rIdx )
        {
            const float rVal( (float)rIdx/(float)_brickRes[2] * extent[2] + brickMin[2] );
            for( int tIdx=0; tIdx<_brickRes[1]; ++tIdx )
            {
                const float tVal( (float)tIdx/(float)_brickRes[1] * extent[1] + brickMin[1] );
                for( int sIdx=0; sIdx<_brickRes[0]; ++sIdx )
                {
                    const float sVal( (float)sIdx / (float)_brickRes[0] * extent[0] + brickMin[0] );

                    const osg::Vec3f pt( sVal - _center[0], tVal - _center[1], rVal - _center[2] );
                    const float length( pt.length() );

                    if( length <= _minRad )
                        *ptr++ = 255;
                    else if( length > _maxRad )
                        *ptr++ = 0;
                    else
                    {
                        float norm( ( length - _minRad ) / transition );
                        *ptr++ = (unsigned char)( ( 1.f - norm ) * 255.f );
                    }
                }
            }
        }

        osg::ref_ptr< osg::Image > image( new osg::Image() );
        image->setImage( _brickRes[0], _brickRes[1], _brickRes[2],
            GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE,
            (unsigned char*) data, osg::Image::USE_NEW_DELETE );
        return( image.release() );
    }

protected:
    osg::Vec3s _brickRes;
    osg::Vec3f _center;
    float _minRad, _maxRad;
};


void createDataSet( const std::string& csFile, VolumeBrickData* shapeGen )
{
    SaveHierarchy* saver( new SaveHierarchy( shapeGen, "shapevolume" ) );

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
        try {
            cs->setDataManager( manager );
        }
        catch( std::exception exc ) {
            LFX_FATAL_STATIC( logstr, std::string(exc.what()) );
            LFX_FATAL_STATIC( logstr, "Unable to set DataManager." );
            exit( 1 );
        }

        saver->save( (DBBasePtr)cs );
    }
#endif
    if( csFile.empty() )
    {
        DBDiskPtr disk( DBDiskPtr( new DBDisk() ) );
        saver->save( (DBBasePtr)disk );
    }
}


int main( int argc, char** argv )
{
    Log::instance()->setPriority( Log::PrioInfo, Log::Console );
    Log::instance()->setPriority( Log::PrioInfo, "lfx.core.hier" );

    LFX_CRITICAL_STATIC( logstr, "With no command line args, write image data as files using DBDisk." );
    LFX_CRITICAL_STATIC( logstr, "-cs <dbFile> Write volume image data files using DBCrunchStore." );
    LFX_CRITICAL_STATIC( logstr, "-cube Generate a cube data set. This is the default if no other shape is specified." );
    LFX_CRITICAL_STATIC( logstr, "-sphere Generate a sphere data set." );
    LFX_CRITICAL_STATIC( logstr, "-ssphere Generate a soft sphere data set." );
    LFX_CRITICAL_STATIC( logstr, "-prune Do not generate empty subvolumes.." );

    osg::ArgumentParser arguments( &argc, argv );

    const bool prune( arguments.find( "-prune" ) > 0 );

    std::string csFile;
#ifdef LFX_USE_CRUNCHSTORE
    arguments.read( "-cs", csFile );
#endif

    bool softSphere( arguments.find( "-ssphere" ) > 0 );

    VolumeBrickData* shapeGen( NULL );
    if( ( arguments.find( "-sphere" ) > 0 ) || softSphere )
        shapeGen = new SphereVolumeBrickData( prune, softSphere );
    else
        shapeGen = new CubeVolumeBrickData( prune );

    createDataSet( csFile, shapeGen );

    return( 0 );
}
