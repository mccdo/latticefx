#version 120


/** begin light **/

varying vec3 ecVertex;
varying vec3 ecNormal;

void vertexLighting( in vec4 ocVertex, in vec3 ocNormal )
{
    ecVertex = vec3( gl_ModelViewMatrix * ocVertex );
    ecNormal = gl_NormalMatrix * ocNormal;
}

/** end light **/


/** begin transfer function **/

uniform sampler1D tf1d;
uniform sampler2D tf2d;
uniform sampler3D tf3d;
attribute vec3 tfInput;
uniform vec2 tfRange;
uniform int tfDimension;
uniform vec4 tfDest;
const int tfDestRGB = 0;
const int tfDestRGBA = 1;
const int tfDestAlpha = 2;

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


uniform bool warpEnabled;
uniform float warpScale;

attribute vec3 warpVertex;
attribute vec3 warpNormal;

void main()
{
    vec4 position = gl_Vertex;
    vec3 normal = gl_Normal;
    if( warpEnabled )
    {
        vec3 vecOff = warpScale * warpVertex;
        position = vec4( (gl_Vertex.xyz + vecOff), gl_Vertex.w );

        vec3 normOff = warpScale * warpNormal;
        normal += normOff;
    }

    gl_Position = gl_ModelViewProjectionMatrix * position;

    transferFunction();
    vertexLighting( position, normal );

    // Clip plane support. Must follow vertexLighting() because that's
    // where ecVertex is computed.
    gl_ClipVertex = vec4( vec3( ecVertex ), 1. );
}
