#include "analogtimebasectrl.h"

AnalogTimebaseCtrl::AnalogTimebaseCtrl(Device *dev, analog_t *analog, QWidget *parent) : QGroupBox(parent)
{
	this->dev = dev;
	this->analog = analog;
	setTitle(tr("Timebase"));

	QVBoxLayout *layout = new QVBoxLayout(this);

	scale = new ScaleValue(&analog->timebase.configure.scale, "s");
	layout->addWidget(scale);

	connect(scale, SIGNAL(valueChanged(float)), this, SLOT(scaleChanged()));
}

AnalogTimebaseCtrl::~AnalogTimebaseCtrl()
{
}

void AnalogTimebaseCtrl::scaleChanged()
{
	if (!analog->calculate()) {
		analog->timebase.configure.scale = analog->timebase.scale;
		scale->updateValue();
	} else {
		emit updateRequest();
		emit updateDisplay();
	}
}