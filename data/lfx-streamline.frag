#version 120


/** begin light **/

varying vec3 ecVertex;
varying vec3 ecNormal;

vec4 fragmentLighting( vec4 baseColor )
{
    vec3 normal = normalize( ecNormal );
    if( !gl_FrontFacing )
        normal = -normal;

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


void main()
{
    gl_FragData[ 0 ] = fragmentLighting( gl_Color );

    // Support for second/glow render target.
    gl_FragData[ 1 ] = vec4( 0., 0., 0., 0. );
}
