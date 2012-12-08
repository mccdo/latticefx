#version 120


uniform vec3 volumeResolution;
uniform vec3 volumeDims;
uniform vec3 volumeCenter;

varying vec3 tcEye;
varying float volumeSize;
varying vec4 clipCoord;
varying vec4 clipEye;

void main( void )
{
    vec3 ocCenter = volumeCenter;
    vec3 ocDims = volumeDims;

    vec4 ecDims = gl_ModelViewMatrix * vec4( ocDims, 1. );
    volumeSize = length( ecDims.xyz );

    // Need to interpolate clip coordinates along the ray.
    clipCoord = gl_ModelViewProjectionMatrix * gl_Vertex;
    // Eye position in clip coordinates (actually could be a uniform):
    clipEye = gl_ProjectionMatrix * vec4( 0., 0., 0., 1. );

    // Compute eye position in texture coordinate space.
    vec4 ocEye = gl_ModelViewMatrixInverse * vec4( 0., 0., 0., 1. );
    tcEye = ( ocEye.xyz - ocCenter ) / ocDims + vec3( .5 );

    // Pass through texture coordinate.
    gl_TexCoord[0] = gl_MultiTexCoord0;

    gl_Position = ftransform();
}
