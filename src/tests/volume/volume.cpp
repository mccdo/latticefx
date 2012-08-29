/*************** <auto-copyright.rb BEGIN do not edit this line> *************
*
* latticeFX is (C) Copyright 2011-2012 by Ames Laboratory
*
* This library is free software; you can redistribute it and/or
* modify it under the terms of the GNU Library General Public
* License as published by the Free Software Foundation; 
* version 2.1 of the License.
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
*************** <auto-copyright.rb END do not edit this line> **************/

#include <latticefx/core/DataSet.h>
#include <latticefx/core/VolumeRenderer.h>
#include <latticefx/core/ChannelDataOSGImage.h>
#include <latticefx/core/TransferFunctionUtils.h>
#include <latticefx/core/Log.h>
#include <latticefx/core/LogMacros.h>

#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgGA/TrackballManipulator>
#include <osg/ClipNode>
#include <osg/MatrixTransform>
#include <osg/PolygonMode>

#include <osgwTools/Shapes.h>

#include <iostream>


using namespace lfx::core;


DataSetPtr prepareVolume( const std::string& fileName, const osg::Vec3& dims )
{
    osg::Image* image( osgDB::readImageFile( fileName ) );
    if( image == NULL )
    {
        LFX_ERROR_STATIC( "lfx.demo", "Can't read image from file \"" + fileName + "\"." );
        return( DataSetPtr( ( DataSet* )NULL ) );
    }
    if( image->getFileName().empty() )
    {
        // Images in ChannelDataOSGImage must have valid file name.
        image->setFileName( fileName );
    }

    ChannelDataOSGImagePtr volumeData( new ChannelDataOSGImage( "volumedata", image ) );

    DataSetPtr dsp( new DataSet() );
    dsp->addChannel( volumeData );

    VolumeRendererPtr renderOp( new VolumeRenderer() );
    renderOp->setVolumeDims( dims );
    renderOp->setPlaneSpacing( .3f );

    renderOp->addInput( "volumedata" );
    dsp->setRenderer( renderOp );

    renderOp->setTransferFunction( lfx::core::loadImageFromDat( "01.dat", LFX_ALPHA_RAMP_0_TO_1 ) );
    renderOp->setTransferFunctionDestination( Renderer::TF_RGBA );

    // Render when alpha values are greater than 0.15.
    renderOp->setHardwareMaskInputSource( Renderer::HM_SOURCE_ALPHA );
    renderOp->setHardwareMaskOperator( Renderer::HM_OP_GT );
    renderOp->setHardwareMaskReference( .15f );

    return( dsp );
}


int main( int argc, char** argv )
{
    Log::instance()->setPriority( Log::PrioInfo, Log::Console );
    Log::instance()->setPriority( Log::PrioTrace, "lfx.core.page" );

    osg::ArgumentParser arguments( &argc, argv );

    std::string fileName( "HeadVolume.dds" );
    arguments.read( "-f", fileName );

    osg::Vec3 dims( 50., 50., 50. );
    arguments.read( "-d", dims[0],dims[1],dims[2] );

    // Create an example data set.
    osg::Group* root (new osg::Group);
    root->getOrCreateStateSet()->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );

    DataSetPtr dsp( prepareVolume( fileName, dims ) );

    if( arguments.find( "-mt" ) > 0 )
    {
        // the translate will occur in the unscaled units, and the scaling will occur around the new origin.
        // A is just translate and nominal scale
        const osg::Matrix transformA = osg::Matrixd::scale( .5, .5, .5 ) *
                osg::Matrixd::translate(-75.0, -75.0, -75.0);
        osg::MatrixTransform* mtA( new osg::MatrixTransform( transformA ) );
        mtA->addChild( dsp->getSceneData() );
        root->addChild( mtA );

        // B: scale and rotate but no translate
        const osg::Matrix transformB = osg::Matrixd::rotate( osg::DegreesToRadians( 45.0 ), 0.0, 1.0, 0.0 ) *
                osg::Matrixd::scale( 2.0, 2.0, 2.0 );
        osg::MatrixTransform* mtB( new osg::MatrixTransform( transformB ) );
        mtB->addChild( dsp->getSceneData() );
        root->addChild( mtB );

        // C: translate, scale AND rotate
        // Note this is a non-uniform scale, performed after the rotation,
        // so the volume will be skewed.
        const osg::Matrix transformC = osg::Matrixd::rotate( osg::DegreesToRadians( 45.0 ), 1.0, 0.0, 0.0 ) *
                osg::Matrixd::scale( 2.0, 2.0, 1.0 ) *
                osg::Matrixd::translate( 100.0, 0.0, 0.0 );
        osg::MatrixTransform* mtC( new osg::MatrixTransform( transformC ) );
        mtC->addChild( dsp->getSceneData() );
        root->addChild( mtC );
        // Put wireframe boxes around volumes to test rotation and scaling of texture versus osg object
        // This requires a pre-built cube object of unit size.
        if( true )
        {
            osg::Geode* cubeNode( new osg::Geode() );
            cubeNode->addDrawable( osgwTools::makeWireBox( osg::Matrixd::scale( dims ), osg::Vec3( .5, .5, .5 ) ) );
            mtA->addChild( cubeNode );
            mtB->addChild( cubeNode );
            mtC->addChild( cubeNode );
        }
    }
    else
        root->addChild( dsp->getSceneData() );

    /*
    // Test hardware clip planes
    osg::ClipNode* cn( new osg::ClipNode() );
    cn->addClipPlane( new osg::ClipPlane( 0, 1., 0., 0., 3. ) );
    root->addChild( cn );
    root->getOrCreateStateSet()->setMode( GL_CLIP_PLANE0, osg::StateAttribute::ON );
    */
    
    osgViewer::Viewer viewer;
    viewer.setUpViewInWindow( 10, 30, 1200, 690 );
    viewer.setCameraManipulator( new osgGA::TrackballManipulator() );
    viewer.addEventHandler( new osgViewer::StatsHandler() );
    viewer.setSceneData( root );

    while( !( viewer.done() ) )
    {
        viewer.frame();
    }
    return( 0 );
}
