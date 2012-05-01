
#include <latticefx/Renderer.h>


namespace lfx {


Renderer::Renderer()
  : OperationBase( OperationBase::RendererType )
{
}
Renderer::Renderer( const Renderer& rhs )
  : OperationBase( rhs )
{
}
Renderer::~Renderer()
{
}


// lfx
}
