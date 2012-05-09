
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
#include <string>



namespace lfx {


VectorRenderer::VectorRenderer()
  : lfx::Renderer(),
    _pointStyle( SIMPLE_POINTS )
{
    setInputNameAlias( POSITION, "positions" );
    setInputNameAlias( DIRECTION, "directions" );
    setInputNameAlias( RADIUS, "radii" );
}
VectorRenderer::VectorRenderer( const VectorRenderer& rhs )
  : lfx::Renderer( rhs ),
    _pointStyle( rhs._pointStyle ),
    _inputTypeMap( rhs._inputTypeMap )
{
}
VectorRenderer::~VectorRenderer()
{
}

osg::Node* VectorRenderer::getSceneGraph( const lfx::ChannelDataPtr maskIn )
{
    osg::ref_ptr< osg::Geode > geode( new osg::Geode );

    ChannelDataPtr posChannel( lfx::getMaskedChannel(
        getInput( getInputTypeAlias( POSITION ) ), maskIn ) );
    osg::Array* sourceArray( posChannel->asOSGArray() );
    const unsigned int numElements( sourceArray->getNumElements() );
    osg::Vec3Array* positions( dynamic_cast< osg::Vec3Array* >( sourceArray ) );

    switch( _pointStyle )
    {
    default:
    case SIMPLE_POINTS:
    {
        osg::Geometry* geom( new osg::Geometry() );
        geom->setUseDisplayList( false );
        geom->setUseVertexBufferObjects( true );

        geom->setVertexArray( positions );
        geom->addPrimitiveSet( new osg::DrawArrays( GL_POINTS, 0, positions->size() ) );
        geode->addDrawable( geom );
        break;
    }
    case POINT_SPRITES:
    {
        break;
    }
    case SPHERES:
    {
        // Geodesic sphere with subdivision=1 produces 20 sides. sub=2 produces 80 sides.
        osg::Geometry* geom( osgwTools::makeGeodesicSphere( 1., 1 ) );
        geom->setUseDisplayList( false );
        geom->setUseVertexBufferObjects( true );
        // TBD bound pad needs to be settable.
        geom->setInitialBound( lfx::getBound( *positions, osg::Vec3( 1., 1., 1. ) ) );
        geode->addDrawable( geom );

        // Set the number of instances.
        unsigned int idx;
        for( idx=0; idx < geom->getNumPrimitiveSets(); ++idx )
            geom->getPrimitiveSet( idx )->setNumInstances( numElements );

        osg::StateSet* stateSet( geode->getOrCreateStateSet() );

        const osg::Vec3f dimensions( lfx::computeTexture3DDimensions( numElements ) );
        osg::Uniform* texDim( new osg::Uniform( "texDim", dimensions ) );
        stateSet->addUniform( texDim );

        osg::Texture3D* posTex( lfx::createTexture3DForInstancedRenderer( posChannel ) );
        stateSet->setTextureAttributeAndModes( 0, posTex, osg::StateAttribute::OFF );

        const ChannelDataPtr radChannel( lfx::getMaskedChannel(
            getInput( getInputTypeAlias( RADIUS ) ), maskIn ) );
        osg::Texture3D* radTex( lfx::createTexture3DForInstancedRenderer( radChannel ) );
        stateSet->setTextureAttributeAndModes( 1, radTex, osg::StateAttribute::OFF );
        break;
    }
    case DIRECTION_VECTORS:
    {
        osg::Geometry* geom( osgwTools::makeArrow() );
        geom->setUseDisplayList( false );
        geom->setUseVertexBufferObjects( true );
        // TBD bound pad needs to be settable.
        geom->setInitialBound( lfx::getBound( *positions, osg::Vec3( 1., 1., 1. ) ) );
        geode->addDrawable( geom );

        // Set the number of instances.
        unsigned int idx;
        for( idx=0; idx < geom->getNumPrimitiveSets(); ++idx )
            geom->getPrimitiveSet( idx )->setNumInstances( numElements );

        osg::StateSet* stateSet( geode->getOrCreateStateSet() );

        const osg::Vec3f dimensions( lfx::computeTexture3DDimensions( numElements ) );
        osg::Uniform* texDim( new osg::Uniform( "texDim", dimensions ) );
        stateSet->addUniform( texDim );

        osg::Texture3D* posTex( lfx::createTexture3DForInstancedRenderer( posChannel ) );
        stateSet->setTextureAttributeAndModes( 0, posTex, osg::StateAttribute::OFF );

        const ChannelDataPtr dirChannel( lfx::getMaskedChannel(
            getInput( getInputTypeAlias( DIRECTION ) ), maskIn ) );
        osg::Texture3D* dirTex( lfx::createTexture3DForInstancedRenderer( dirChannel ) );
        stateSet->setTextureAttributeAndModes( 1, dirTex, osg::StateAttribute::OFF );
        break;
    }
    }

    return( geode.release() );
}

osg::StateSet* VectorRenderer::getRootState()
{
    osg::ref_ptr< osg::StateSet > stateSet( new osg::StateSet() );

    switch( _pointStyle )
    {
    default:
    case SIMPLE_POINTS:
    {
        stateSet->setMode( GL_LIGHTING, osg::StateAttribute::OFF );

        osg::Program* program = new osg::Program();
        stateSet->setAttribute( program );
        osg::Shader* vertexShader = new osg::Shader( osg::Shader::VERTEX );
        vertexShader->loadShaderSourceFromFile( osgDB::findDataFile( "lfx-simplepoints.vs" ) );
        program->addShader( vertexShader );
        osg::Shader* fragmentShader = new osg::Shader( osg::Shader::FRAGMENT );
        fragmentShader->loadShaderSourceFromFile( osgDB::findDataFile( "lfx-simplepoints.fs" ) );
        program->addShader( fragmentShader );
        break;
    }
    case POINT_SPRITES:
    {
        break;
    }
    case SPHERES:
    {
        osg::Uniform* posUni( new osg::Uniform( osg::Uniform::SAMPLER_3D, "texPos" ) ); posUni->set( 0 );
        stateSet->addUniform( posUni );

        osg::Uniform* radUni( new osg::Uniform( osg::Uniform::SAMPLER_3D, "texRad" ) ); radUni->set( 1 );
        stateSet->addUniform( radUni );

        osg::Program* program = new osg::Program();
        stateSet->setAttribute( program );
        osg::Shader* vertexShader = new osg::Shader( osg::Shader::VERTEX );
        vertexShader->loadShaderSourceFromFile( osgDB::findDataFile( "lfx-pointspheres.vs" ) );
        program->addShader( vertexShader );
        osg::Shader* fragmentShader = new osg::Shader( osg::Shader::FRAGMENT );
        fragmentShader->loadShaderSourceFromFile( osgDB::findDataFile( "lfx-pointspheres.fs" ) );
        program->addShader( fragmentShader );
        break;
    }
    case DIRECTION_VECTORS:
    {
        osg::Uniform* posUni( new osg::Uniform( osg::Uniform::SAMPLER_3D, "texPos" ) ); posUni->set( 0 );
        stateSet->addUniform( posUni );

        osg::Uniform* dirUni( new osg::Uniform( osg::Uniform::SAMPLER_3D, "texDir" ) ); dirUni->set( 1 );
        stateSet->addUniform( dirUni );

        osg::Program* program = new osg::Program();
        stateSet->setAttribute( program );
        osg::Shader* vertexShader = new osg::Shader( osg::Shader::VERTEX );
        vertexShader->loadShaderSourceFromFile( osgDB::findDataFile( "lfx-vectorfield.vs" ) );
        program->addShader( vertexShader );
        osg::Shader* fragmentShader = new osg::Shader( osg::Shader::FRAGMENT );
        fragmentShader->loadShaderSourceFromFile( osgDB::findDataFile( "lfx-vectorfield.fs" ) );
        program->addShader( fragmentShader );
        break;
    }
    }

    return( stateSet.release() );
}


void VectorRenderer::setPointStyle( const PointStyle& pointStyle )
{
    _pointStyle = pointStyle;
}
VectorRenderer::PointStyle VectorRenderer::getPointStyle() const
{
    return( _pointStyle );
}

void VectorRenderer::setInputNameAlias( const InputType& inputType, const std::string& alias )
{
    _inputTypeMap[ inputType ] = alias;
}
std::string VectorRenderer::getInputTypeAlias( const InputType& inputType ) const
{
    InputTypeMap::const_iterator it( _inputTypeMap.find( inputType ) );
    if( it != _inputTypeMap.end() )
        return( it->second );
    else
        return( "" );
}


// lfx
}
