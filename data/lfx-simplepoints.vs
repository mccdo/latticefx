#version 120


/** begin transfer function **/

uniform sampler1D tf1d;
attribute float tfInput;
uniform int tfDest;
const int tfDestRGB = 0;
const int tfDestRGBA = 1;
const int tfDestAlpha = 2;

void transferFunction()
{
    vec4 result = texture1D( tf1d, tfInput );
    if( tfDest == tfDestRGB )
    {
        gl_FrontColor.rgb = result.rgb;
        gl_FrontColor.a = gl_Color.a;
    }
    else if( tfDest == tfDestRGBA )
    {
        gl_FrontColor = result;
    }
    else
    {
        gl_FrontColor.rgb = gl_Color.rgb;
        gl_FrontColor.a = result.a;
    }
}

/** end transfer function **/


/** begin hardware mask **/

attribute float hmInput;
uniform vec4 hmParams;

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
    if( hmParams[ 0 ] == 0. )
        value = gl_FrontColor.a;
    else if( hmParams[ 0 ] == 1. )
        value = gl_FrontColor.r;
    else if( hmParams[ 0 ] == 2. )
        value = hmInput;

    bool result;
    if( hmParams[ 1 ] == 1. ) // Equal
        result = ( value == hmParams[ 3 ] );
    else if( hmParams[ 1 ] == 2. ) // Less than
        result = ( value < hmParams[ 3 ] );
    else if( hmParams[ 1 ] == 3. ) // Greater than
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
