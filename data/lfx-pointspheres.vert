#version 120

#extension GL_ARB_draw_instanced : require


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
uniform sampler3D tfInput;
uniform vec2 tfRange;
uniform int tfDimension;
uniform vec4 tfDest;

void transferFunction( in vec3 tC )
{
    vec4 xfer;
    vec4 inputSample = texture3D( tfInput, tC );
    if( tfDimension == 1 ) // 1D transfer function.
    {
        // tfInput texture format is GL_ALPHA32F_ARB.
        float index = inputSample.a;
        index = ( index - tfRange.x ) / ( tfRange.y - tfRange.x );
        xfer = texture1D( tf1d, index );
    }
    else if( tfDimension == 2 ) // 2D transfer function.
    {
        // tfInput texture format is GL_LUMINANCE_ALPHA32F_ARB.
        vec2 index = inputSample.ba;
        index = ( index - tfRange.x ) / ( tfRange.y - tfRange.x );
        xfer = texture2D( tf2d, index );
    }
    else if( tfDimension == 3 ) // 3D transfer function.
    {
        vec3 index = inputSample.rgb;
        index = ( index - tfRange.x ) / ( tfRange.y - tfRange.x );
        xfer = texture3D( tf3d, index );
    }

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

uniform sampler3D hmInput;
uniform vec4 hmParams;
uniform float hmEpsilon;
const float hmAlpha = 0.;
const float hmRed = 1.;
const float hmScalar = 2.;

// Return true if passed, false if failed.
bool hardwareMask( in vec3 tC )
{
    // hmParams has all mask parameters in a single vec4 uniform:
    //   Element 0: Input source (0=alpha, 1=red, 2=scalar, 1000=no mask
    //   Element 1: Mask operator (0=EQ, -1=LT, 1=GT).
    //   Element 2: Operator negate flag (1=no negate, -1=negate).
    //   Element 3: Reference value.

    float value;
    if( hmParams[ 0 ] < hmScalar )
    {
        // either red (1) or alpha (0).
        value = dot( gl_FrontColor.ra, vec2( hmParams[ 0 ], 1. - hmParams[ 0 ] ) );
    }
    else if( hmParams[ 0 ] == hmScalar )
    {
        // hmInput texture format is GL_ALPHA32F_ARB.
        value = texture3D( hmInput, tC ).a;
    }
    else
    {
        // no mask
        return( true );
    }

    // sign() returns 1=pos, 0=0, and -1=neg -- same as what we have in hmParams[ 1 ].
    // So if sign(value-ref) == hmParams[1], we have passed the hm test.
    float signEps = sign( value - hmParams[ 3 ] - hmEpsilon );
    float signPlusEps = sign( value - hmParams[ 3 ] + hmEpsilon );
    if( signEps != signPlusEps )
        signEps = 0.;
    bool result = ( signEps == hmParams[ 1 ] );

    if( hmParams[ 2 ] == -1. ) // Negate
        result = !result;
    return( result );
}

/** end hardware mask **/



uniform sampler3D texPos;
uniform sampler3D texRad;
uniform vec3 texDim;


vec3 generateTexCoord( const in int iid )
{
    int s = int( texDim.x );
    int t = int( texDim.y );
    int p = int( texDim.z );
    int p1 = iid / (s*t);     // p1 = p coord in range (0 to (p-1) )
    int tiid = iid - ( p1 * (s*t) );
    int t1 = tiid / s;           // likewise, t1 = t coord...
    int s1 = tiid - ( t1 * s );  // ...and s1 = s coord.

    // Normalize (s1,t1,p1) to range (0 to (1-epsilon)).
    // Man. TextureRectangle would make this much easier.
    float sEps = 1. / (texDim.x * 2. );
    float tEps = 1. / (texDim.y * 2. );
    float pEps = 1. / (texDim.z * 2. );
    return( vec3( s1 / texDim.x + sEps, t1 / texDim.y + tEps, p1 / texDim.z + pEps ) );
}


void main()
{
    // Generate stp texture coords from the instance ID.
    vec3 tC = generateTexCoord( gl_InstanceIDARB );

    transferFunction( tC );
    if( !hardwareMask( tC ) )
    {
        // "Discard" in vectex shader: set clip coord x, y, and z all > w.
        // (Setting them < -w would also work).
        gl_Position = vec4( 1., 1., 1., 0. );
        return;
    }

    // Sample (look up) xyz position
    vec4 pos = texture3D( texPos, tC );

    // Sample (look up) radius. texRad texture format is GL_ALPHA32F_ARB.
    float scale = texture3D( texRad, tC ).a;

    // Scale and translate the vertex, then transform to clip coords.
    vec4 oVec = vec4( scale * gl_Vertex.xyz + pos.xyz, 1.0 );
    gl_Position = gl_ModelViewProjectionMatrix * oVec;
    gl_ClipVertex = gl_ModelViewMatrix * oVec;

    vertexLighting( oVec, gl_Normal );
}
