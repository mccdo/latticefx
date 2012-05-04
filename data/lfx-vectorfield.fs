#version 120


/** begin light **/

varying vec3 ecVertex;
varying vec3 ecNormal;

vec4 fragmentLighting( vec4 baseColor )
{
    vec3 lightVec = normalize( gl_LightSource[0].position.xyz - ecVertex );
    vec3 eyeVec = normalize( -ecVertex );
    vec3 reflectVec = normalize( -reflect( lightVec, ecNormal ) );

    vec4 amb = gl_LightSource[0].ambient * baseColor;

    vec4 diff = gl_LightSource[0].diffuse * baseColor * max( dot( ecNormal, lightVec ), 0. );
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
    // TBD baseColor comes from transfer function.
    vec4 baseColor = vec4( 1., 1., 1., 1. );

    // TBD Note: Must support RTT (use gl_FragData[]).
    gl_FragColor = fragmentLighting( baseColor );
}
