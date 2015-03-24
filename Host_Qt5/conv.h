#ifndef CONV_H
#define CONV_H

#include <QColor>
#include <QVector4D>

namespace conv
{
	float rawUInt32ToFloat(quint32 x);
	QVector4D colorToVector4D(const QColor& colour);
	QColor vector4DToColor(const QVector4D &vec);
}

#endif // CONV_H
