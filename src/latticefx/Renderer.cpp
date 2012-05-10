
#include <latticefx/Renderer.h>


namespace lfx {


Renderer::Renderer()
  : OperationBase( OperationBase::RendererType ),
    _baseUnit( 8 )
{
}
Renderer::Renderer( const Renderer& rhs )
  : OperationBase( rhs ),
    _baseUnit( rhs._baseUnit )
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


// lfx
}
