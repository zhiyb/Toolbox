#version 100
attribute highp float data;
attribute highp float index;
uniform highp int maxValue;
uniform highp int hCount;
uniform highp float timebase;
uniform highp float frequency;
uniform highp mat4 projection;
uniform highp mat4 modelView;

void main(void)
{
	vec4 position;
	//			time index	total time	total grid	size	centralise
	position.x = index * 2147483648.f * (1.f / frequency) / (timebase * float(hCount)) * 2.f - 1.f;
	position.y = data * 2147483648.f / float(maxValue);
	position.z = 0.f;
	position.w = 1.f;
	gl_Position.xyzw = projection * modelView * position;
}
