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


vec4 rotatePointToVector(vec4 point, vec4 vector)
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

void findNearFarCubeVertexDist(vec3 cubeCenter, vec3 cubeDims, out vec4 mvNearestVertex, out vec4 mvFarthestVertex, out float nearVertDist, out float farVertDist)
{
   nearVertDist = 100000.0;
   farVertDist = -100000.0;
   vec4 cubeVertex = vec4(0.0, 0.0, 0.0, 1.0);
   mvNearestVertex = vec4(0.0, 0.0, 0.0, nearVertDist);
   mvFarthestVertex = vec4(0.0, 0.0, 0.0, farVertDist);
   
   for (int cubeX = 0; cubeX < 2; ++cubeX)
   {
      cubeVertex.x = cubeCenter.x;
      cubeVertex.x += (cubeX == 1 ? cubeDims.x * .5: -cubeDims.x * .5);
      for (int cubeY = 0; cubeY < 2; ++cubeY)
      {
         cubeVertex.y = cubeCenter.y;
         cubeVertex.y += (cubeY == 1 ? cubeDims.y * .5: -cubeDims.y * .5);
         for (int cubeZ = 0; cubeZ < 2; ++cubeZ)
         {
            cubeVertex.z = cubeCenter.z;
            cubeVertex.z += (cubeZ == 1 ? cubeDims.z * .5: -cubeDims.z * .5);
            vec4 mvCubeVertex = gl_ModelViewMatrix * cubeVertex;

            // In view space -z is in front of the camera
             float vertDistZ = -mvCubeVertex.z;
             float vertDistLen = length(mvCubeVertex);
             if (mvCubeVertex.z > 0.0) vertDistLen = -vertDistLen;
             float minVertDist = vertDistLen < vertDistZ ? vertDistLen: vertDistZ;
             float maxVertDist = vertDistLen > vertDistZ ? vertDistLen: vertDistZ;
             if (minVertDist < nearVertDist)
             {
                nearVertDist = minVertDist;
                mvNearestVertex = mvCubeVertex;
             }
             if (maxVertDist > farVertDist)
             {
                farVertDist = maxVertDist;
                mvFarthestVertex = mvCubeVertex;
             }
         }
      }
   }
}

vec3 getCubeScales(mat4 modelMat)
{
   vec3 modelMatScales;
   modelMatScales.x = length(vec3(modelMat[0].x, modelMat[1].x, modelMat[2].x));
   modelMatScales.y = length(vec3(modelMat[0].y, modelMat[1].y, modelMat[2].y));
   modelMatScales.z = length(vec3(modelMat[0].z, modelMat[1].z, modelMat[2].z));
   return modelMatScales;
}

float getCubeDiagonalLength(vec3 modelMatScales, out vec3 cubeDims)
{
   cubeDims = VolumeDims * modelMatScales;
   return length(cubeDims);
}

vec3 getCubeCenterPos(vec3 modelMatTrans)
{
   return VolumeCenter + modelMatTrans;
}

void main( void )
{
   vec4 newVertexPos = gl_Vertex;
   
   mat4 modelMat = osg_ViewMatrixInverse * gl_ModelViewMatrix;
   vec3 modelTranslation = vec3(modelMat[0].w, modelMat[1].w, modelMat[2].w);
   vec3 cubeDims;
   
   float cubeDiagonal = getCubeDiagonalLength(getCubeScales(modelMat), cubeDims);
   vec3 cubeCenter = getCubeCenterPos(modelTranslation);
   
   vec4 mvNearestVertex, mvFarthestVertex;
   float farVertDist, nearVertDist;
   findNearFarCubeVertexDist(cubeCenter, cubeDims, mvNearestVertex, mvFarthestVertex, nearVertDist, farVertDist);
   
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
    vec4 mvVolumeCenter = gl_ModelViewMatrix * vec4(cubeCenter, 1.0);
    // find the view vector from camera to object
    vec4 mvCubeDirection = vec4(normalize(mvVolumeCenter.xyz), 1.0);
    // Find location of our new quad
    vec4 quadPosition = vec4(mvCubeDirection.xyz * curQuadDist, 1.0);
         
    // Resize the quad's vertex by the distance across the cube diagonally
    // gl_Vertex.x & y values are +/- 0.5, so just multiply by the diagonal.
    newVertexPos.x = gl_Vertex.x * cubeDiagonal;
    newVertexPos.y = gl_Vertex.y * cubeDiagonal;
    newVertexPos.z = 0.0;
          
    // rotate the point so normal aligns with the view vector
    newVertexPos = rotatePointToVector(newVertexPos, normalize(-quadPosition) );
   
    // move quad to target position in view space
    newVertexPos.x += quadPosition.x;
    newVertexPos.y += quadPosition.y;
    newVertexPos.z += quadPosition.z;
         
    // Find the coordinates in model space relative to the data cube
    // newVertexPos is in eye coords. vertexCopy is in object coords.
    vec4 vertexCopy = gl_ModelViewMatrixInverse * newVertexPos;
    Texcoord = vec3(.5 + (vertexCopy.x - cubeCenter.x) / VolumeDims.x,
        .5 + (vertexCopy.y - cubeCenter.y) / VolumeDims.y,
        .5 + (vertexCopy.z - cubeCenter.z) / VolumeDims.z);
            
    // Surrounding texture coords used for surface normal derivation
    TexcoordUp  = Texcoord + vec3(0.0, .01, 0.0);
    TexcoordRight = Texcoord + vec3(.01, 0.0, 0.0);
    TexcoordBack    = Texcoord + vec3(0.0, 0.0, .01);
    TexcoordDown = Texcoord + vec3(0.0, -.01, 0.0);
    TexcoordLeft  = Texcoord + vec3(-.01, 0.0, 0.0);
    TexcoordFront  = Texcoord + vec3(0.0, 0.0, -.01);


    // set the position
    gl_Position = gl_ProjectionMatrix * newVertexPos;

    // Lighting
    ecVertex = newVertexPos.xyz;
    gl_FrontColor = gl_Color;
    gl_BackColor = gl_Color;

    // Clip plane support: Clip vertex is eye coord vertex.
    gl_ClipVertex = vec4( vec3( ecVertex ), 1. );
}
