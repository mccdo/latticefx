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

    // Sample (look up) xyz position
    vec4 pos = texture3D( texPos, tC );

    // Sample (look up) radius
    float scale = texture3D( texRad, tC ).a;

    // Scale and translate the vertex, then transform to clip coords.
    vec4 oVec = vec4( scale * gl_Vertex.xyz + pos.xyz, 1.0 );
    gl_Position = gl_ModelViewProjectionMatrix * oVec;

    vertexLighting( oVec, gl_Normal );

    gl_FrontColor = vec4( tC, 1. );
}
