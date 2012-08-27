/*************** <auto-copyright.rb BEGIN do not edit this line> **************
 *
 * VE-Suite is (C) Copyright 1998-2012 by Iowa State University
 *
 * Original Development Team:
 *   - ISU's Thermal Systems Virtual Engineering Group,
 *     Headed by Kenneth Mark Bryden, Ph.D., www.vrac.iastate.edu/~kmbryden
 *   - Reaction Engineering International, www.reaction-eng.com
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
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
 * -----------------------------------------------------------------
 * Date modified: $Date$
 * Version:       $Rev$
 * Author:        $Author$
 * Id:            $Id$
 * -----------------------------------------------------------------
 *
 *************** <auto-copyright.rb END do not edit this line> ***************/
#include <latticefx/core/ChannelDataOSGArray.h>
#include <latticefx/core/ChannelDataOSGImage.h>
#include <latticefx/core/ChannelDataLOD.h>
#include <latticefx/core/ChannelDataImageSet.h>
#include <latticefx/core/Preprocess.h>
#include <latticefx/core/DBUtils.h>
#include <latticefx/core/DataSet.h>
#include <latticefx/core/Renderer.h>
#include <latticefx/core/PagingThread.h>

#include <latticefx/core/Log.h>
#include <latticefx/core/LogMacros.h>

#include <osg/Image>
#include <osg/Texture3D>
#include <osg/Texture2D>
#include <osg/io_utils>

#include <osgDB/WriteFile>
#include <osgDB/ReadFile>

#include <osgViewer/Viewer>
#include <osgGA/TrackballManipulator>

#include <sstream>
#include <iostream>

#define CONE_HEIGHT 250
#define CONE_RADIUS 125

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
        : 
        Preprocess(),
        _depth( depth )
    {
        setActionType( Preprocess::REPLACE_DATA );
        m_dataBB.set( -TEXTURE_HALF_X, -TEXTURE_HALF_Y,        0.,
                       TEXTURE_HALF_X,  TEXTURE_HALF_Y, TEXTURE_Z );
    }
    
    virtual ChannelDataPtr operator()()
    {
        ChannelDataOSGImagePtr input( boost::static_pointer_cast< ChannelDataOSGImage >( _inputs[ 0 ] ) );
        const std::string dataName( input->getName() );
        osg::Vec3d subOrigin( m_dataBB.xMin(), m_dataBB.yMin(), m_dataBB.zMin() );
        return( recurseBuildTree( 0, 0., 25000., subOrigin ) );
    }
    
protected:
    ChannelDataOSGImagePtr generateImageData( const std::string& fileName, const unsigned int depth, const std::string& dataName )
    {
        std::ostringstream ostr;
        ostr << fileName << depth << "_" << m_brickBB.xMin() << "_" 
            << m_brickBB.yMin() << "_" << m_brickBB.zMin() << ".png";
        
        std::cout << " Depth " << depth << " " << m_brickBB.radius() << std::endl 
            << m_brickBB.xMin() << " " <<  m_brickBB.yMin() << " " <<   m_brickBB.zMin() << std::endl
            << m_brickBB.xMax() << " " <<  m_brickBB.yMax() << " " <<   m_brickBB.zMax() << std::endl;
        double factor = 1./double(SUBSAMPLE_SIZE - 1);
        double deltaX = (m_dataBB.xMax() - m_dataBB.xMin()) * factor; 
        double deltaY = (m_dataBB.yMax() - m_dataBB.yMin()) * factor; 
        double deltaZ = (m_dataBB.zMax() - m_dataBB.zMin()) * factor;

        size_t numPixels = SUBSAMPLE_SIZE * SUBSAMPLE_SIZE * SUBSAMPLE_SIZE;
        unsigned char* pixels( new unsigned char[ numPixels ] );
        unsigned char* pixelPtr( pixels );
        double x, y, z;
        for( size_t k = 0; k < SUBSAMPLE_SIZE; ++k )
        {
            for( size_t j = 0; j < SUBSAMPLE_SIZE; ++j )
            {
                for( size_t i = 0; i < SUBSAMPLE_SIZE; ++i )
                {
                    x = m_brickBB.xMin() + i * deltaX;
                    y = m_brickBB.yMin() + j * deltaY;
                    z = m_brickBB.zMin() + k * deltaZ;

                    *pixelPtr++ = ( testVoxel( x, y, z ) ? 255 : 0 );
                }
            }
        }
        
        const std::string imageName( ostr.str() );
        osg::ref_ptr< osg::Image > localImage = writeVoxel( pixels, imageName );
        localImage->setFileName( imageName );
        
        ChannelDataOSGImagePtr cdip( new ChannelDataOSGImage( dataName, localImage ) );
        if( DBUsesCrunchStore() )
        {
            cdip->setStorageModeHint( ChannelData::STORE_IN_DB );
            cdip->setDBKey( imageName );
            cdip->reset();
        }
        return( cdip );
    }
    
    ChannelDataPtr recurseBuildTree( unsigned int depth, const double minRange, const double maxRange, osg::Vec3d& brickOrigin )
    {
        const std::string baseName( "pagetex-near" );
        const std::string channelName( "texture" );
        
        double factor = std::pow( 0.5, int(depth) );
        double x = (m_dataBB.xMax() - m_dataBB.xMin()) * factor; 
        double y = (m_dataBB.yMax() - m_dataBB.yMin()) * factor; 
        double z = (m_dataBB.zMax() - m_dataBB.zMin()) * factor;
        m_brickBB.set( brickOrigin.x(),     brickOrigin.y(),     brickOrigin.z(), 
                       brickOrigin.x() + x, brickOrigin.y() + y, brickOrigin.z() + z );

        if( depth == _depth )
            return( generateImageData( baseName, depth, channelName ) );
        
        ChannelDataLODPtr cdLOD( new ChannelDataLOD( channelName ) );
        unsigned int channelIdx( cdLOD->addChannel( generateImageData( baseName, depth, channelName ) ) );
        cdLOD->setRange( channelIdx, RangeValues( minRange, maxRange ) );
        
        const unsigned int nextDepth( depth + 1 );
        // If nextDepth == _depth, then we're at the end, so set nextMax to FLT_MAX.
        // Otherwise, increase maxRange by 4 because the area of a circle circumscribing
        // a box goes up by a factor of 4 when the box edge double in size.
        const double nextMax( ( nextDepth == _depth ) ? FLT_MAX : ( maxRange * 4. ) );
        
        ChannelDataImageSetPtr cdImageSet( new ChannelDataImageSet( channelName ) );

      
        ChannelDataPtr brick;
        x *= 0.5;
        y *= 0.5;
        z *= 0.5;
        osg::Vec3d subOrigin;
        subOrigin.set( brickOrigin.x(), 
                       brickOrigin.y(), 
                       brickOrigin.z() );
        brick = recurseBuildTree( nextDepth, 0., nextMax, subOrigin );
        channelIdx = cdImageSet->addChannel( brick );
        cdImageSet->setOffset( channelIdx, osg::Vec3( -1., -1., -1. ) );

        subOrigin.set( brickOrigin.x() + x, 
                       brickOrigin.y(), 
                       brickOrigin.z() );
        brick = recurseBuildTree( nextDepth, 0., nextMax, subOrigin );
        channelIdx = cdImageSet->addChannel( brick );
        cdImageSet->setOffset( channelIdx, osg::Vec3( 1., -1., -1. ) );
        
        subOrigin.set( brickOrigin.x(), 
                       brickOrigin.y() + y, 
                       brickOrigin.z() );
        brick = recurseBuildTree( nextDepth, 0., nextMax, subOrigin );
        channelIdx = cdImageSet->addChannel( brick );
        cdImageSet->setOffset( channelIdx, osg::Vec3( -1., 1., -1. ) );
        
        subOrigin.set( brickOrigin.x() + x, 
                       brickOrigin.y() + y, 
                       brickOrigin.z() );
        brick = recurseBuildTree( nextDepth, 0., nextMax, subOrigin );
        channelIdx = cdImageSet->addChannel( brick );
        cdImageSet->setOffset( channelIdx, osg::Vec3( 1., 1., -1. ) );
        
        subOrigin.set( brickOrigin.x(), 
                       brickOrigin.y(), 
                       brickOrigin.z() + z );
        brick = recurseBuildTree( nextDepth, 0., nextMax, subOrigin );
        channelIdx = cdImageSet->addChannel( brick );
        cdImageSet->setOffset( channelIdx, osg::Vec3( -1., -1., 1. ) );
        
        subOrigin.set( brickOrigin.x() + x, 
                       brickOrigin.y(), 
                       brickOrigin.z() + z );
        brick = recurseBuildTree( nextDepth, 0., nextMax, subOrigin );
        channelIdx = cdImageSet->addChannel( brick );
        cdImageSet->setOffset( channelIdx, osg::Vec3( 1., -1., 1. ) );
        
        subOrigin.set( brickOrigin.x(), 
                       brickOrigin.y() + y, 
                       brickOrigin.z() + z );
        brick = recurseBuildTree( nextDepth, 0., nextMax, subOrigin );
        channelIdx = cdImageSet->addChannel( brick );
        cdImageSet->setOffset( channelIdx, osg::Vec3( -1., 1., 1. ) );
        
        subOrigin.set( brickOrigin.x() + x, 
                       brickOrigin.y() + y, 
                       brickOrigin.z() + z );
        brick = recurseBuildTree( nextDepth, 0., nextMax, subOrigin );
        channelIdx = cdImageSet->addChannel( brick );
        cdImageSet->setOffset( channelIdx, osg::Vec3( 1., 1., 1. ) );
        
        // Regardless of the depth level, there are two LODs. The first is displayed
        // for range (0., maxRange), and the second is displayed for range
        // (maxRange, FLT_MAX). In this case, the second LOD is a hierarchy of
        // LODs that are displayed at subranges of (maxRange, FLT_MAX).
        channelIdx = cdLOD->addChannel( cdImageSet );
        cdLOD->setRange( channelIdx, RangeValues( maxRange, FLT_MAX ) );
        return( cdLOD );
    }
    
    bool testVoxel( const double x, const double y, const double z )
    {
        if( z >= CONE_HEIGHT )
            return false;
        
        const double radiusTest = double( (x - TEXTURE_HALF_X) * (x - TEXTURE_HALF_X) +
                                         (y - TEXTURE_HALF_Y) * (y - TEXTURE_HALF_Y) );
        
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
        image->setImage( SUBSAMPLE_SIZE, SUBSAMPLE_SIZE, SUBSAMPLE_SIZE, GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE,
                        pixels, osg::Image::USE_NEW_DELETE );
        
        //osg::Texture3D* texture = new osg::Texture3D( image );
        //texture->setFilter( osg::Texture::MIN_FILTER, osg::Texture2D::NEAREST );
        //texture->setFilter( osg::Texture::MAG_FILTER, osg::Texture2D::NEAREST );
        
        osgDB::writeImageFile( *image, filename );
        return image.release();
    }
    
    unsigned int _depth;
    osg::BoundingBoxd m_dataBB;
    osg::BoundingBoxd m_brickBB;
};

bool testVoxel( const int x, const int y, const int z )
{
    if( z >= CONE_HEIGHT )
        return false;

    const double radiusTest = double( (x - TEXTURE_HALF_X) * (x - TEXTURE_HALF_X) +
        (y - TEXTURE_HALF_Y) * (y - TEXTURE_HALF_Y) );
    
    const double coneConstant = double( CONE_RADIUS ) / double( CONE_HEIGHT );
    
    const double heightRadius = 
       coneConstant * coneConstant * ( z - CONE_HEIGHT ) * ( z - CONE_HEIGHT );
    //double heightRadius =
    //    ( double( CONE_RADIUS ) * double( z ) / double( CONE_HEIGHT ) );
    //heightRadius *= heightRadius;
    
    return( heightRadius >= radiusTest );
}

void writeVoxel( const size_t numPixels, unsigned char* pixels )
{
    osg::ref_ptr< osg::Image > image = new osg::Image();
    //We will let osg manage the raw image data
    image->setImage( TEXTURE_X, TEXTURE_Y, TEXTURE_Z, GL_LUMINANCE, GL_LUMINANCE, GL_UNSIGNED_BYTE,
                pixels, osg::Image::USE_NEW_DELETE );

    //osg::Texture3D* texture = new osg::Texture3D( image );
    //texture->setFilter( osg::Texture::MIN_FILTER, osg::Texture2D::NEAREST );
    //texture->setFilter( osg::Texture::MAG_FILTER, osg::Texture2D::NEAREST );
    
    osgDB::writeImageFile( *image, "cone_texture.ive" );
}

DataSetPtr createDataSet()
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
    //dsp->addChannel( imageData );
    
    ImageProcess* op( new ImageProcess );
    op->addInput( "texture" );
    dsp->addPreprocess( PreprocessPtr( op ) );
    
    /*BoxRenderer* renderOp( new BoxRenderer );
    renderOp->setVolumeDims( osg::Vec3( TEXTURE_X, TEXTURE_Y, TEXTURE_Z ) );
    renderOp->setVolumeOrigin( osg::Vec3( -TEXTURE_HALF_X, -TEXTURE_HALF_Y, 0. ) );
    renderOp->addInput( "texture" );
    dsp->setRenderer( RendererPtr( renderOp ) );*/
    
    return( dsp );
}

int main( int argc, char** argv )
{
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
    Log::instance()->setPriority( Log::PrioInfo, Log::Console );

    DataSetPtr dsp( createDataSet() );
    
    osgViewer::Viewer viewer;
    viewer.setUpViewInWindow( 20, 30, 800, 460 );
    osg::ref_ptr< osgGA::TrackballManipulator > tbm( new osgGA::TrackballManipulator() );
    viewer.setCameraManipulator( tbm.get() );
    
    osg::Group* root( new osg::Group );
    root->addChild( dsp->getSceneData() );
    root->addChild( osgDB::readNodeFile( "axes.osg" ) );
    viewer.setSceneData( root );
    tbm->home( 0. );
    
    // Test hierarchy utils
    //MyHierarchyDB myDB;
    //traverseHeirarchy( dsp->getChannel( "texture" ), myDB );
    
    // Really we would need to change the projection matrix and viewport
    // in an event handler that catches window size changes. We're cheating.
    PagingThread* pageThread( PagingThread::instance() );
    const osg::Camera* cam( viewer.getCamera() );
    pageThread->setTransforms( cam->getProjectionMatrix(), cam->getViewport() );
    
    osg::Vec3d eye, center, up;
    while( !viewer.done() )
    {
        tbm->getInverseMatrix().getLookAt( eye, center, up );
        pageThread->setTransforms( osg::Vec3( eye ) );
        viewer.frame();
    }
    return( 0 );
#endif
    //OSG handles the memory
    //delete [] pixels;
    return 0;
}
