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
		message_t::set_t set;
		msg.command = CMD_ANALOG;
		msg.id = analog->id;
		set.id = CTRL_START;				// Stop ADC
		set.value = 0;
		set.bytes = 1;
		msg.settings.append(set);
		set.id = CTRL_DATA;				// Set data format
		set.value = analog->scanModeConfigure();
		set.bytes = 1;
		msg.settings.append(set);
		if (!analog->scanModeConfigure()) {
			set.id = CTRL_FRAME;			// Set frame(buffer) length per channel
			set.value = analog->buffer.configure.sizePerChannel;
			set.bytes = 4;
			msg.settings.append(set);
		}
		msg.settings.append(message_t::set_t());	// End settings
		dev->send(msg);

		msg = message_t();
		msg.update.id = analog->id;
		emit updateAt(msg.sequence);
		msg.command = CMD_TIMER;
		msg.id = analog->timer.id;
		set.id = CTRL_SET;				// Set timer
		set.value = analog->timer.configure.value;
		set.bytes = analog->timer.bytes();
		msg.settings.append(set);
		msg.settings.append(message_t::set_t());	// End settings
		dev->send(msg);
	}
}
