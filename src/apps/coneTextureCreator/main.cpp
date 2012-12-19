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

#define CONE_HEIGHT 250
#define CONE_RADIUS 125

#define SPHERE_RADIUS 128

#define TEXTURE_X 256
#define TEXTURE_Y 256
#define TEXTURE_Z 256

#define TEXTURE_HALF_X 128
#define TEXTURE_HALF_Y 128

#define SUBSAMPLE_SIZE 32

using namespace lfx::core;

class ImageProcess : public Preprocess
{
public:
    ImageProcess( unsigned int depth=3 )
      : Preprocess(),
        _depth( depth ),
        _prune( false )
    {
        setActionType( Preprocess::IGNORE_DATA );
        m_dataBB.set( -TEXTURE_HALF_X, -TEXTURE_HALF_Y,        0.,
                       TEXTURE_HALF_X,  TEXTURE_HALF_Y, TEXTURE_Z );
    }

    void setSparse( const bool prune ) { _prune = prune; }
    
    virtual ChannelDataPtr operator()()
    {
        //ChannelDataOSGImagePtr input( boost::static_pointer_cast< ChannelDataOSGImage >( _inputs[ 0 ] ) );
        const std::string dataName( "volumetexture" );//input->getName() );
        osg::Vec3d subOrigin( m_dataBB.xMin(), m_dataBB.yMin(), m_dataBB.zMin() );
        std::string brickNum;
        return( recurseBuildTree( 0, 0., 25000., subOrigin, brickNum ) );
    }
    
protected:
    ChannelDataOSGImagePtr generateImageData( const std::string& fileName, const unsigned int depth, const std::string& dataName, const std::string& brickNum )
    {
        std::ostringstream ostr;
        ostr << fileName << "-" << brickNum << "-" << ".ive";
        
        LFX_CRITICAL_STATIC( "lfx.coneTextureCreator", "Processing " + ostr.str() );

        if( _prune )
        {
            // Check for a NULL volume.
            const osg::Vec3d p0( m_brickBB._min );
            const osg::Vec3d p1( m_brickBB._max );
            const osg::Vec3d p2( p0.x(), p0.y(), p1.z() );
            const osg::Vec3d p3( p0.x(), p1.y(), p0.z() );
            const osg::Vec3d p4( p0.x(), p1.y(), p1.z() );
            const osg::Vec3d p5( p1.x(), p0.y(), p0.z() );
            const osg::Vec3d p6( p1.x(), p0.y(), p1.z() );
            const osg::Vec3d p7( p1.x(), p1.y(), p0.z() );
            if( !testVoxel( p0 ) && !testVoxel( p1 ) && !testVoxel( p2 ) && !testVoxel( p3 ) &&
                !testVoxel( p4 ) && !testVoxel( p5 ) && !testVoxel( p6 ) && !testVoxel( p7 ) &&
                !testVoxel( m_brickBB.center() ) )
            {
                // All corners are outside the cone. NULL volume.
                LFX_CRITICAL_STATIC( "lfx.coneTextureCreator", "\tNULL, skipping." );
                return( ChannelDataOSGImagePtr( (ChannelDataOSGImage*)NULL ) );
            }
        }


        //std::cout << " Depth " << depth << " " << m_brickBB.radius() << std::endl 
        //    << m_brickBB._min << std::endl
        //    << m_brickBB._max << std::endl;
        double factor = 1./double(SUBSAMPLE_SIZE - 1);
        double deltaX = (m_brickBB.xMax() - m_brickBB.xMin()) * factor; 
        double deltaY = (m_brickBB.yMax() - m_brickBB.yMin()) * factor; 
        double deltaZ = (m_brickBB.zMax() - m_brickBB.zMin()) * factor;
        //std::cout << deltaX << " " << deltaY << " " << deltaZ << std::endl;
        size_t numPixels = SUBSAMPLE_SIZE * SUBSAMPLE_SIZE * SUBSAMPLE_SIZE;
        unsigned char* pixels( new unsigned char[ numPixels ] );
        unsigned char* pixelPtr( pixels );
        double x, y, z;
        bool pixelsPresent = false;
        for( size_t k = 0; k < SUBSAMPLE_SIZE; ++k )
        {
            for( size_t j = 0; j < SUBSAMPLE_SIZE; ++j )
            {
                for( size_t i = 0; i < SUBSAMPLE_SIZE; ++i )
                {
                    x = m_brickBB.xMin() + i * deltaX;
                    y = m_brickBB.yMin() + j * deltaY;
                    z = m_brickBB.zMin() + k * deltaZ;

                    if( testVoxel( x, y, z ) )
                    {
                        *pixelPtr++ = 255;
                        pixelsPresent = true;
                    }
                    else
                    {
                        *pixelPtr++ = 0;
                    }
                }
            }
        }
        

        ChannelDataOSGImagePtr cdip;
        if( !pixelsPresent )
        {
            std::cout << "nothing in this brick" << std::endl;
        }

        const std::string imageName( ostr.str() );
        osg::ref_ptr< osg::Image > localImage = writeVoxel( pixels, imageName );
        localImage->setFileName( imageName );
        
        cdip = ChannelDataOSGImagePtr( new ChannelDataOSGImage( dataName, localImage.get() ) );
        cdip->setDBKey( imageName );
        cdip->reset();

        return( cdip );
    }
    
    ChannelDataPtr recurseBuildTree( unsigned int depth, const double minRange, const double maxRange, osg::Vec3d& brickOrigin, std::string& brickNum )
    {
        const std::string baseName( "conetexture" );
        const std::string channelName( "texture" );
        
        //Remember that 0.5^0 = 1 therefore this works for all depths to
        //partition the dataset properly.
        double factor = std::pow( 0.5, int(depth) );
        double x = (m_dataBB.xMax() - m_dataBB.xMin()) * factor; 
        double y = (m_dataBB.yMax() - m_dataBB.yMin()) * factor; 
        double z = (m_dataBB.zMax() - m_dataBB.zMin()) * factor;
        m_brickBB.set( brickOrigin.x(),     brickOrigin.y(),     brickOrigin.z(), 
                       brickOrigin.x() + x, brickOrigin.y() + y, brickOrigin.z() + z );

        if( depth == _depth )
            return( generateImageData( baseName, depth, channelName, brickNum ) );
        
        ChannelDataLODPtr cdLOD( new ChannelDataLOD( channelName ) );
        ChannelDataPtr brick = generateImageData( baseName, depth, channelName, brickNum );
        unsigned int channelIdx;
        if( brick.get() != NULL )
        {
            channelIdx = cdLOD->addChannel( brick );
            cdLOD->setRange( channelIdx, RangeValues( minRange, maxRange ) );
        }
        
        const unsigned int nextDepth( depth + 1 );
        // If nextDepth == _depth, then we're at the end, so set nextMax to FLT_MAX.
        // Otherwise, increase maxRange by 4 because the area of a circle circumscribing
        // a box goes up by a factor of 4 when the box edge double in size.
        const double nextMax( ( nextDepth == _depth ) ? FLT_MAX : ( maxRange * 4. ) );
        
        ChannelDataImageSetPtr cdImageSet( new ChannelDataImageSet( channelName ) );

        //We multiply by another 1/2 because we are really at nextDepth at this
        //point which is an additional depth down than what we are at above. We
        //need these new locations now to tell where we should be positioning
        //the subOrigins in world space for further sampling.
        x *= 0.5;
        y *= 0.5;
        z *= 0.5;
        //The numbers listed here follow the same order of the code in this
        //method:
        //  void AssembleHierarchy::addChannelData( ChannelDataPtr cdp, 
        //                                          const std::string nameString,
        //                                          const osg::Vec3& offset, 
        //                                          const unsigned int depth )
        //The numbers correspond to the appropriate octant that is paired with
        //the octants listed below.
        brickNum.append( "0" );
        osg::Vec3d subOrigin;
        subOrigin.set( brickOrigin.x(), 
                       brickOrigin.y(), 
                       brickOrigin.z() );
        brick = recurseBuildTree( nextDepth, 0., nextMax, subOrigin, brickNum );
        if( brick.get() != NULL )
        {
            channelIdx = cdImageSet->addChannel( brick );
            cdImageSet->setOffset( channelIdx, osg::Vec3( -1., -1., -1. ) );
        }
        brickNum.erase( brickNum.end() - 1 );

        brickNum.append( "1" );
        subOrigin.set( brickOrigin.x() + x, 
                       brickOrigin.y(), 
                       brickOrigin.z() );
        brick = recurseBuildTree( nextDepth, 0., nextMax, subOrigin, brickNum );
        if( brick.get() != NULL )
        {
            channelIdx = cdImageSet->addChannel( brick );
            cdImageSet->setOffset( channelIdx, osg::Vec3( 1., -1., -1. ) );
        }
        brickNum.erase( brickNum.end() - 1 );

        brickNum.append( "2" );
        subOrigin.set( brickOrigin.x(), 
                       brickOrigin.y() + y, 
                       brickOrigin.z() );
        brick = recurseBuildTree( nextDepth, 0., nextMax, subOrigin, brickNum );
        if( brick.get() != NULL )
        {
            channelIdx = cdImageSet->addChannel( brick );
            cdImageSet->setOffset( channelIdx, osg::Vec3( -1., 1., -1. ) );
        }
        brickNum.erase( brickNum.end() - 1 );

        brickNum.append( "3" );
        subOrigin.set( brickOrigin.x() + x, 
                       brickOrigin.y() + y, 
                       brickOrigin.z() );
        brick = recurseBuildTree( nextDepth, 0., nextMax, subOrigin, brickNum );
        if( brick.get() != NULL )
        {
            channelIdx = cdImageSet->addChannel( brick );
            cdImageSet->setOffset( channelIdx, osg::Vec3( 1., 1., -1. ) );
        }
        brickNum.erase( brickNum.end() - 1 );

        brickNum.append( "4" );
        subOrigin.set( brickOrigin.x(), 
                       brickOrigin.y(), 
                       brickOrigin.z() + z );
        brick = recurseBuildTree( nextDepth, 0., nextMax, subOrigin, brickNum );
        if( brick.get() != NULL )
        {
            channelIdx = cdImageSet->addChannel( brick );
            cdImageSet->setOffset( channelIdx, osg::Vec3( -1., -1., 1. ) );
        }
        brickNum.erase( brickNum.end() - 1 );

        brickNum.append( "5" );
        subOrigin.set( brickOrigin.x() + x, 
                       brickOrigin.y(), 
                       brickOrigin.z() + z );
        brick = recurseBuildTree( nextDepth, 0., nextMax, subOrigin, brickNum );
        if( brick.get() != NULL )
        {
            channelIdx = cdImageSet->addChannel( brick );
            cdImageSet->setOffset( channelIdx, osg::Vec3( 1., -1., 1. ) );
        }
        brickNum.erase( brickNum.end() - 1 );

        brickNum.append( "6" );
        subOrigin.set( brickOrigin.x(), 
                       brickOrigin.y() + y, 
                       brickOrigin.z() + z );
        brick = recurseBuildTree( nextDepth, 0., nextMax, subOrigin, brickNum );
        if( brick.get() != NULL )
        {
            channelIdx = cdImageSet->addChannel( brick );
            cdImageSet->setOffset( channelIdx, osg::Vec3( -1., 1., 1. ) );
        }
        brickNum.erase( brickNum.end() - 1 );

        brickNum.append( "7" );
        subOrigin.set( brickOrigin.x() + x, 
                       brickOrigin.y() + y, 
                       brickOrigin.z() + z );
        brick = recurseBuildTree( nextDepth, 0., nextMax, subOrigin, brickNum );
        if( brick.get() != NULL )
        {
            channelIdx = cdImageSet->addChannel( brick );
            cdImageSet->setOffset( channelIdx, osg::Vec3( 1., 1., 1. ) );
        }
        brickNum.erase( brickNum.end() - 1 );

        if( cdLOD->getNumChannels() > 0 )
        {
            if( cdImageSet->getNumChannels() > 0 )
            {
                channelIdx = cdLOD->addChannel( cdImageSet );
                // Regardless of the depth level, there are two LODs. The first is displayed
                // for range (0., maxRange), and the second is displayed for range
                // (maxRange, FLT_MAX). In this case, the second LOD is a hierarchy of
                // LODs that are displayed at subranges of (maxRange, FLT_MAX).
                cdLOD->setRange( channelIdx, RangeValues( maxRange, FLT_MAX ) );
            }
            return( cdLOD );
        }
        else
            return( ChannelDataPtr( (ChannelData*)NULL ) );
    }
    
    virtual bool testVoxel( const osg::Vec3d& p )
    {
        return( testVoxel( p.x(), p.y(), p.z() ) );
    }
    virtual bool testVoxel( const double x, const double y, const double z )
    {
        if( z >= CONE_HEIGHT )
            return false;
        
        //Since x and y are in world space there is no need to center the data
        //about 0,0,0
        const double radiusTest = double( (x * x) + (y * y) );
        
        const double coneConstant = double( CONE_RADIUS ) / double( CONE_HEIGHT );
        
        const double heightRadius = 
            coneConstant * coneConstant * ( z - CONE_HEIGHT ) * ( z - CONE_HEIGHT );
        //double heightRadius =
        //    ( double( CONE_RADIUS ) * double( z ) / double( CONE_HEIGHT ) );
        //heightRadius *= heightRadius;
        
        return( heightRadius >= radiusTest );
    }
    
    osg::Image* writeVoxel( unsigned char* pixels, const std::string& filename )
    {
        osg::ref_ptr< osg::Image > image = new osg::Image();
        //We will let osg manage the raw image data
        image->setImage( SUBSAMPLE_SIZE, SUBSAMPLE_SIZE, SUBSAMPLE_SIZE, 
                         GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE,
                         pixels, osg::Image::USE_NEW_DELETE );

        if( _db != NULL )
            _db->storeImage( image.get(), filename );

        return image.release();
    }
    
    unsigned int _depth;
    bool _prune;
    osg::BoundingBoxd m_dataBB;
    osg::BoundingBoxd m_brickBB;
};

class CircleImageProcess : public ImageProcess
{
public:
    CircleImageProcess( unsigned int depth=3 )
        : 
        ImageProcess( depth )
    {
        setActionType( Preprocess::REPLACE_DATA );
        m_dataBB.set( -TEXTURE_HALF_X, -TEXTURE_HALF_Y,        0.,
                       TEXTURE_HALF_X,  TEXTURE_HALF_Y, TEXTURE_Z );
    }
    
    virtual ChannelDataPtr operator()()
    {
        //ChannelDataOSGImagePtr input( boost::static_pointer_cast< ChannelDataOSGImage >( _inputs[ 0 ] ) );
        const std::string dataName( "volumetexture" );//input->getName() );
        osg::Vec3d subOrigin( m_dataBB.xMin(), m_dataBB.yMin(), m_dataBB.zMin() );
        std::string brickNum;
        return( recurseBuildTree( 0, 0., 25000., subOrigin, brickNum ) );
    }
    
protected:
    virtual bool testVoxel( const double x, const double y, const double z )
    {
        if( y >= SPHERE_RADIUS || x >= SPHERE_RADIUS )
            return false;
        
        //Since x and y are in world space there is no need to center the data
        //about 0,0,0. We do need to sift the sphere up by the radius so that
        //it fits in the volume correctly.
        const double radiusTest = double( (x * x) + (y * y) + ( (z - SPHERE_RADIUS) * (z - SPHERE_RADIUS) ) );
        
        const double sphereConstant = double( SPHERE_RADIUS ) * double( SPHERE_RADIUS );
                
        return( sphereConstant >= radiusTest );
    }
};

class CylinderImageProcess : public ImageProcess
{
public:
    CylinderImageProcess( unsigned int depth=3 )
    : 
    ImageProcess( depth )
    {
        setActionType( Preprocess::REPLACE_DATA );
        m_dataBB.set( -TEXTURE_HALF_X, -TEXTURE_HALF_Y,        0.,
                       TEXTURE_HALF_X,  TEXTURE_HALF_Y, TEXTURE_Z );
    }
    
    virtual ChannelDataPtr operator()()
    {
        //ChannelDataOSGImagePtr input( boost::static_pointer_cast< ChannelDataOSGImage >( _inputs[ 0 ] ) );
        const std::string dataName( "volumetexture" );//input->getName() );
        osg::Vec3d subOrigin( m_dataBB.xMin(), m_dataBB.yMin(), m_dataBB.zMin() );
        std::string brickNum;
        return( recurseBuildTree( 0, 0., 25000., subOrigin, brickNum ) );
    }
    
protected:
    virtual bool testVoxel( const double x, const double y, const double z )
    {
        if( y >= SPHERE_RADIUS || x >= SPHERE_RADIUS )
            return false;
        
        //Since x and y are in world space there is no need to center the data
        //about 0,0,0. We do need to sift the sphere up by the radius so that
        //it fits in the volume correctly.
        const double radiusTest = double( (x * x) + (y * y) );
        
        const double sphereConstant = double( SPHERE_RADIUS ) * double( SPHERE_RADIUS );
        
        return( sphereConstant >= radiusTest );
    }
};


DataSetPtr createDataSet( const std::string& csFile, const bool prune )
{
    /*const std::string baseFileName( "pagetex-near0.png" );
    osg::Image* image( osgDB::readImageFile( baseFileName ) );
    image->setFileName( baseFileName );
    ChannelDataOSGImagePtr imageData( new ChannelDataOSGImage( "texture", image ) );
    if( DBUsesCrunchStore() )
    {
        imageData->setStorageModeHint( ChannelData::STORE_IN_DB );
        imageData->setDBKey( baseFileName );
    }*/
    
    DataSetPtr dsp( new DataSet() );
    //We add a dummy dataset just for the purpose of being able to generate
    //the test bricks.
    //dsp->addChannel( imageData );
    
    ImageProcess* op( new ImageProcess );
    op->setSparse( prune );

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
            LFX_FATAL_STATIC( "lfx.demo", std::string(exc.what()) );
            LFX_FATAL_STATIC( "lfx.demo", "Unable to set DataManager." );
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

    //op->addInput( "texture" );
    dsp->addPreprocess( PreprocessPtr( op ) );
    
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

#if 0
    size_t numPixels = TEXTURE_X * TEXTURE_Y * TEXTURE_Z;
    unsigned char* pixels( new unsigned char[ numPixels ] );
    unsigned char* pixelPtr( pixels );
    for( size_t k = 0; k < TEXTURE_Z; ++k )
    {
        for( size_t j = 0; j < TEXTURE_Y; ++j )
        {
            for( size_t i = 0; i < TEXTURE_X; ++i )
            {
                *pixelPtr++ = ( testVoxel( i, j, k ) ? 255 : 0 );
            }
        }
    }

    writeVoxel( numPixels, pixels );
#else
    DataSetPtr dsp( createDataSet( csFile, prune ) );
    
    dsp->updateAll();
#endif
    //OSG handles the memory
    //delete [] pixels;
    return 0;
}
