#include "timebasectrl.h"

TimebaseCtrl::TimebaseCtrl(Device *dev, analog_t *analog, QWidget *parent) : QGroupBox(parent)
{
	setTitle(tr("Timebase"));
	this->dev = dev;
	this->analog = analog;
	QVBoxLayout *layout = new QVBoxLayout(this);
	scale = new ScaleValue(&analog->timebase.configure.scale, "s");
	layout->addWidget(scale);
	connect(scale, SIGNAL(valueChanged(float)), this, SLOT(scaleChanged()));
}

TimebaseCtrl::~TimebaseCtrl()
{
}

void TimebaseCtrl::scaleChanged()
{
	if (!analog->calculate()) {
		analog->timebase.configure.scale = analog->timebase.scale;
		scale->updateValue();
	} else {
		message_t msg;
		msg.command = CMD_TIMER;
		msg.id = analog->id;
		message_t::set_t set;
		set.id = CTRL_SET;
		set.value = analog->timer.configure.value;
		set.bytes = analog->timer.bytes();
		msg.settings.append(set);
		msg.settings.append(message_t::set_t());
		emit updateAt(msg.sequence);
		dev->send(msg);
	}
}
