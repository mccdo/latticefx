//uniform vec3 fvLightPosition;
//uniform vec3 fvEyePosition;
//uniform vec4 vViewPosition;
//uniform vec4 vViewDirection;

uniform vec3 VolumeDims;
uniform vec3 VolumeCenter;

uniform float PlaneSpacing;
//uniform float TexStart;
//uniform float TexSpacing;

varying vec3 Texcoord;
//varying vec3 ViewDirection;
//varying vec3 LightDirection;
//varying vec3 Normal;
//varying float TexSample;

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

float findNearestCubeVertexDist(out vec4 mvNearestVertex)
{
   float distMin = 100000.0;
   vec4 cubeVertex = vec4(0.0, 0.0, 0.0, 1.0);
   mvNearestVertex = vec4(0.0, 0.0, 0.0, distMin);
   
   for (int cubeX = 0; cubeX < 2; ++cubeX)
   {
      cubeVertex.x = VolumeCenter.x;
      cubeVertex.x += (cubeX == 1 ? VolumeDims.x * .5: -VolumeDims.x * .5);
      for (int cubeY = 0; cubeY < 2; ++cubeY)
      {
         cubeVertex.y = VolumeCenter.y;
         cubeVertex.y += (cubeY == 1 ? VolumeDims.y * .5: -VolumeDims.y * .5);
         for (int cubeZ = 0; cubeZ < 2; ++cubeZ)
         {
            cubeVertex.z = VolumeCenter.z;
            cubeVertex.z += (cubeZ == 1 ? VolumeDims.z * .5: -VolumeDims.z * .5);
            vec4 mvCubeVertex = gl_ModelViewMatrix * cubeVertex;
            // In view space -z is in front of the camera
            if (mvCubeVertex.z < 0.0)
            {
               float vertDist = length(mvCubeVertex);
               if (vertDist < distMin)
               {
                  distMin = vertDist;
                  mvNearestVertex = mvCubeVertex;
               }
            }
         }
      }
   }
   return distMin;
}

float findFarthestCubeVertexDist(out vec4 mvFarthestVertex)
{
   float distMax = -100000.0;
   vec4 cubeVertex = vec4(0.0, 0.0, 0.0, 1.0);
   mvFarthestVertex = vec4(0.0, 0.0, 0.0, distMax);

   
   for (int cubeX = 0; cubeX < 2; ++cubeX)
   {
      cubeVertex.x = VolumeCenter.x;
      cubeVertex.x += (cubeX == 1 ? VolumeDims.x * .5: -VolumeDims.x * .5);
      for (int cubeY = 0; cubeY < 2; ++cubeY)
      {
         cubeVertex.y = VolumeCenter.y;
         cubeVertex.y += (cubeY == 1 ? VolumeDims.y * .5: -VolumeDims.y * .5);
         for (int cubeZ = 0; cubeZ < 2; ++cubeZ)
         {
            cubeVertex.z = VolumeCenter.z;
            cubeVertex.z += (cubeZ == 1 ? VolumeDims.z * .5: -VolumeDims.z * .5);
            vec4 mvCubeVertex = gl_ModelViewMatrix * cubeVertex;
            // In view space -z is in front of the camera
            if (mvCubeVertex.z < 0.0)
            {
               float vertDist = length(mvCubeVertex);
               if (vertDist > distMax)
               {
                  distMax = vertDist;
                  mvFarthestVertex = mvCubeVertex;
               }
            }
         }
      }
   }
   return distMax;
}

float getCubeDiagonalLength(void)
{
   vec3 diagonalVec = vec3(VolumeDims.x, VolumeDims.y, VolumeDims.z);
   return length(diagonalVec);
}

void main( void )
{
   vec4 newVertexPos = gl_Vertex;
   vec4 mvNearestVertex;
   vec4 mvFarthestVertex;
   
   // below lines un-needed
   //TexSample = 1.0 - (TexStart + TexSpacing * gl_InstanceID);
   //Texcoord    = gl_MultiTexCoord0.xyz;

   float cubeDiagonal = getCubeDiagonalLength() * .5;
   
   float farVertDist = findFarthestCubeVertexDist(mvFarthestVertex);
   float nearVertMinDist = findNearestCubeVertexDist(mvNearestVertex);
   if (farVertDist > 0.0)
   {
      float curQuadDist = farVertDist - PlaneSpacing * gl_InstanceID;
      if (curQuadDist > nearVertMinDist && curQuadDist > 0.0)
      {
         // All work to be done in view space
         // Find center of cube in model space where the origin is the camera location
         vec4 mvVolumeCenter = gl_ModelViewMatrix * vec4(VolumeCenter, 1.0);
         // find the view vector from camera to object
         vec4 mvCubeDirection = vec4(normalize(mvVolumeCenter.xyz), 1.0);
         // Find location of our new quad
         vec4 quadPosition = vec4(mvCubeDirection.xyz * curQuadDist, 1.0);
         
         // Resize the quad's vertex by the distance across the cube diagonally
         if (gl_Vertex.x >= 0.0)
            newVertexPos.x = cubeDiagonal;
         else
            newVertexPos.x = -cubeDiagonal;
         if (gl_Vertex.y >= 0.0)
            newVertexPos.y = cubeDiagonal;
         else
            newVertexPos.y = -cubeDiagonal;
         newVertexPos.z = 0.0;
          
         // rotate the point so normal aligns with the view vector
         newVertexPos = rotatePointToVector(newVertexPos, normalize(-quadPosition) );
   
         // move quad to target position in view space
         newVertexPos.x += quadPosition.x;
         newVertexPos.y += quadPosition.y;
         newVertexPos.z += quadPosition.z;
         
         // Find the coordinates in model space relative to the data cube
         vec4 vertexCopy = gl_ModelViewMatrixInverse * newVertexPos;
         Texcoord    = vec3(.5 + (vertexCopy.x - VolumeCenter.x) / VolumeDims.x,
            .5 + (vertexCopy.y - VolumeCenter.y) / VolumeDims.y, .5 + (vertexCopy.z - VolumeCenter.z) / VolumeDims.z);
       }
      else
      {
         newVertexPos = vec4(0.0, 0.0, 10000.0, 1.0);
      }
   }
   else
   {
      newVertexPos = vec4(0.0, 0.0, 10000.0, 1.0);
   }

   // set the position
   gl_Position = gl_ProjectionMatrix * newVertexPos;

    
   //vec4 fvObjectPosition = gl_ModelViewMatrix * newVertexPos;
   
   //ViewDirection  = fvEyePosition - fvObjectPosition.xyz;
   //LightDirection = fvLightPosition - fvObjectPosition.xyz;
   //Normal         = gl_NormalMatrix * gl_Normal;
   
}
