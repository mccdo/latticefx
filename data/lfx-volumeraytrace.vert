#version 120


uniform vec3 volumeResolution;
uniform vec3 volumeDims;
uniform vec3 volumeCenter;

varying vec3 tcEye;

void main( void )
{
    vec3 ocCenter = volumeCenter;
    vec3 ocDims = volumeDims;

    // Compute eye position in texture coordinate space.
    vec4 ocEye = gl_ModelViewMatrixInverse * vec4( 0., 0., 0., 1. );
    tcEye = ( ocEye.xyz - ocCenter ) / ocDims + vec3( .5 );

    // Pass through texture coordinate.
    gl_TexCoord[0] = gl_MultiTexCoord0;

    gl_Position = ftransform();
}
