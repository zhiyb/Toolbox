#version 100
// Analog YT mode vertex buffer

// x-axis(time) calculation
attribute highp float index;
uniform highp float timebase, frequency;
uniform highp int hCount;

// y-axis(voltage) calculation
// ADC data
attribute highp float data;
// ADC reference voltage, voltage offset, display scale (grid), ADC value offset (AC/DC)
uniform highp float reference, offset, scale, vOffset;
// Vertical grid count, ADC maximum value
uniform highp int vCount, maxValue;

// coordinate calculation
uniform highp mat4 projection;
uniform highp mat4 modelView;

void main(void)
{
	vec4 position;
	float v;
	//	time index   total time  total grid	size	   centralise
	position.x = index / frequency / timebase / float(hCount) * 2.f - 1.f;

	// ADC value
	v = data * 2147483648.f + vOffset;
	// Voltage
	v = v / float(maxValue) * reference + offset;
	// Total grid
	v /= scale;
	// Screen position
	position.y = v / float(vCount / 2);

	position.z = 0.f;
	position.w = 1.f;
	gl_Position.xyzw = projection * modelView * position;
	gl_PointSize = 2.f;
}
