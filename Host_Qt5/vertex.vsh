#version 100
// General vertex shader
attribute vec2 vertex;
uniform highp mat4 projection, modelView;

void main(void)
{
	vec4 position = vec4(vertex, 0, 1);
	gl_Position.xyzw = projection * modelView * position;
}
