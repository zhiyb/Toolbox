#include "conv.h"

float conv::rawUInt32ToFloat(quint32 x)
{
	union {
		float f;	// assuming 32-bit IEEE 754 single-precision
		quint32 i;	// assuming 32-bit unsigned int
	} u;
	u.i = x;
	return u.f;
}

QVector4D conv::colorToVector4D(const QColor &colour)
{
	return QVector4D(colour.redF(), colour.greenF(), colour.blueF(), colour.alphaF());
}

QColor conv::vector4DToColor(const QVector4D &vec)
{
	QColor colour;
	colour.setRedF(vec.x());
	colour.setGreenF(vec.y());
	colour.setBlueF(vec.z());
	colour.setAlphaF(vec.w());
	return colour;
}
