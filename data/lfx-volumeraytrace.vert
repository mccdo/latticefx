#version 120


uniform vec3 volumeResolution;
uniform vec3 volumeDims;
uniform vec3 volumeCenter;

uniform mat3 osg_NormalMatrixInverse;

// This variable comes from osgWorks, osgwTools library,
//   MultiCameraProjectionMatrix class, which allows the
//   OSG near & far computation to span two Cameras.
uniform mat4 osgw_ProjectionMatrix;

varying vec3 tcEye;
varying float ecVolumeSize;


vec4 eyeToTexCoords( in vec4 ec, in vec4 ocCenter, in vec4 ocDims )
{
    vec4 oc = gl_ModelViewMatrixInverse * ec;
    return( ( oc - ocCenter ) / ocDims + vec4( .5 ) );
}


void main( void )
{
    vec4 ocCenter = vec4( volumeCenter, 1.f );
    vec4 ocDims = vec4( volumeDims, 1.f );

    vec4 ecDims = gl_ModelViewMatrix * ocDims;
    ecVolumeSize = length( ecDims.xyz );

    // Compute eye position in texture coordinate space.
    const vec4 ecEye = vec4( 0., 0., 0., 1. );
    tcEye = eyeToTexCoords( ecEye, ocCenter, ocDims ).xyz;

    // Pass through texture coordinate.
    gl_TexCoord[0] = gl_MultiTexCoord0;

    // Do NOT use ftransform() -- Must use the osgWorks
    // MultiCameraProjectionMatrix for correct z testing.
    gl_Position = osgw_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex;
}
