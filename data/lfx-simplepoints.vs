#version 120


/** begin transfer function **/

uniform sampler1D tf1d;
attribute float tfInput;
uniform int tfDest;
const int tfDestRGB = 0;
const int tfDestRGBA = 1;
const int tfDestAlpha = 2;

void transferFunction()
{
    vec4 result = texture1D( tf1d, tfInput );
    if( tfDest == tfDestRGB )
    {
        gl_FrontColor.rgb = result.rgb;
        gl_FrontColor.a = gl_Color.a;
    }
    else if( tfDest == tfDestRGBA )
    {
        gl_FrontColor = result;
    }
    else
    {
        gl_FrontColor.rgb = gl_Color.rgb;
        gl_FrontColor.a = result.a;
    }
}

/** end transfer function **/



void main()
{
    gl_Position = ftransform();
    gl_ClipVertex = gl_ModelViewMatrix * gl_Vertex;

    transferFunction();
}
