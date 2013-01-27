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

#include <latticefx/core/ChannelDataOSGArray.h>
#include <latticefx/core/ChannelDataOSGImage.h>
#include <latticefx/core/ChannelDataLOD.h>
#include <latticefx/core/ChannelDataImageSet.h>
#include <latticefx/core/HierarchyUtils.h>
#include <latticefx/core/Preprocess.h>
#include <latticefx/core/DataSet.h>
#include <latticefx/core/Renderer.h>
#include <latticefx/core/PagingThread.h>

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
#include <osg/Image>
#include <osg/Texture3D>
#include <osg/Texture2D>
#include <osg/io_utils>

#include <osgViewer/Viewer>
#include <osgGA/TrackballManipulator>

#include <sstream>
#include <iostream>


const std::string logstr( "lfx.demo" );


class VolumeBrickData
{
public:
    VolumeBrickData()
      : _numBricks( 0, 0, 0 )
    {}
    ~VolumeBrickData()
    {}

    void setNumBricks( const osg::Vec3s& numBricks )
    {
        _numBricks = numBricks;
        _images.resize( numBricks[0] * numBricks[1] * numBricks[2] );
    }
    osg::Vec3s getNumBricks() const
    {
        return( _numBricks );
    }

    void addBrick( const osg::Vec3s& brickNum, osg::Image* image )
    {
        const int idx( brickIndex( brickNum ) );
        if( ( idx < 0 ) || ( idx >= _images.size() ) )
            return;
        else
            _images[ idx ] = image;
    }
    virtual osg::Image* getBrick( const osg::Vec3s& brickNum ) const
    {
        const int idx( brickIndex( brickNum ) );
        if( ( idx < 0 ) || ( idx >= _images.size() ) )
            return( NULL );
        else
            return( _images[ idx ].get() );
    }
    osg::Image* getBrick( const std::string& brickName ) const
    {
        // Depth check
        short dim( 1 << brickName.length() );
        if( ( dim != _numBricks[0] ) || ( dim != _numBricks[1] ) || ( dim != _numBricks[2] ) )
            return( NULL );

        osg::Vec3s brickNum;
        if( brickName.empty() )
            return( getBrick( brickNum ) );

        osg::Vec3s half( _numBricks[0]/2, _numBricks[1]/2, _numBricks[2]/2 );
        for( int idx=0; idx < brickName.length(); ++idx )
        {
            int pos( (char)( brickName[ idx ] ) - 0 );
            brickNum[0] += ( pos & 0x1 ) ? half[0] : 0;
            brickNum[1] += ( pos & 0x2 ) ? half[1] : 0;
            brickNum[2] += ( pos & 0x4 ) ? half[2] : 0;
            half.set( half[0]/2, half[1]/2, half[2]/2 );
        }

        return( getBrick( brickNum ) );
    }

protected:
    int brickIndex( const osg::Vec3s& brickNum ) const
    {
        if( ( brickNum[0] >= _numBricks[0] ) || ( brickNum[0] < 0 ) ||
            ( brickNum[1] >= _numBricks[1] ) || ( brickNum[1] < 0 ) ||
            ( brickNum[2] >= _numBricks[2] ) || ( brickNum[2] < 0 ) )
                return( -1 );

        return( brickNum[0] * _numBricks[1] * _numBricks[2] +
            brickNum[1] * _numBricks[2] +
            brickNum[2] );
    }

    osg::Vec3s _numBricks;

    typedef std::vector< osg::ref_ptr< osg::Image > > ImageVector;
    ImageVector _images;
};

class CubeVolumeBrickData : public VolumeBrickData
{
public:
    CubeVolumeBrickData()
      : VolumeBrickData(),
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


class Downsampler
{
public:
    Downsampler( const VolumeBrickData* hiRes )
      : _hi( hiRes ),
        _low( NULL )
    {}
    ~Downsampler()
    {
    }

    /** \brief TBD
    \details
    Note that calling code is responsible for calling delete
    on the return value. */
    VolumeBrickData* getLow() const
    {
        if( _low )
            return( _low );
        _low = new VolumeBrickData();

        osg::Vec3s numBricks( _hi->getNumBricks() );
        numBricks[0] >>= 1;   if( numBricks[0] <= 1 ) numBricks[0] = 1;
        numBricks[1] >>= 1;   if( numBricks[1] <= 1 ) numBricks[1] = 1;
        numBricks[2] >>= 1;   if( numBricks[2] <= 1 ) numBricks[2] = 1;
        _low->setNumBricks( numBricks );

        for( int rIdx=0; rIdx<numBricks[2]; ++rIdx )
        {
            for( int tIdx=0; tIdx<numBricks[1]; ++tIdx )
            {
                for( int sIdx=0; sIdx<numBricks[0]; ++sIdx )
                {
                    osg::ref_ptr< osg::Image > i0( _hi->getBrick( osg::Vec3s( sIdx*2,   tIdx*2,   rIdx*2 ) ) );
                    osg::ref_ptr< osg::Image > i1( _hi->getBrick( osg::Vec3s( sIdx*2+1, tIdx*2,   rIdx*2 ) ) );
                    osg::ref_ptr< osg::Image > i2( _hi->getBrick( osg::Vec3s( sIdx*2,   tIdx*2+1, rIdx*2 ) ) );
                    osg::ref_ptr< osg::Image > i3( _hi->getBrick( osg::Vec3s( sIdx*2+1, tIdx*2+1, rIdx*2 ) ) );
                    osg::ref_ptr< osg::Image > i4( _hi->getBrick( osg::Vec3s( sIdx*2,   tIdx*2,   rIdx*2+1 ) ) );
                    osg::ref_ptr< osg::Image > i5( _hi->getBrick( osg::Vec3s( sIdx*2+1, tIdx*2,   rIdx*2+1 ) ) );
                    osg::ref_ptr< osg::Image > i6( _hi->getBrick( osg::Vec3s( sIdx*2,   tIdx*2+1, rIdx*2+1 ) ) );
                    osg::ref_ptr< osg::Image > i7( _hi->getBrick( osg::Vec3s( sIdx*2+1, tIdx*2+1, rIdx*2+1 ) ) );

                    osg::Image* image( sample( i0.get(), i1.get(), i2.get(), i3.get(),
                        i4.get(), i5.get(), i6.get(), i7.get() ) );
                    _low->addBrick( osg::Vec3s( sIdx, tIdx, rIdx ), image );
                }
            }
        }

        return( _low );
    }

protected:
    osg::Image* sample( const osg::Image* i0, const osg::Image* i1, const osg::Image* i2, const osg::Image* i3,
        const osg::Image* i4, const osg::Image* i5, const osg::Image* i6, const osg::Image* i7 ) const
    {
        osg::ref_ptr< osg::Image > image( new osg::Image() );

        unsigned char* data( new unsigned char[ i0->s() * i0->t() * i0->r() ] );
        image->setImage( i0->s(), i0->t(), i0->t(),
            GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE,
            (unsigned char*) data, osg::Image::USE_NEW_DELETE );
        unsigned char* ptr( data );

        for( int rIdx=0; rIdx<image->r(); ++rIdx )
        {
            int rBit( ( rIdx < image->r()/2 ) ? 0 : 1 );
            for( int tIdx=0; tIdx<image->t(); ++tIdx )
            {
                int tBit( ( tIdx < image->t()/2 ) ? 0 : 1 );
                for( int sIdx=0; sIdx<image->s(); ++sIdx )
                {
                    int sBit( ( sIdx < image->s()/2 ) ? 0 : 1 );
                    osg::Image* inImage;
                    switch( (rBit << 2) + (tBit << 1) + sBit )
                    {
                    case 0: inImage = const_cast< osg::Image* >( i0 ); break;
                    case 1: inImage = const_cast< osg::Image* >( i1 ); break;
                    case 2: inImage = const_cast< osg::Image* >( i2 ); break;
                    case 3: inImage = const_cast< osg::Image* >( i3 ); break;
                    case 4: inImage = const_cast< osg::Image* >( i4 ); break;
                    case 5: inImage = const_cast< osg::Image* >( i5 ); break;
                    case 6: inImage = const_cast< osg::Image* >( i6 ); break;
                    case 7: inImage = const_cast< osg::Image* >( i7 ); break;
                    }

                    const int s( sIdx % ( inImage->s() / 2 ) );
                    const int t( tIdx % ( inImage->t() / 2 ) );
                    const int r( rIdx % ( inImage->r() / 2 ) );
                    int pixel(
                        *( inImage->data( s*2,   t*2,   r*2 ) ) +
                        *( inImage->data( s*2+1, t*2,   r*2 ) ) +
                        *( inImage->data( s*2,   t*2+1, r*2 ) ) +
                        *( inImage->data( s*2+1, t*2+1, r*2 ) ) +
                        *( inImage->data( s*2,   t*2,   r*2+1 ) ) +
                        *( inImage->data( s*2+1, t*2,   r*2+1 ) ) +
                        *( inImage->data( s*2,   t*2+1, r*2+1 ) ) +
                        *( inImage->data( s*2+1, t*2+1, r*2+1 ) )
                        );
                    pixel >>= 3; // Divide by 8.
                    *ptr++ = (unsigned char)pixel;
                }
            }
        }

        return( image.release() );
    }

    const VolumeBrickData* _hi;
    mutable VolumeBrickData* _low;
};



using namespace lfx::core;

class HierarchyPreprocess : public Preprocess
{
public:
    HierarchyPreprocess( VolumeBrickData* base )
      : Preprocess(),
        _base( base )
    {
        setActionType( Preprocess::IGNORE_DATA );

        short xDim( _base->getNumBricks().x() );
        _depth = 0;
        while( xDim > 0 )
        {
            ++_depth;
            xDim >>= 1;
        }
    }
    ~HierarchyPreprocess()
    {
        for( unsigned int idx=0; idx < _lodVec.size()-1; ++idx )
        {
            delete _lodVec[ idx ];
        }
    }

    virtual ChannelDataPtr operator()()
    {
        if( !( _lodVec.empty() ) )
        {
            LFX_FATAL_STATIC( logstr, "Non-empty LOD vector." );
            return( ChannelDataPtr( (ChannelData*)NULL ) );
        }
        _lodVec.resize( _depth );

        _lodVec[ _depth-1 ] = _base;
        for( int depthIdx = _depth-1; depthIdx > 0; --depthIdx )
        {
            Downsampler ds( _lodVec[ depthIdx ] );
            _lodVec[ depthIdx-1 ] = ds.getLow();
        }


        AssembleHierarchy ah( _depth );
        ChannelDataPtr root( ah.getRoot() );
        recurseAddData( ah, std::string( "" ) );
        return( root );
    }
    
protected:
    ChannelDataPtr generateChannelData( const std::string& brickName, osg::Image* image )
    {
        const std::string baseName( "cubetexture" );
        const std::string channelName( "texture" );

        const std::string fileName( baseName + "-" + brickName + "-.ive" );
        image->setFileName( fileName );
        if( _db != NULL )
            _db->storeImage( image, fileName );

        ChannelDataOSGImagePtr cdip( ChannelDataOSGImagePtr( new ChannelDataOSGImage( channelName, image ) ) );
        cdip->setDBKey( fileName );
        cdip->reset();

        return( cdip );
    }
    
    void recurseAddData( AssembleHierarchy& ah, std::string& brickName )
    {
        const int depth( brickName.length() );
        VolumeBrickData* vbd( _lodVec[ depth ] );

        ChannelDataPtr brick( generateChannelData( brickName, vbd->getBrick( brickName ) ) );
        ah.addChannelData( brick, brickName );
        LFX_CRITICAL_STATIC( logstr, "Added brick " + brickName );

        if( depth < _depth-1 )
        {
            recurseAddData( ah, brickName + std::string( "0" ) );
            recurseAddData( ah, brickName + std::string( "1" ) );
            recurseAddData( ah, brickName + std::string( "2" ) );
            recurseAddData( ah, brickName + std::string( "3" ) );
            recurseAddData( ah, brickName + std::string( "4" ) );
            recurseAddData( ah, brickName + std::string( "5" ) );
            recurseAddData( ah, brickName + std::string( "6" ) );
            recurseAddData( ah, brickName + std::string( "7" ) );
        }
    }
    
    
    unsigned int _depth;

    VolumeBrickData* _base;

    typedef std::vector< VolumeBrickData* > LODVector;
    LODVector _lodVec;
};



DataSetPtr createDataSet( const std::string& csFile, const bool prune )
{
    DataSetPtr dsp( new DataSet() );
    
    HierarchyPreprocess* op( new HierarchyPreprocess( new CubeVolumeBrickData() ) );
    dsp->addPreprocess( PreprocessPtr( op ) );

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

        op->setDB( (DBBasePtr)cs );
    }
#endif
    if( csFile.empty() )
    {
        DBDiskPtr disk( DBDiskPtr( new DBDisk() ) );
        op->setDB( (DBBasePtr)disk );
    }

    return( dsp );
}

int main( int argc, char** argv )
{
    Log::instance()->setPriority( Log::PrioInfo, Log::Console );
    Log::instance()->setPriority( Log::PrioTrace, "lfx.db.cs" );

    LFX_CRITICAL_STATIC( logstr, "With no command line args, write image data as files using DBDisk." );
    LFX_CRITICAL_STATIC( logstr, "-cs <dbFile> Write volume image data files using DBCrunchStore." );

    osg::ArgumentParser arguments( &argc, argv );

    const bool prune( arguments.find( "-prune" ) > 0 );

    std::string csFile;
#ifdef LFX_USE_CRUNCHSTORE
    arguments.read( "-cs", csFile );
#endif

    DataSetPtr dsp( createDataSet( csFile, prune ) );
    dsp->updateAll();

    return( 0 );
}
