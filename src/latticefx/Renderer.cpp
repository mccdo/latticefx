
#include <latticefx/Renderer.h>
#include <osg/Image>


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


// lfx
}
