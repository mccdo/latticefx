#version 120


/** begin light **/

varying vec3 ecVertex;

vec4 fragmentLighting( vec4 baseColor, vec3 normal )
{
    vec3 lightVec = normalize( gl_LightSource[0].position.xyz - ecVertex );
    vec3 eyeVec = normalize( -ecVertex );
    vec3 reflectVec = normalize( -reflect( lightVec, normal ) );

    vec4 amb = gl_LightSource[0].ambient * baseColor;

    vec4 diff = gl_LightSource[0].diffuse * baseColor * max( dot( normal, lightVec ), 0. );
    diff = clamp( diff, 0., 1. );
   
    // Hm. front material shininess is negative for some reason. Hack in "10.0" for now.
    float specExp = 10.; // gl_FrontMaterial.shininess
    vec4 spec = gl_FrontLightProduct[0].specular *
        pow( max( dot( reflectVec, eyeVec ), 0. ), specExp );
    spec = clamp( spec, 0., 1. );

    vec4 outColor = gl_FrontLightModelProduct.sceneColor + amb + diff + spec;
    outColor.a = baseColor.a;
    return( outColor );
}

/** end light **/


/** begin transfer function **/

uniform sampler1D tf1d;
uniform vec2 tfRange;
uniform int tfDimension;
uniform vec4 tfDest;

vec4 transferFunction( in float index )
{
    float range = tfRange.y - tfRange.x;
    float rangeIndex = ( index - tfRange.x ) / range;

    // Support only 1D transfer function for now.
    vec4 xfer = texture1D( tf1d, rangeIndex );

    // If tfDimension is non-zero, we get the normal destination mask.
    // If zero, set dest mask to all zeros to get all gl_Color.
    vec4 localDestMask = tfDest * min( float( tfDimension ), 1. );

    // localDestMask is rgba floats, and will be either 1.0 or 0.0 for each element.
    // For element=1.0, take element from the xfer function.
    // Otherwise, take element from glColor.
    vec4 returnColor = ( xfer * localDestMask )
        + ( gl_Color * ( 1. - localDestMask ) );

    return( returnColor );
}

/** end transfer function **/


/** begin hardware mask **/

uniform sampler3D hmInput;
uniform vec4 hmParams;
const float hmAlpha = 0.;
const float hmRed = 1.;
const float hmScalar = 2.;

// Return true if passed, false if failed.
bool hardwareMask( in vec3 tC, in vec4 baseColor )
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
        value = dot( baseColor.ra, vec2( hmParams[ 0 ], 1. - hmParams[ 0 ] ) );
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
    bool result = ( sign( value - hmParams[ 3 ] ) == hmParams[ 1 ] );

    if( hmParams[ 2 ] == -1. ) // Negate
        result = !result;
    return( result );
}

/** end hardware mask **/



uniform sampler3D VolumeTexture;

varying vec3 Texcoord;
varying vec3 TexcoordUp;
varying vec3 TexcoordRight;
varying vec3 TexcoordBack;
varying vec3 TexcoordDown;
varying vec3 TexcoordLeft;
varying vec3 TexcoordFront;

varying vec3 ecUp;
varying vec3 ecRight;
varying vec3 ecBack;
varying vec3 ecDown;
varying vec3 ecLeft;
varying vec3 ecFront;


bool TestInBounds(vec3 sample)
{
   return (sample.x > 0.0 && sample.x < 1.0 && sample.y > 0.0 && sample.y < 1.0 && sample.z > 0.0 && sample.z < 1.0);
}

void main( void )
{
    // Vectex shader always sends (eye oriented) quads. Much of the quad
    // might be outside the volume. Immediately discard if this is the case.
    if( !( TestInBounds( Texcoord ) ) )
        discard;


    // Get volume sample. Format is GL_KUMINANCE, so the same volume value
    // will be stored in fvBaseColor.r, g, and b. fvBaseColor.a will be 1.0.
    vec4 fvBaseColor = texture3D( VolumeTexture, Texcoord );


    vec4 color = transferFunction( fvBaseColor.r );
    if( !hardwareMask( Texcoord, color ) )
        discard;


    // Clip plane test.
    if( dot( vec4( ecVertex, 1. ), gl_ClipPlane[ 0 ] ) < 0. )
        discard;


    vec4 fvUpColor = vec4( 0., 0., 0., 0. );
    if( dot( vec4( ecUp, 1. ), gl_ClipPlane[ 0 ] ) > 0. )
        fvUpColor = transferFunction( texture3D( VolumeTexture, TexcoordUp ).r );
    vec4 fvRightColor = vec4( 0., 0., 0., 0. );
    if( dot( vec4( ecRight, 1. ), gl_ClipPlane[ 0 ] ) > 0. )
        fvRightColor = transferFunction( texture3D( VolumeTexture, TexcoordRight ).r );
    vec4 fvBackColor = vec4( 0., 0., 0., 0. );
    if( dot( vec4( ecBack, 1. ), gl_ClipPlane[ 0 ] ) > 0. )
        fvBackColor = transferFunction( texture3D( VolumeTexture, TexcoordBack ).r );
    vec4 fvDownColor = vec4( 0., 0., 0., 0. );
    if( dot( vec4( ecDown, 1. ), gl_ClipPlane[ 0 ] ) > 0. )
        fvDownColor = transferFunction( texture3D( VolumeTexture, TexcoordDown ).r );
    vec4 fvLeftColor = vec4( 0., 0., 0., 0. );
    if( dot( vec4( ecLeft, 1. ), gl_ClipPlane[ 0 ] ) > 0. )
        fvLeftColor = transferFunction( texture3D( VolumeTexture, TexcoordLeft ).r );
    vec4 fvFrontColor = vec4( 0., 0., 0., 0. );
    if( dot( vec4( ecFront, 1. ), gl_ClipPlane[ 0 ] ) > 0. )
        fvFrontColor = transferFunction( texture3D( VolumeTexture, TexcoordFront ).r );
    vec4 xVec = fvLeftColor - fvRightColor;
    vec4 yVec = fvDownColor - fvUpColor;
    vec4 zVec = fvFrontColor - fvBackColor;

    vec3 ocNormal = vec3( xVec.a, yVec.a, zVec.a );
    vec3 ecNormal = normalize( gl_NormalMatrix * ocNormal );

    vec4 finalColor = fragmentLighting( color, ecNormal );


    gl_FragData[ 0 ] = finalColor;

    // Support for second/glow render target.
    gl_FragData[ 1 ] = vec4( 0., 0., 0., 0. );
}
