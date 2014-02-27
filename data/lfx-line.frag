#version 120


void main()
{
    gl_FragData[ 0 ] = gl_Color;

    // Support for second/glow render target.
    gl_FragData[ 1 ] = vec4( 0., 0., 0., 0. );
}
