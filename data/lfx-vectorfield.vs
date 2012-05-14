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
uniform sampler3D tfInput;
uniform int tfDest;
const int tfDestRGB = 0;
const int tfDestRGBA = 1;
const int tfDestAlpha = 2;

void transferFunction( in vec3 tC )
{
    // Get index. tfInput texture format is GL_ALPHA32F_ARB.
    float index = texture3D( tfInput, tC ).a;
    vec4 result = texture1D( tf1d, index );
    if( tfDest == tfDestRGB )
    {
        gl_FrontColor.rgb = result.rgb;
        gl_BackColor.rgb = result.rgb;
        gl_FrontColor.a = gl_Color.a;
        gl_BackColor.a = gl_Color.a;
    }
    else if( tfDest == tfDestRGBA )
    {
        gl_FrontColor = result;
        gl_BackColor = result;
    }
    else
    {
        gl_FrontColor.rgb = gl_Color.rgb;
        gl_BackColor.rgb = gl_Color.rgb;
        gl_FrontColor.a = result.a;
        gl_BackColor.a = result.a;
    }
}

/** end transfer function **/


/** begin hardware mask **/

uniform sampler3D hmInput;
uniform vec4 hmParams;

// Return true if passed, false if failed.
bool hardwareMask( in vec3 tC )
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
    {
        // hmInput texture format is GL_ALPHA32F_ARB.
        value = texture3D( hmInput, tC ).a;
    }

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



uniform sampler3D texPos;
uniform sampler3D texDir;
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

mat3 makeOrientMat( const in vec3 dir )
{
    // Compute a vector at a right angle to the direction.
    // First try projection direction into xy rotated -90 degrees.
    // If that gives us a very short vector,
    // then project into yz instead, rotated -90 degrees.
    vec3 c = vec3( dir.y, -dir.x, 0.0 );
    if( dot( c, c ) < 0.1 )
        c = vec3( 0.0, dir.z, -dir.y );
        
    // Appears to be a bug in normalize when z==0
    //normalize( c.xyz );
    float l = length( c );
    c /= l;

    vec3 up = normalize( cross( dir, c ) );

    // Orientation uses the cross product vector as x,
    // the up vector as y, and the direction vector as z.
    return( mat3( c, up, dir ) );
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

    // Sample (look up) direction vector and obtain the scale factor
    vec4 dir = texture3D( texDir, tC );
    float scale = length( dir.xyz );

    // Create an orientation matrix. Orient/transform the arrow.
    mat3 orientMat = makeOrientMat( normalize( dir.xyz ) );
    vec3 oVec = orientMat * (scale * gl_Vertex.xyz);
    vec4 hoVec = vec4( oVec + pos.xyz, 1.0 );
    gl_Position = gl_ModelViewProjectionMatrix * hoVec;
    gl_ClipVertex = gl_ModelViewMatrix * hoVec;

    vertexLighting( hoVec, orientMat * gl_Normal );
}
