
#include <latticefx/Renderer.h>
#include <osg/Image>
#include <osg/Texture1D>
#include <osg/Texture2D>
#include <osg/Texture3D>


namespace lfx {


Renderer::Renderer()
  : OperationBase( OperationBase::RendererType ),
    _baseUnit( 8 ),
    _unitAssignmentCounter( 8 ),
    _tfDest( TF_ALPHA ),
    _hmSource( HM_SOURCE_ALPHA ),
    _hmReference( 0.f ),
    _hmOperator( HM_OP_OFF )
{
}
Renderer::Renderer( const Renderer& rhs )
  : OperationBase( rhs ),
    _baseUnit( rhs._baseUnit ),
    _unitAssignmentCounter( rhs._unitAssignmentCounter ),
    _unitAssignmentMap( rhs._unitAssignmentMap ),
    _tfImage( rhs._tfImage ),
    _tfInputName( rhs._tfInputName ),
    _tfDest( rhs._tfDest ),
    _hmSource( rhs._hmSource ),
    _hmInputName( rhs._hmInputName ),
    _hmReference( rhs._hmReference ),
    _hmOperator( rhs._hmOperator )
{
}
Renderer::~Renderer()
{
}


void Renderer::setTextureBaseUnit( const unsigned int baseUnit )
{
    _baseUnit = baseUnit;
    resetTextureUnitAssignments();
}
unsigned int Renderer::getTextureBaseUnit() const
{
    return( _baseUnit );
}
unsigned int Renderer::getOrAssignTextureUnit( const std::string& key )
{
    UnitAssignmentMap::const_iterator it( _unitAssignmentMap.find( key ) );
    if( it == _unitAssignmentMap.end() )
    {
        _unitAssignmentMap[ key ] = _unitAssignmentCounter;
        return( _unitAssignmentCounter++ );
    }
    else
        return( it->second );
}
void Renderer::resetTextureUnitAssignments()
{
    _unitAssignmentCounter = _baseUnit;
    _unitAssignmentMap.clear();
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

void Renderer::setTransferFunctionDestination( const Renderer::TransferFunctionDestination dest )
{
    _tfDest = dest;
}
Renderer::TransferFunctionDestination Renderer::getTransferFunctionDestination() const
{
    return( _tfDest );
}


void Renderer::setHardwareMaskInputSource( const HardwareMaskInputSource source )
{
    _hmSource = source;
}
const Renderer::HardwareMaskInputSource Renderer::getHardwareMaskInputSource() const
{
    return( _hmSource );
}

void Renderer::setHardwareMaskInput( const std::string& inputName )
{
    _hmInputName = inputName;
}
const std::string& Renderer::getHardwareMaskInput() const
{
    return( _hmInputName );
}

void Renderer::setHardwareMaskReference( const float reference )
{
    _hmReference = reference;
}
float Renderer::getHardwareMaskReference() const
{
    return( _hmReference );
}

void Renderer::setHardwareMaskOperator( const unsigned int& maskOp )
{
    _hmOperator = maskOp;
}
unsigned int Renderer::getHardwareMaskOperator() const
{
    return( _hmOperator );
}



void Renderer::addHardwareFeatureUniforms( osg::StateSet* stateSet )
{
    int tfDimension;
    osg::Image* function( getTransferFunction() );
    if( function == NULL )
    {
        // No transfer function specified.
        tfDimension = 0;
    }
    else if( ( function->t() == 1 ) && ( function->r() == 1 ) )
    {
        // 1D transfer function.
        tfDimension = 1;

        osg::Texture1D* tf1dTex( new osg::Texture1D( function ) );
        const unsigned int tf1dUnit( getOrAssignTextureUnit( "tf1d" ) );
        stateSet->setTextureAttributeAndModes( tf1dUnit, tf1dTex, osg::StateAttribute::OFF );

        osg::Uniform* tf1dUni( new osg::Uniform( osg::Uniform::SAMPLER_1D, "tf1d" ) ); tf1dUni->set( (int)tf1dUnit );
        stateSet->addUniform( tf1dUni );
    }
    else if( function->r() == 1 )
    {
        // 2D transfer function.
        tfDimension = 2;

        osg::Texture2D* tf2dTex( new osg::Texture2D( function ) );
        const unsigned int tf2dUnit( getOrAssignTextureUnit( "tf2d" ) );
        stateSet->setTextureAttributeAndModes( tf2dUnit, tf2dTex, osg::StateAttribute::OFF );

        osg::Uniform* tf2dUni( new osg::Uniform( osg::Uniform::SAMPLER_2D, "tf2d" ) ); tf2dUni->set( (int)tf2dUnit );
        stateSet->addUniform( tf2dUni );
    }
    else
    {
        // 3D transfer function.
        tfDimension = 3;

        osg::Texture3D* tf3dTex( new osg::Texture3D( function ) );
        const unsigned int tf3dUnit( getOrAssignTextureUnit( "tf3d" ) );
        stateSet->setTextureAttributeAndModes( tf3dUnit, tf3dTex, osg::StateAttribute::OFF );

        osg::Uniform* tf3dUni( new osg::Uniform( osg::Uniform::SAMPLER_3D, "tf3d" ) ); tf3dUni->set( (int)tf3dUnit );
        stateSet->addUniform( tf3dUni );
    }

    osg::Uniform* tfDimUni( new osg::Uniform( "tfDimension", tfDimension ) );
    stateSet->addUniform( tfDimUni );

    osg::Uniform* tfDestUni( new osg::Uniform( "tfDest", (int)getTransferFunctionDestination() ) );
    stateSet->addUniform( tfDestUni );


    // Cram all mask parameters into a single vec4 uniform:
    //   Element 0: Input source (0=alpha, 1=red, 2=scalar
    //   Element 1: Mask operator (0=OFF, 1=EQ, 2=LT, 3=GT).
    //   Element 2: Operator negate flag (1=negate).
    //   Element 3: Reference value.
    osg::Vec4 maskParams( 0., 0., 0., _hmReference );
    if( _hmOperator != HM_OP_OFF )
    {
        if( ( _hmSource & HM_SOURCE_RED ) != 0 )
            maskParams[ 0 ] = 1.f;
        else if( ( _hmSource & HM_SOURCE_SCALAR ) != 0 )
            maskParams[ 0 ] = 2.f;

        if( ( _hmOperator & HM_OP_EQ ) != 0 )
            maskParams[ 1 ] = 1.f;
        else if( ( _hmOperator & HM_OP_LT ) != 0 )
            maskParams[ 1 ] = 2.f;
        else if( ( _hmOperator & HM_OP_GT ) != 0 )
            maskParams[ 1 ] = 3.f;

        if( ( _hmOperator & HM_OP_NOT ) != 0 )
            maskParams[ 2 ] = 1.f;
    }

    osg::Uniform* hmParamsUni( new osg::Uniform( "hmParams", maskParams ) );
    stateSet->addUniform( hmParamsUni );
}


// lfx
}
