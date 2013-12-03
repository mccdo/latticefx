#version 120


uniform sampler2D stlImage;

void main()
{
    vec4 color = texture2D( stlImage, gl_TexCoord[ 0 ].st );
    color.rgb = gl_Color.rgb;
    gl_FragData[ 0 ] = color;

    // Support for second/glow render target.
    gl_FragData[ 1 ] = vec4( 0., 0., 0., 0. );
}
