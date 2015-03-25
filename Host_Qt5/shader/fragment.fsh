#version 100
// General fragment shader
uniform mediump vec4 colour;

void main(void)
{
    gl_FragColor.rgba = colour;
}
