#version 120

#extension GL_ARB_draw_instanced : require


uniform vec3 VolumeDims;
uniform vec3 VolumeCenter;
uniform float PlaneSpacing;

uniform mat4 osg_ViewMatrixInverse;

varying vec3 Texcoord;
varying vec3 TexcoordUp;
varying vec3 TexcoordRight;
varying vec3 TexcoordBack;
varying vec3 TexcoordDown;
varying vec3 TexcoordLeft;
varying vec3 TexcoordFront;

varying vec3 ecVertex;


vec4 rotatePointToVector( vec4 point, vec4 vector )
{
   // build a rotation matrix that will rotate the point that begins at 0,0,1
   mat4 yrot;
   mat4 xrot;
  
   // rotate around y first
   float hypot = vector.x * vector.x + vector.z * vector.z;
   hypot = sqrt(hypot);
   float cosTheta = vector.z / hypot;
   float sinTheta = vector.x / hypot;

   yrot[0] = vec4(cosTheta, 0.0, sinTheta, 0.0);
   yrot[1] = vec4(0.0, 1.0, 0.0, 0.0);
   yrot[2] = vec4(-sinTheta, 0.0, cosTheta, 0.0);
   yrot[3] = vec4(0.0, 0.0, 0.0, 1.0);

   vec4 temp = vector * yrot;
   
   // rotate around x
   hypot = temp.y * temp.y + temp.z * temp.z;
   cosTheta = temp.z / hypot;
   sinTheta = temp.y / hypot;
   
   xrot[0] = vec4(1.0, 0.0, 0.0, 0.0);
   xrot[1] = vec4(0.0, cosTheta, sinTheta, 0.0);
   xrot[2] = vec4(0.0, -sinTheta, cosTheta, 0.0);
   xrot[3] = vec4(0.0, 0.0, 0.0, 1.0);
   
   mat4 finalRot = yrot * xrot;
   
   vec4 rotPoint = point * finalRot;
   return rotPoint;
}

void findNearFarCubeVertexDist( in vec3 cubeCenter, in vec3 cubeDims, out float nearVertDist, out float farVertDist )
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
    // However, length() involves a sqrt. So, instead we use the dot product of
    // the eye coord vector to determine the max distance, then, once we've
    // finally found it, we take the sqrt of that dot product. This means just
    // one sqrt is performed.
    // Mathematically:
    //   sqrt( dot( v, v ) ) == length( v )
    farVertDist = -1000000000.;
    if( v0.z < 0. ) // Don't even consider it if it's not in front of the 'eye'.
        farVertDist = max( farVertDist, dot( v0, v0 ) );
    if( v1.z < 0. )
        farVertDist = max( farVertDist, dot( v1, v1 ) );
    if( v2.z < 0. )
        farVertDist = max( farVertDist, dot( v2, v2 ) );
    if( v3.z < 0. )
        farVertDist = max( farVertDist, dot( v3, v3 ) );
    if( v4.z < 0. )
        farVertDist = max( farVertDist, dot( v4, v4 ) );
    if( v5.z < 0. )
        farVertDist = max( farVertDist, dot( v5, v5 ) );
    if( v6.z < 0. )
        farVertDist = max( farVertDist, dot( v6, v6 ) );
    if( v7.z < 0. )
        farVertDist = max( farVertDist, dot( v7, v7 ) );

    if( farVertDist < 0. )
    {
        // The entire volume is behind the 'eye'.
        nearVertDist = farVertDist;
        return;
    }
    // Take our single sqrt here.
    farVertDist = sqrt( farVertDist );

    // The nearest vert distance will always be the eye coord z. However, eye coords are
    // right handed, so -z is in front of the 'eye'. Therefore, negate each z value.
    nearVertDist = min( 1000000000., -v0.z );
    nearVertDist = min( nearVertDist, -v1.z );
    nearVertDist = min( nearVertDist, -v2.z );
    nearVertDist = min( nearVertDist, -v3.z );
    nearVertDist = min( nearVertDist, -v4.z );
    nearVertDist = min( nearVertDist, -v5.z );
    nearVertDist = min( nearVertDist, -v6.z );
    nearVertDist = min( nearVertDist, -v7.z );
}

vec3 getCubeScales(mat4 modelMat)
{
   vec3 modelMatScales;
   modelMatScales.x = length(vec3(modelMat[0].x, modelMat[1].x, modelMat[2].x));
   modelMatScales.y = length(vec3(modelMat[0].y, modelMat[1].y, modelMat[2].y));
   modelMatScales.z = length(vec3(modelMat[0].z, modelMat[1].z, modelMat[2].z));
   return modelMatScales;
}

float getCubeDiagonalLength( in vec3 modelMatScales, out vec3 cubeDims  )
{
   cubeDims = VolumeDims * modelMatScales;
   return( length( cubeDims ) );
}

void main( void )
{
    vec3 ocDims = VolumeDims;
    vec3 ocCenter = VolumeCenter;
    vec3 wcCubeDims;
    float wcDiagLength = getCubeDiagonalLength( getCubeScales( gl_ModelViewMatrix ), wcCubeDims );

    float farVertDist, nearVertDist;
    findNearFarCubeVertexDist( ocCenter, wcCubeDims, nearVertDist, farVertDist );
   
    float curQuadDist = farVertDist - PlaneSpacing * gl_InstanceIDARB;
    if( ( farVertDist <= 0.0 ) ||
        ( curQuadDist <= nearVertDist ) )
    {
        // Clip the vertex.
        gl_Position = vec4( 1., 1., 1., 0. );
        return;
    }

    // All work to be done in (eye coords) view space
    // Find center of cube in (eye coords) model space where the origin is the camera location
    vec4 ecCenter = gl_ModelViewMatrix * vec4( ocCenter, 1.0 );
    // find the view vector from camera to object
    vec3 ecCenterDir = normalize( ecCenter.xyz );
    // Find location of our new quad
    vec4 ecQuadCenter = vec4( ecCenterDir * curQuadDist, 1.0);

    // Resize the quad's vertex by the distance across the cube diagonally
    // gl_Vertex.x and y values are +/- 0.5, so just multiply by the diagonal.
    vec4 quadVertex = vec4( gl_Vertex.xy * wcDiagLength, 0., gl_Vertex.w );

    // rotate the point so normal aligns with the view vector
    quadVertex = rotatePointToVector( quadVertex, vec4( -ecCenterDir, 1. ) );
   
    // move quad to target position in view space
    vec4 ecQuadVertex = vec4( ecQuadCenter.xyz + quadVertex.xyz, 1. );
         
    // Find the coordinates in obj coord space relative to the data cube
    vec4 vertexCopy = gl_ModelViewMatrixInverse * ecQuadVertex;
    Texcoord = vec3( .5 + (vertexCopy.x - ocCenter.x) / ocDims.x,
        .5 + (vertexCopy.y - ocCenter.y) / ocDims.y,
        .5 + (vertexCopy.z - ocCenter.z) / ocDims.z );
            
    // Surrounding texture coords used for surface normal derivation
    TexcoordUp  = Texcoord + vec3(0.0, .01, 0.0);
    TexcoordRight = Texcoord + vec3(.01, 0.0, 0.0);
    TexcoordBack    = Texcoord + vec3(0.0, 0.0, .01);
    TexcoordDown = Texcoord + vec3(0.0, -.01, 0.0);
    TexcoordLeft  = Texcoord + vec3(-.01, 0.0, 0.0);
    TexcoordFront  = Texcoord + vec3(0.0, 0.0, -.01);


    // set the position
    gl_Position = gl_ProjectionMatrix * ecQuadVertex;

    // Lighting
    ecVertex = ecQuadVertex.xyz;
    gl_FrontColor = gl_Color;
    gl_BackColor = gl_Color;

    // Clip plane support: Clip vertex is eye coord vertex.
    gl_ClipVertex = vec4( vec3( ecVertex ), 1. );
}
