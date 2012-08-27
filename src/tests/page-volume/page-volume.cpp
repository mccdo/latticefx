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
#include <latticefx/core/HierarchyUtils.h>
#include <latticefx/core/TransferFunctionUtils.h>
#include <latticefx/core/Log.h>
#include <latticefx/core/LogMacros.h>

#include <osgViewer/Viewer>
#include <osg/Group>
#include <osg/ArgumentParser>

#include <string>


static std::string logstr( "lfx.demo" );

using namespace lfx::core;


class ImageHierarchyLoader : public Preprocess
{
public:
    ImageHierarchyLoader()
    {
    }
    ~ImageHierarchyLoader() {}

    virtual ChannelDataPtr operator()()
    {
        LFX_INFO_STATIC( logstr, "Preprocessing." );
        return( ChannelDataPtr( (ChannelData*)NULL ) );
    }
};


DataSetPtr prepareVolume( const std::string& fileName, const osg::Vec3& dims )
{
    DataSetPtr dsp( new DataSet() );

    ImageHierarchyLoader* ihl = new ImageHierarchyLoader();
    dsp->addPreprocess( PreprocessPtr( (Preprocess*)ihl ) );

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

    osg::ArgumentParser arguments( &argc, argv );

    std::string fileName;
    arguments.read( "-f", fileName );
    if( fileName.empty() )
    {
        LFX_FATAL_STATIC( logstr, "Must specify \"-f <filename>\" on command line." );
        return( 1 );
    }

    osg::Vec3 dims( 50., 50., 50. );
    arguments.read( "-d", dims[0],dims[1],dims[2] );

    // Create an example data set.
    osg::Group* root (new osg::Group);
    root->getOrCreateStateSet()->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );

    DataSetPtr dsp( prepareVolume( fileName, dims ) );
    root->addChild( dsp->getSceneData() );

    osgViewer::Viewer viewer;
    viewer.setUpViewInWindow( 20, 30, 800, 460 );
    viewer.setSceneData( root );

    return( viewer.run() );
}
