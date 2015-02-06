#include "scalevalue.h"

ScaleValue::ScaleValue(scale_t *scale, QString unit, QWidget *parent) : QToolButton(parent)
{
	//setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
	setMinimumWidth(80);
	this->scale = scale;
	this->unit = unit;
	updateValue();
}

ScaleValue::~ScaleValue()
{
}

void ScaleValue::updateValue()
{
	setText(tr("- %1%2 +").arg(scale->toString()).arg(unit));
}

void ScaleValue::mousePressEvent(QMouseEvent *e)
{
	if (e->pos().x() < width() / 2) {
		if (!scale->decrease())
			goto ret;
	} else {
		if (!scale->increase())
			goto ret;
	}
	updateValue();
	emit valueChanged(scale->value());
ret:
	QToolButton::mousePressEvent(e);
}
