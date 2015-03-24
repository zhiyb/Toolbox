#include "analogtriggerctrl.h"

AnalogTriggerCtrl::AnalogTriggerCtrl(Device *dev, analog_t *analog, QWidget *parent) : QGroupBox(parent)
{
	this->dev = dev;
	this->analog = analog;
	setTitle("Trigger");

	QVBoxLayout *layout = new QVBoxLayout(this);
	QLabel *lID = new QLabel(tr("Hello, world!"));
	layout->addWidget(lID);
}

AnalogTriggerCtrl::~AnalogTriggerCtrl()
{
}
