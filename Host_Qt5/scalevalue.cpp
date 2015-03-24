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

void ScaleValue::mouseReleaseEvent(QMouseEvent *e)
{
	QToolButton::mouseReleaseEvent(e);
	if (!e->isAccepted())
		return;
	if (e->pos().x() < width() / 2) {
		if (!scale->decrease())
			return;
	} else {
		if (!scale->increase())
			return;
	}
	updateValue();
	emit valueChanged(scale->value());
}
