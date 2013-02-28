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
#include <latticefx/core/DataSet.h>
#include <latticefx/core/ChannelData.h>
#include <latticefx/core/ChannelDataOSGArray.h>
#include <latticefx/core/RTPOperation.h>
#include <latticefx/core/Renderer.h>
#include <latticefx/core/TextureUtils.h>
#include <latticefx/core/BoundUtils.h>
#include <latticefx/core/Log.h>
#include <latticefx/core/LogMacros.h>

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Texture3D>
#include <osg/Shader>
#include <osg/Program>
#include <osg/Uniform>
#include <osgViewer/Viewer>
#include <osgDB/FileUtils>

#include <osgwTools/Shapes.h>


using namespace lfx::core;


class InstancedVectors : public Renderer
{
public:
    InstancedVectors() : Renderer()
    {}
    virtual ~InstancedVectors()
    {}

    virtual osg::Node* getSceneGraph( const ChannelDataPtr maskIn )
    {
        const osg::Array* sourceArray( _inputs[ 0 ]->asOSGArray() );
        const osg::Vec3Array* positions( dynamic_cast< const osg::Vec3Array* >( sourceArray ) );

        osg::ref_ptr< osg::Geode > geode( new osg::Geode );

        osg::Geometry* geom( osgwTools::makeArrow() );
        geom->setUseDisplayList( false );
        geom->setUseVertexBufferObjects( true );
        geom->setInitialBound( getBound( *positions, osg::Vec3( 1., 1., 1. ) ) );
        geode->addDrawable( geom );

        // Set the number of instances.
        const unsigned int numElements( sourceArray->getNumElements() );
        for( unsigned int idx=0; idx < geom->getNumPrimitiveSets(); ++idx )
            geom->getPrimitiveSet( idx )->setNumInstances( numElements );

        osg::StateSet* stateSet( geode->getOrCreateStateSet() );

        osg::Texture3D* posTex( createTexture3DForInstancedRenderer( getInput( "positions" ) ) );
        stateSet->setTextureAttributeAndModes( 0, posTex, osg::StateAttribute::OFF );
        osg::Uniform* posUni( new osg::Uniform( osg::Uniform::SAMPLER_3D, "texPos" ) ); posUni->set( 0 );
        stateSet->addUniform( posUni );

        osg::Texture3D* dirTex( createTexture3DForInstancedRenderer( getInput( "directions" ) ) );
        stateSet->setTextureAttributeAndModes( 1, dirTex, osg::StateAttribute::OFF );
        osg::Uniform* dirUni( new osg::Uniform( osg::Uniform::SAMPLER_3D, "texDir" ) ); dirUni->set( 1 );
        stateSet->addUniform( dirUni );

        const osg::Vec3f dimensions( computeTexture3DDimensions( numElements ) );
        osg::Uniform* texDim( new osg::Uniform( "texDim", dimensions ) );
        stateSet->addUniform( texDim );

        osg::Program* program = new osg::Program();
        stateSet->setAttribute( program );
        osg::Shader* vertexShader = new osg::Shader( osg::Shader::VERTEX );
        vertexShader->loadShaderSourceFromFile( osgDB::findDataFile( "lfx-vectorfield.vert" ) );
        program->addShader( vertexShader );
        osg::Shader* fragmentShader = new osg::Shader( osg::Shader::FRAGMENT );
        fragmentShader->loadShaderSourceFromFile( osgDB::findDataFile( "lfx-vectorfield.frag" ) );
        program->addShader( fragmentShader );

        return( geode.release() );
    }

protected:
};

DataSetPtr prepareDataSet()
{
    osg::ref_ptr< osg::Vec3Array > vertArray( new osg::Vec3Array );
    osg::ref_ptr< osg::Vec3Array > dirArray( new osg::Vec3Array );
    const unsigned int w( 4 ), h( 1 ), d( 1 );
    vertArray->resize( w*h*d );
    dirArray->resize( w*h*d );
    unsigned int wIdx, hIdx, dIdx, index( 0 );
    for( wIdx=0; wIdx<w; ++wIdx )
    {
        for( hIdx=0; hIdx<h; ++hIdx )
        {
            for( dIdx=0; dIdx<d; ++dIdx )
            {
                const float x( ((double)wIdx)/(w-1.) * 4. - 2. );
                const float y( 0. );
                const float z( 0. );
                (*vertArray)[ index ].set( x, y, z );
                (*dirArray)[ index ].set( sin(x), .5, .5 );
                ++index;
            }
        }
    }
    ChannelDataOSGArrayPtr vertData( new ChannelDataOSGArray( "positions", vertArray.get() ) );
    ChannelDataOSGArrayPtr dirData( new ChannelDataOSGArray( "directions", dirArray.get() ) );

    // Create a data set and add the vertex and direction data.
    DataSetPtr dsp( new DataSet() );
    dsp->addChannel( vertData );
    dsp->addChannel( dirData );

    RendererPtr renderOp( new InstancedVectors() );
    renderOp->addInput( vertData->getName() );
    renderOp->addInput( dirData->getName() );
    dsp->setRenderer( renderOp );

    return( dsp );
}



int main( int argc, char** argv )
{
    Log::instance()->setPriority( Log::PrioInfo, Log::Console );

    // Create an example data set.
    DataSetPtr dsp( prepareDataSet() );

    osgViewer::Viewer viewer;
    viewer.setUpViewInWindow( 10, 30, 800, 440 );
    // Obtain the data set's scene graph and add it to the viewer.
    viewer.setSceneData( dsp->getSceneData() );
    return( viewer.run() );
}
