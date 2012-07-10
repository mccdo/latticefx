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
uniform int tfDest;
const int tfDestRGB = 0;
const int tfDestRGBA = 1;
const int tfDestAlpha = 2;

void transferFunction()
{
    vec3 localInput = tfInput;
    vec3 range = vec3( tfRange.y - tfRange.x );
    vec3 index = ( localInput - vec3(tfRange.x ) ) / range;
    vec4 result;
    if( tfDimension == 1 ) // 1D transfer function.
        result = texture1D( tf1d, index.s );
    else if( tfDimension == 2 ) // 2D transfer function.
        result = texture2D( tf2d, index.st );
    else if( tfDimension == 3 ) // 3D transfer function.
        result = texture3D( tf3d, index.stp );
    else // Transfer function is disabled.
    {
        gl_FrontColor = gl_Color;
        gl_BackColor = gl_Color;
        return;
    }

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

    /*
    vec3 norm = normalize( gl_NormalMatrix * normal );
    // Simple diffuse lighting computation with light at infinite viewer.
    float diff = max( 0., dot( norm.xyz, vec3( 0., 0., 1. ) ) );
    gl_FrontColor = vec4( vec3( diff ), 1. );
    */
}
