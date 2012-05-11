
#include <latticefx/Renderer.h>
#include <osg/Image>
#include <osg/Texture1D>
#include <osg/Texture2D>
#include <osg/Texture3D>


namespace lfx {


Renderer::Renderer()
  : OperationBase( OperationBase::RendererType ),
    _baseUnit( 8 ),
    _tfDest( TF_ALPHA )
{
}
Renderer::Renderer( const Renderer& rhs )
  : OperationBase( rhs ),
    _baseUnit( rhs._baseUnit ),
    _tfImage( rhs._tfImage ),
    _tfInputName( rhs._tfInputName ),
    _tfDest( rhs._tfDest )
{
}
Renderer::~Renderer()
{
}


void Renderer::setTextureBaseUnit( const unsigned int baseUnit )
{
    _baseUnit = baseUnit;
}
unsigned int Renderer::getTextureBaseUnit() const
{
    return( _baseUnit );
}


void Renderer::setTransferFunction( osg::Image* image )
{
    _tfImage = image;
}
osg::Image* Renderer::getTransferFunction()
{
    return( _tfImage.get() );
}
const osg::Image* Renderer::getTransferFunction() const
{
    return( _tfImage.get() );
}

void Renderer::setTransferFunctionInput( const std::string& inputName )
{
    _tfInputName = inputName;
}
const std::string& Renderer::getTransferFunctionInput() const
{
    return( _tfInputName );
}

void Renderer::setTransferFunctionDestination( Renderer::TransferFunctionDestination dest )
{
    _tfDest = dest;
}
Renderer::TransferFunctionDestination Renderer::getTransferFunctionDestination() const
{
    return( _tfDest );
}



void Renderer::addTransferFunctionUniforms( osg::StateSet* stateSet, int& baseUnit )
{
    osg::Image* function( getTransferFunction() );
    if( ( function->t() == 1 ) && ( function->r() == 1 ) )
    {
        // 1D transfer function.
        osg::Texture1D* tf1dTex( new osg::Texture1D( function ) );
        stateSet->setTextureAttributeAndModes( baseUnit, tf1dTex, osg::StateAttribute::OFF );

        osg::Uniform* tf1dUni( new osg::Uniform( osg::Uniform::SAMPLER_1D, "tf1d" ) ); tf1dUni->set( baseUnit++ );
        stateSet->addUniform( tf1dUni );
    }
    else if( function->r() == 1 )
    {
        // 2D transfer function.
        osg::Texture2D* tf2dTex( new osg::Texture2D( function ) );
        stateSet->setTextureAttributeAndModes( baseUnit, tf2dTex, osg::StateAttribute::OFF );

        osg::Uniform* tf2dUni( new osg::Uniform( osg::Uniform::SAMPLER_2D, "tf2d" ) ); tf2dUni->set( baseUnit++ );
        stateSet->addUniform( tf2dUni );
    }
    else
    {
        // 3D transfer function.
        osg::Texture3D* tf3dTex( new osg::Texture3D( function ) );
        stateSet->setTextureAttributeAndModes( baseUnit, tf3dTex, osg::StateAttribute::OFF );

        osg::Uniform* tf3dUni( new osg::Uniform( osg::Uniform::SAMPLER_3D, "tf3d" ) ); tf3dUni->set( baseUnit++ );
        stateSet->addUniform( tf3dUni );
    }

    osg::Uniform* tfDestUni( new osg::Uniform( "tfDest", (int)getTransferFunctionDestination() ) );
    stateSet->addUniform( tfDestUni );
}


// lfx
}
