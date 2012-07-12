#version 120


uniform sampler3D VolumeTexture;
uniform sampler2D TransferFunction;

varying vec3 Texcoord;
varying vec3 TexcoordUp;
varying vec3 TexcoordRight;
varying vec3 TexcoordBack;
varying vec3 TexcoordDown;
varying vec3 TexcoordLeft;
varying vec3 TexcoordFront;


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

    return( gl_FrontLightModelProduct.sceneColor + amb + diff + spec );
}

/** end light **/


bool TestInBounds(vec3 sample)
{
   return (sample.x > 0.0 && sample.x < 1.0 && sample.y > 0.0 && sample.y < 1.0 && sample.z > 0.0 && sample.z < 1.0);
}

void main( void )
{
    if( !( TestInBounds( Texcoord ) ) )
        discard;


    // Sample current fragment texture and six surrounding texture coordinates
    vec4 fvBaseColor = texture3D( VolumeTexture, Texcoord );

    // Early discard
    const float alphaThresh = 25./255.;
    if( fvBaseColor.r < alphaThresh )
        discard;


    vec4 fvUpColor = texture3D( VolumeTexture, TexcoordUp );
    vec4 fvRightColor = texture3D( VolumeTexture, TexcoordRight );
    vec4 fvBackColor = texture3D( VolumeTexture, TexcoordBack );
    vec4 fvDownColor = texture3D( VolumeTexture, TexcoordDown );
    vec4 fvLeftColor = texture3D( VolumeTexture, TexcoordLeft );
    vec4 fvFrontColor = texture3D( VolumeTexture, TexcoordFront );
    fvUpColor = fvUpColor - fvBaseColor;
    fvRightColor = fvRightColor - fvBaseColor;
    fvBackColor = fvBackColor - fvBaseColor;
    fvDownColor = fvDownColor - fvBaseColor;
    fvLeftColor = fvLeftColor - fvBaseColor;
    fvFrontColor = fvFrontColor - fvBaseColor;

    vec3 ocNormal = normalize( vec3( fvLeftColor.r - fvRightColor.r, fvDownColor.r - fvUpColor.r, fvFrontColor.r - fvBackColor.r ) );
    vec3 ecNormal = gl_NormalMatrix * ocNormal;

    //vec4 xfer = texture2D( TransferFunction, vec2(fvBaseColor.r, 0.0) );
    vec4 xfer = vec4( 1., 1., 1., 1. );
    fvBaseColor = fragmentLighting( xfer, ecNormal );
    fvBaseColor.a = 1.;


    gl_FragData[ 0 ] = fvBaseColor;

    // Support for second/glow render target.
    gl_FragData[ 1 ] = vec4( 0., 0., 0., 0. );
}
