attribute vec2 vertex;
uniform int pointSize;
uniform highp mat4 projection, modelView;

void main(void)
{
	vec4 position;
	position.xy = vertex;
	position.z = 0;
	position.w = 1;
	gl_Position.xyzw = projection * modelView * position;
	gl_PointSize = pointSize;
}
