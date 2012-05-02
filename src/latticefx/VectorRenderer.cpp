
#include <latticefx/VectorRenderer.h>
#include <latticefx/ChannelDataOSGArray.h>
#include <latticefx/TextureUtils.h>
#include <latticefx/MaskUtils.h>
#include <latticefx/BoundUtils.h>

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Texture3D>
#include <osg/Shader>
#include <osg/Program>
#include <osg/Uniform>
#include <osgDB/FileUtils>

#include <osgwTools/Shapes.h>



namespace lfx {


VectorRenderer::VectorRenderer()
  : lfx::Renderer()
{
}
VectorRenderer::VectorRenderer( const VectorRenderer& rhs )
{
}
VectorRenderer::~VectorRenderer()
{}

osg::Node* VectorRenderer::getSceneGraph( const lfx::ChannelDataPtr maskIn )
{
    const osg::Array* sourceArray( getInput( "positions" )->asOSGArray() );
    const osg::Vec3Array* positions( dynamic_cast< const osg::Vec3Array* >( sourceArray ) );

    osg::ref_ptr< osg::Geode > geode( new osg::Geode );

    osg::Geometry* geom( osgwTools::makeArrow() );
    geom->setUseDisplayList( false );
    geom->setUseVertexBufferObjects( true );
    geom->setInitialBound( lfx::getBound( *positions, osg::Vec3( 1., 1., 1. ) ) );
    geode->addDrawable( geom );

    // Set the number of instances.
    const unsigned int numElements( sourceArray->getNumElements() );
    unsigned int idx;
    for( idx=0; idx < geom->getNumPrimitiveSets(); ++idx )
        geom->getPrimitiveSet( idx )->setNumInstances( numElements );

    osg::StateSet* stateSet( geode->getOrCreateStateSet() );

    osg::Texture3D* posTex( lfx::createTexture3DForInstancedRenderer( getInput( "positions" ) ) );
    stateSet->setTextureAttributeAndModes( 0, posTex, osg::StateAttribute::OFF );
    osg::Uniform* posUni( new osg::Uniform( osg::Uniform::SAMPLER_3D, "texPos" ) ); posUni->set( 0 );
    stateSet->addUniform( posUni );

    osg::Texture3D* dirTex( lfx::createTexture3DForInstancedRenderer( getInput( "directions" ) ) );
    stateSet->setTextureAttributeAndModes( 1, dirTex, osg::StateAttribute::OFF );
    osg::Uniform* dirUni( new osg::Uniform( osg::Uniform::SAMPLER_3D, "texDir" ) ); dirUni->set( 1 );
    stateSet->addUniform( dirUni );

    const osg::Vec3f dimensions( lfx::computeTexture3DDimensions( numElements ) );
    osg::Uniform* texDim( new osg::Uniform( "texDim", dimensions ) );
    stateSet->addUniform( texDim );

    osg::Program* program = new osg::Program();
    stateSet->setAttribute( program );
    osg::Shader* vertexShader = new osg::Shader( osg::Shader::VERTEX );
    vertexShader->loadShaderSourceFromFile( osgDB::findDataFile( "lfx-vectorfield.vs" ) );
    program->addShader( vertexShader );
    osg::Shader* fragmentShader = new osg::Shader( osg::Shader::FRAGMENT );
    fragmentShader->loadShaderSourceFromFile( osgDB::findDataFile( "lfx-vectorfield.fs" ) );
    program->addShader( fragmentShader );

    return( geode.release() );
}


// lfx
}
