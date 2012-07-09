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
#include <latticefx/core/ChannelData.h>
#include <latticefx/core/ChannelDataOSGArray.h>
#include <latticefx/core/SurfaceRenderer.h>

#include <osgViewer/Viewer>


void createTriangles( osg::Vec3Array* verts, osg::Vec3Array* norms )
{
    const int count( 5 );
    const int startX( count / 2 );

    for( int idx=0; idx<count; idx++ )
    {
        verts->push_back( osg::Vec3( startX+idx, 0., 0. ) );
        verts->push_back( osg::Vec3( startX+idx+1, 0., 0. ) );
        verts->push_back( osg::Vec3( startX+idx, 0., 1. ) );
        norms->push_back( osg::Vec3( 0., -1., 0. ) );
        norms->push_back( osg::Vec3( 0., -1., 0. ) );
        norms->push_back( osg::Vec3( 0., -1., 0. ) );
    }
}

lfx::DataSetPtr prepareDataSet()
{
    osg::ref_ptr< osg::Vec3Array > verts( new osg::Vec3Array );
    osg::ref_ptr< osg::Vec3Array > norms( new osg::Vec3Array );
    createTriangles( verts.get(), norms.get() );

    lfx::ChannelDataOSGArrayPtr cdv( new lfx::ChannelDataOSGArray( verts.get(), "vertices" ) );
    lfx::ChannelDataOSGArrayPtr cdn( new lfx::ChannelDataOSGArray( norms.get(), "normals" ) );

    // Create a data set and add the vertex data.
    lfx::DataSetPtr dsp( new lfx::DataSet() );
    dsp->addChannel( cdv );
    dsp->addChannel( cdn );

    lfx::SurfaceRendererPtr renderOp( new lfx::SurfaceRenderer() );
    renderOp->setInputNameAlias( lfx::SurfaceRenderer::POSITION, cdv->getName() );
    renderOp->setInputNameAlias( lfx::SurfaceRenderer::NORMAL, cdn->getName() );

    renderOp->addInput( cdv->getName() );
    renderOp->addInput( cdn->getName() );
    dsp->setRenderer( renderOp );

    return( dsp );
}



int main( int argc, char** argv )
{
    lfx::DataSetPtr dsp( prepareDataSet() );

    osgViewer::Viewer viewer;
    viewer.setUpViewInWindow( 20, 30, 800, 460 );
    viewer.setSceneData( dsp->getSceneData() );
    return( viewer.run() );
}
