#version 120


/** begin transfer function **/

uniform sampler1D tf1d;
uniform sampler2D tf2d;
uniform sampler3D tf3d;
attribute vec3 tfInput;
uniform vec2 tfRange;
uniform int tfDimension;
uniform vec4 tfDest;

void transferFunction()
{
    vec3 localInput = tfInput;
    vec3 index = ( localInput - tfRange.x ) / ( tfRange.y - tfRange.x );
    vec4 xfer;
    if( tfDimension == 1 ) // 1D transfer function.
        xfer = texture1D( tf1d, index.s );
    else if( tfDimension == 2 ) // 2D transfer function.
        xfer = texture2D( tf2d, index.st );
    else if( tfDimension == 3 ) // 3D transfer function.
        xfer = texture3D( tf3d, index.stp );

    // If tfDimension is non-zero, we get the normal destination mask.
    // If zero, set dest mask to all zeros to get all gl_Color.
    vec4 localDestMask = tfDest * max( float( tfDimension ), 1. );

    // localDestMask is rgba floats, and will be either 1.0 or 0.0 for each element.
    // For element=1.0, take element from the xfer function.
    // Otherwise, take element from glColor.
    gl_FrontColor = ( xfer * localDestMask )
        + ( gl_Color * ( 1. - localDestMask ) );
    gl_BackColor = ( xfer * localDestMask )
        + ( gl_Color * ( 1. - localDestMask ) );
}

/** end transfer function **/


/** begin hardware mask **/

attribute float hmInput;
uniform vec4 hmParams;
const float hmAlpha = 0.;
const float hmRed = 1.;
const float hmScalar = 2.;
const float hmEqual = 1.;
const float hmLessThan = 2.;
const float hmGreaterThan = 3.;

// Return true if passed, false if failed.
bool hardwareMask()
{
    // hmParams has all mask parameters in a single vec4 uniform:
    //   Element 0: Input source (0=alpha, 1=red, 2=scalar
    //   Element 1: Mask operator (0=OFF, 1=EQ, 2=LT, 3=GT).
    //   Element 2: Operator negate flag (1=negate).
    //   Element 3: Reference value.

    if( hmParams[ 1 ] == 0. ) // Off
        return( true );

    float value;
    if( hmParams[ 0 ] == hmAlpha )
        value = gl_FrontColor.a;
    else if( hmParams[ 0 ] == hmRed )
        value = gl_FrontColor.r;
    else if( hmParams[ 0 ] == hmScalar )
        value = hmInput;

    bool result;
    if( hmParams[ 1 ] == hmEqual )
        result = ( value == hmParams[ 3 ] );
    else if( hmParams[ 1 ] == hmLessThan )
        result = ( value < hmParams[ 3 ] );
    else if( hmParams[ 1 ] == hmGreaterThan )
        result = ( value > hmParams[ 3 ] );

    if( hmParams[ 2 ] == 1. ) // Negate
        result = !result;
    return( result );
}

/** end hardware mask **/



void main()
{
    transferFunction();
    if( !hardwareMask() )
    {
        // "Discard" in vectex shader: set clip coord x, y, and z all > w.
        // (Setting them < -w would also work).
        gl_Position = vec4( 1., 1., 1., 0. );
        return;
    }

    gl_Position = ftransform();
    gl_ClipVertex = gl_ModelViewMatrix * gl_Vertex;
}
