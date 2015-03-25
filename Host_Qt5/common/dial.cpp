#include "dial.h"

Dial::Dial(QWidget *parent) : QDial(parent)
{
	prec = 1000;
	setMinimum(0);
	setMaximum(prec);
	setWrapping(true);
	reset();
}

void Dial::setPrecision(int v)
{
	setMaximum(v);
	setValue(value() * v / prec);
	prec = v;
	prev = value();
}

void Dial::reset(void)
{
	disconnect(this, SLOT(valueChanged(int)));
	prec = 1000;
	setMinimum(0);
	setMaximum(prec);
	setValue(prev = precision() / 2);
	connect(this, SIGNAL(valueChanged(int)), this, SLOT(valueChanged(int)));
}

void Dial::mouseReleaseEvent(QMouseEvent *me)
{
	QDial::mouseReleaseEvent(me);
	if (me->button() == Qt::RightButton && me->x() >= 0 && me->y() >= 0 && me->x() < width() && me->y() < height())
		emit rightClicked();
}

void Dial::valueChanged(int v)
{
	emit moved((qreal)stepsFrom(prev) / (qreal)precision());
	prev = v;
}

int Dial::stepsFrom(int v)
{
	int diff = value() - v;
	if (abs(diff) >= precision() / 2)
		diff = diff + (diff < 0 ? precision() : -precision());
	return diff;
}
