
#include <latticefx/VectorRenderer.h>
#include <latticefx/ChannelDataOSGArray.h>
#include <latticefx/TextureUtils.h>
#include <latticefx/BoundUtils.h>

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Texture1D>
#include <osg/Texture2D>
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

    ChannelDataPtr posChannel(
        getInput( getInputTypeAlias( POSITION ) )->getMaskedChannel( maskIn ) );
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

        unsigned int baseUnit( getTextureBaseUnit() );
        osg::Texture3D* posTex( lfx::createTexture3DForInstancedRenderer( posChannel ) );
        stateSet->setTextureAttributeAndModes( baseUnit++, posTex, osg::StateAttribute::OFF );

        const ChannelDataPtr radChannel(
            getInput( getInputTypeAlias( RADIUS ) )->getMaskedChannel( maskIn ) );
        osg::Texture3D* radTex( lfx::createTexture3DForInstancedRenderer( radChannel ) );
        stateSet->setTextureAttributeAndModes( baseUnit++, radTex, osg::StateAttribute::OFF );
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

        unsigned int baseUnit( getTextureBaseUnit() );
        osg::Texture3D* posTex( lfx::createTexture3DForInstancedRenderer( posChannel ) );
        stateSet->setTextureAttributeAndModes( baseUnit++, posTex, osg::StateAttribute::OFF );

        const ChannelDataPtr dirChannel( getInput( getInputTypeAlias( DIRECTION ) )->getMaskedChannel( maskIn ) );
        osg::Texture3D* dirTex( lfx::createTexture3DForInstancedRenderer( dirChannel ) );
        stateSet->setTextureAttributeAndModes( baseUnit++, dirTex, osg::StateAttribute::OFF );
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
        // stateSet->setMode( GL_VERTEX_PROGRAM_TWO_SIDED_LIGHTING, osg::StateAttribute::ON );

        int baseUnit( (int)( getTextureBaseUnit() ) );
        osg::Uniform* posUni( new osg::Uniform( osg::Uniform::SAMPLER_3D, "texPos" ) ); posUni->set( baseUnit++ );
        stateSet->addUniform( posUni );

        osg::Uniform* radUni( new osg::Uniform( osg::Uniform::SAMPLER_3D, "texRad" ) ); radUni->set( baseUnit++ );
        stateSet->addUniform( radUni );

        const ChannelDataPtr tfInputChannel( getInput( getTransferFunctionInput() ) );
        osg::Texture3D* tfInputTex( lfx::createTexture3DForInstancedRenderer( tfInputChannel ) );
        stateSet->setTextureAttributeAndModes( baseUnit, tfInputTex, osg::StateAttribute::OFF );

        osg::Uniform* tfInputUni( new osg::Uniform( osg::Uniform::SAMPLER_3D, "tfInput" ) ); tfInputUni->set( baseUnit++ );
        stateSet->addUniform( tfInputUni );

        osg::Texture1D* tf1dTex( new osg::Texture1D( getTransferFunction() ) );
        stateSet->setTextureAttributeAndModes( baseUnit, tf1dTex, osg::StateAttribute::OFF );

        osg::Uniform* tf1dUni( new osg::Uniform( osg::Uniform::SAMPLER_1D, "tf1d" ) ); tf1dUni->set( baseUnit++ );
        stateSet->addUniform( tf1dUni );

        osg::Uniform* tfDestUni( new osg::Uniform( "tfDest", (int)getTransferFunctionDestination() ) );
        stateSet->addUniform( tfDestUni );

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
        int baseUnit( (int)( getTextureBaseUnit() ) );
        osg::Uniform* posUni( new osg::Uniform( osg::Uniform::SAMPLER_3D, "texPos" ) ); posUni->set( baseUnit++ );
        stateSet->addUniform( posUni );

        osg::Uniform* dirUni( new osg::Uniform( osg::Uniform::SAMPLER_3D, "texDir" ) ); dirUni->set( baseUnit++ );
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
