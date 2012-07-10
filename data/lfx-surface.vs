#version 120


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

    vec3 norm = normalize( gl_NormalMatrix * normal );
    // Simple diffuse lighting computation with light at infinite viewer.
    float diff = max( 0., dot( norm.xyz, vec3( 0., 0., 1. ) ) );
    gl_FrontColor = vec4( vec3( diff ), 1. );
}
