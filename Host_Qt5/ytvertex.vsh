#version 100
// Analog YT mode vertex buffer
// x-axis(time) calculation
attribute highp float index;
uniform highp float timebase, frequency;
uniform highp int hCount;
// y-axis(voltage) calculation
attribute highp float data;
uniform highp float reference, offset, scale;
uniform highp int vCount, maxValue;
// coordinate calculation
uniform highp mat4 projection;
uniform highp mat4 modelView;

void main(void)
{
	vec4 position;
	//		time index		total time	total grid	size	centralise
	position.x = index * 2147483648.f * (1.f / frequency) / timebase / float(hCount) * 2.f - 1.f;
	//		ADC value		proportion		voltage	total grid	size
	position.y = (data * 2147483648.f / float(maxValue) * reference + offset) / scale / float(vCount / 2);
	position.z = 0.f;
	position.w = 1.f;
	gl_Position.xyzw = projection * modelView * position;
}
