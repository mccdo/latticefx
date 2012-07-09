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

#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <osgGA/TrackballManipulator>
#include <osg/ClipNode>
#include <osg/MatrixTransform>

#include <iostream>




lfx::DataSetPtr prepareVolume( const std::string& fileName, const osg::Vec3& dims )
{
    osg::Image* image( osgDB::readImageFile( fileName ) );
    if( image == NULL )
    {
        OSG_FATAL << "Can't read image from file \"" << fileName << "\"." << std::endl;
        return( lfx::DataSetPtr( ( lfx::DataSet* )NULL ) );
    }

    lfx::ChannelDataOSGImagePtr volumeData( new lfx::ChannelDataOSGImage( "volumedata", image ) );

    lfx::DataSetPtr dsp( new lfx::DataSet() );
    dsp->addChannel( volumeData );

    lfx::VolumeRendererPtr renderOp( new lfx::VolumeRenderer() );
    renderOp->setVolumeDims( dims );
    renderOp->setPlaneSpacing( .3f );
    renderOp->addInput( "volumedata" );
    dsp->setRenderer( renderOp );

    renderOp->setTransferFunction( lfx::loadImageFromDat( "02.dat", LFX_ALPHA_RAMP_0_TO_1 ) );
    renderOp->setTransferFunctionInput( "" );
    renderOp->setTransferFunctionDestination( lfx::Renderer::TF_ALPHA );

    renderOp->setHardwareMaskInputSource( lfx::Renderer::HM_SOURCE_ALPHA );
    renderOp->setHardwareMaskReference( 0. );
    renderOp->setHardwareMaskOperator( lfx::Renderer::HM_OP_GT );

    return( dsp );
}


int main( int argc, char** argv )
{
    osg::ArgumentParser arguments( &argc, argv );

    std::string fileName( "HeadVolume.dds" );
    arguments.read( "-f", fileName );

    osg::Vec3 dims( 50., 50., 50. );
    arguments.read( " -d", dims[0],dims[1],dims[2] );

    // Create an example data set.
	osg::Group* root (new osg::Group);
    root->getOrCreateStateSet()->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );

    lfx::DataSetPtr dsp( prepareVolume( fileName, dims ) );

    if( arguments.find( "-mt" ) > 0 )
    {
        osg::MatrixTransform* mtA( new osg::MatrixTransform );
	    osg::MatrixTransform* mtB( new osg::MatrixTransform );
	    osg::MatrixTransform* mtC( new osg::MatrixTransform );
	    // Test Matrix position/scale
	    osg::Matrix transformA, transformB, transformC;
	    // the translate will occur in the unscaled units, and the scaling will occur around the new origin.
	    // A is just translate and nominal scale
	    transformA *= osg::Matrixd::scale(1.0, 1.0, 0.5);
	    transformA *= osg::Matrixd::translate(-50.0, 0.0, 0.0);
	    mtA->setMatrix(transformA);
	    // B: scale and rotate but no translate
	    transformB *= osg::Matrixd::rotate(osg::DegreesToRadians(45.0), 0.0, 1.0, 0.0); // 45 degrees about +Y axis
	    transformB *= osg::Matrixd::scale(2.0, 2.0, 2.0);
	    transformB *= osg::Matrixd::translate(0.0, 0.0, 0.0);
	    mtB->setMatrix(transformB);
	    // C: translate, scale AND rotate
	    transformC *= osg::Matrixd::rotate(osg::DegreesToRadians(45.0), 1.0, 0.0, 0.0); // 45 degrees about +X axis
	    transformC *= osg::Matrixd::scale(2.0, 2.0, 1.0);
	    transformC *= osg::Matrixd::translate(50.0, 0.0, 0.0);
	    mtC->setMatrix(transformC);
	    root->addChild(mtA);
	    root->addChild(mtB);
	    root->addChild(mtC);
	    // multi-parent the same scene data
	    mtA->addChild( dsp->getSceneData() );
	    mtB->addChild( dsp->getSceneData() );
	    mtC->addChild( dsp->getSceneData() );
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
    viewer.setUpViewInWindow( 10, 30, 800, 440 );
    viewer.setCameraManipulator( new osgGA::TrackballManipulator() );
    viewer.setSceneData( root );

    while( !( viewer.done() ) )
    {
        viewer.frame();
    }
    return( 0 );
}
