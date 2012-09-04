#version 120

#extension GL_ARB_draw_instanced : require


uniform vec3 volumeDims;
uniform vec3 VolumeCenter;
uniform float PlaneSpacing;
uniform float volumeNumPlanes;

uniform mat4 osg_ViewMatrixInverse;

varying vec3 Texcoord;
varying vec3 TexcoordUp;
varying vec3 TexcoordRight;
varying vec3 TexcoordBack;
varying vec3 TexcoordDown;
varying vec3 TexcoordLeft;
varying vec3 TexcoordFront;

varying vec3 ecVertex;
varying vec3 ecUp;
varying vec3 ecRight;
varying vec3 ecBack;
varying vec3 ecDown;
varying vec3 ecLeft;
varying vec3 ecFront;


vec4 rotatePointToVector( vec4 point, vec4 vector )
{
    // build a rotation matrix that will rotate the point that begins at 0,0,1

    // rotate around y first
    float hypot = vector.x * vector.x + vector.z * vector.z;
    hypot = sqrt(hypot);
    float cosTheta = vector.z / hypot;
    float sinTheta = vector.x / hypot;

    mat4 yrot = mat4(
        cosTheta, 0.0, -sinTheta, 0.0,
        0.0, 1.0, 0.0, 0.0,
        sinTheta, 0.0, cosTheta, 0.0,
        0.0, 0.0, 0.0, 1.0 );

    vec4 temp = yrot * vector;

    // rotate around x
    hypot = temp.y * temp.y + temp.z * temp.z;
    cosTheta = temp.z / hypot;
    sinTheta = temp.y / hypot;

    mat4 xrot = mat4( 
        1.0, 0.0, 0.0, 0.0,
        0.0, cosTheta, -sinTheta, 0.0,
        0.0, sinTheta, cosTheta, 0.0,
        0.0, 0.0, 0.0, 1.0 );

    vec4 rotPoint = xrot * yrot * point;
    return( rotPoint );
}

void findNearFarCubeVertexDist( in vec3 ecCenterDir, in vec3 cubeCenter, in vec3 cubeDims, out float nearVertDist, out float farVertDist )
{
    // This could all be done with nested loops and in fact was originally coded that way.
    // However, unrolling the loops makes it much more efficient.

    // Compute the 8 cube verts in eye coords.
    vec3 c = cubeCenter;
    vec3 hd = cubeDims * .5;
    vec4 v0 = gl_ModelViewMatrix * vec4( c.x - hd.x, c.y - hd.y, c.z - hd.z, 1. );
    vec4 v1 = gl_ModelViewMatrix * vec4( c.x + hd.x, c.y - hd.y, c.z - hd.z, 1. );
    vec4 v2 = gl_ModelViewMatrix * vec4( c.x - hd.x, c.y + hd.y, c.z - hd.z, 1. );
    vec4 v3 = gl_ModelViewMatrix * vec4( c.x + hd.x, c.y + hd.y, c.z - hd.z, 1. );
    vec4 v4 = gl_ModelViewMatrix * vec4( c.x - hd.x, c.y - hd.y, c.z + hd.z, 1. );
    vec4 v5 = gl_ModelViewMatrix * vec4( c.x + hd.x, c.y - hd.y, c.z + hd.z, 1. );
    vec4 v6 = gl_ModelViewMatrix * vec4( c.x - hd.x, c.y + hd.y, c.z + hd.z, 1. );
    vec4 v7 = gl_ModelViewMatrix * vec4( c.x + hd.x, c.y + hd.y, c.z + hd.z, 1. );

    // The farthest vert distance will be the length of the eye coord vector.
    // However, length() involves a sqrt. So, for efficiency, use the squared
    // length (dot product).
    float farDist = -1000000000.;
    vec4 farVert;
    if( v0.z < 0. ) // Don't even consider it if it's not in front of the 'eye'.
    {
        float d = dot( v0, v0 );
        if( d > farDist )
        {
            farDist = d;
            farVert = v0;
        }
    }
    if( v1.z < 0. )
    {
        float d = dot( v1, v1 );
        if( d > farDist )
        {
            farDist = d;
            farVert = v1;
        }
    }
    if( v2.z < 0. )
    {
        float d = dot( v2, v2 );
        if( d > farDist )
        {
            farDist = d;
            farVert = v2;
        }
    }
    if( v3.z < 0. )
    {
        float d = dot( v3, v3 );
        if( d > farDist )
        {
            farDist = d;
            farVert = v3;
        }
    }
    if( v4.z < 0. )
    {
        float d = dot( v4, v4 );
        if( d > farDist )
        {
            farDist = d;
            farVert = v4;
        }
    }
    if( v5.z < 0. )
    {
        float d = dot( v5, v5 );
        if( d > farDist )
        {
            farDist = d;
            farVert = v5;
        }
    }
    if( v6.z < 0. )
    {
        float d = dot( v6, v6 );
        if( d > farDist )
        {
            farDist = d;
            farVert = v6;
        }
    }
    if( v7.z < 0. )
    {
        float d = dot( v7, v7 );
        if( d > farDist )
        {
            farDist = d;
            farVert = v7;
        }
    }

    if( farDist < 0. )
    {
        // The entire volume is behind the 'eye'.
        farVertDist = farDist;
        nearVertDist = 0.;
        return;
    }

    // The nearest vert distance will always be the eye coord z. However, eye coords are
    // right handed, so -z is in front of the 'eye'. Therefore, negate each z value.
    float nearDist = 1000000000.;
    vec4 nearVert;
    if( -v0.z < nearDist )
    {
        nearDist = -v0.z;
        nearVert = v0;
    }
    if( -v1.z < nearDist )
    {
        nearDist = -v1.z;
        nearVert = v1;
    }
    if( -v2.z < nearDist )
    {
        nearDist = -v2.z;
        nearVert = v2;
    }
    if( -v3.z < nearDist )
    {
        nearDist = -v3.z;
        nearVert = v3;
    }
    if( -v4.z < nearDist )
    {
        nearDist = -v4.z;
        nearVert = v4;
    }
    if( -v5.z < nearDist )
    {
        nearDist = -v5.z;
        nearVert = v5;
    }
    if( -v6.z < nearDist )
    {
        nearDist = -v6.z;
        nearVert = v6;
    }
    if( -v7.z < nearDist )
    {
        nearDist = -v7.z;
        nearVert = v7;
    }

    // Compute the distances to the planes defined by the given normal
    // ecCenterDir, and containing the vertices farVert and nearVert.
    //   Distance d = dot( n, p )
    // n must be normalized.
    farVertDist = dot( ecCenterDir, farVert.xyz );

    // Should never have a near distance less than 0; we would never
    // render a quad at that distance.
    nearVertDist = max( dot( ecCenterDir, nearVert.xyz ), 0. );
}

vec3 getCubeScales(mat4 modelMat)
{
    vec3 modelMatScales;
    modelMatScales.x = length(vec3(modelMat[0].x, modelMat[1].x, modelMat[2].x));
    modelMatScales.y = length(vec3(modelMat[0].y, modelMat[1].y, modelMat[2].y));
    modelMatScales.z = length(vec3(modelMat[0].z, modelMat[1].z, modelMat[2].z));
    return modelMatScales;
}

float getCubeDiagonalLength( in vec3 modelMatScales )
{
    vec3 cubeDims = volumeDims * modelMatScales;
    return( length( cubeDims ) );
}

void main( void )
{
    // Shortcut names with coordinate system prefix.
    vec3 ocDims = volumeDims;
    vec3 ocCenter = VolumeCenter;

    // Compute a normalized direction vector to the volume center in eye coordinates.
    vec4 ecCenter = gl_ModelViewMatrix * vec4( ocCenter, 1.0 );
    vec3 ecCenterDir = normalize( ecCenter.xyz );

    // Compute nearest and furthest distances to the min and max cube extents.
    float farVertDist, nearVertDist;
    findNearFarCubeVertexDist( ecCenterDir, ocCenter, ocDims, nearVertDist, farVertDist );

    // Determine distance to current quad slice, based on plane spacing and the instance ID.
    float planeSpacing;
    if( volumeNumPlanes > 0. )
        planeSpacing = ( farVertDist - nearVertDist ) / volumeNumPlanes;
    else
        planeSpacing = PlaneSpacing;
    float curQuadDist = farVertDist - planeSpacing * gl_InstanceIDARB;

    // Shortcut return: Clip entire quad slice if no more rendering is needed.
    if( curQuadDist <= nearVertDist )
    {
        // Clip the vertex.
        gl_Position = vec4( 1., 1., 1., 0. );
        return;
    }

    // Compute the center position of the current quad.
    vec4 ecQuadCenter = vec4( ecCenterDir * curQuadDist, 1.0);

    // Resize the quad's vertex xy coords by the distance across the cube diagonally.
    // Incoming gl_Vertex.x and y values are +/- 0.5, so just multiply by the diagonal length.
    float ecDiagLength = getCubeDiagonalLength( getCubeScales( gl_ModelViewMatrix ) );
    vec4 quadVertex = vec4( gl_Vertex.xy * ecDiagLength, 0., gl_Vertex.w );

    // rotate the quad so its normal aligns with the volume center vector.
    quadVertex = rotatePointToVector( quadVertex, vec4( -ecCenterDir, 1. ) );

    // move quad to target position in eye coordinates.
    vec4 ecQuadVertex = vec4( ecQuadCenter.xyz + quadVertex.xyz, 1. );

    // Compute the texture coordinate for the quad vertex.
    // The extent of the volume (accounting for ocCenter and ocDims) is -0.5 to 0.5
    // in all axes. By adding 0.5 to each coordinate, we get tex coords in the range
    // 0.0 to 1.0. Our tex coords will be outside that range (we render with overlap
    // to account for a worst-case view alignment).
    vec3 ocQuadVertex = ( gl_ModelViewMatrixInverse * ecQuadVertex ).xyz;
    Texcoord = ( ocQuadVertex - ocCenter ) / ocDims + vec3( .5 );

    // Surrounding texture coords used for surface normal derivation.
    const float delta = 0.01;
    TexcoordUp = Texcoord + vec3( 0., delta, 0. );
    TexcoordRight = Texcoord + vec3( delta, 0. , 0. );
    TexcoordBack = Texcoord + vec3( 0., 0., delta );
    TexcoordDown = Texcoord + vec3( 0., -delta, 0. );
    TexcoordLeft = Texcoord + vec3( -delta, 0., 0. );
    TexcoordFront = Texcoord + vec3( 0., 0., -delta );

    // Compute corresponding eye coords for clip plane tests.
    ecUp = ( gl_ModelViewMatrix * vec4( ( TexcoordUp - vec3( .5 ) ) * ocDims - ocCenter, 1. ) ).xyz;
    ecRight = ( gl_ModelViewMatrix * vec4( ( TexcoordRight - vec3( .5 ) ) * ocDims - ocCenter, 1. ) ).xyz;
    ecBack = ( gl_ModelViewMatrix * vec4( ( TexcoordBack - vec3( .5 ) ) * ocDims - ocCenter, 1. ) ).xyz;
    ecDown = ( gl_ModelViewMatrix * vec4( ( TexcoordDown - vec3( .5 ) ) * ocDims - ocCenter, 1. ) ).xyz;
    ecLeft = ( gl_ModelViewMatrix * vec4( ( TexcoordLeft - vec3( .5 ) ) * ocDims - ocCenter, 1. ) ).xyz;
    ecFront = ( gl_ModelViewMatrix * vec4( ( TexcoordFront - vec3( .5 ) ) * ocDims - ocCenter, 1. ) ).xyz;


    // Wrap-uo code.
    // Compute clip coordinates.
    gl_Position = gl_ProjectionMatrix * ecQuadVertex;

    // Additional values used in fragment lighting code.
    ecVertex = ecQuadVertex.xyz;
    gl_FrontColor = gl_Color;
    gl_BackColor = gl_Color;

    // Clip plane support: Clip vertex is eye coord vertex.
    gl_ClipVertex = vec4( ecVertex, 1. );
}
