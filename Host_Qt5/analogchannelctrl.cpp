#include "analogchannelctrl.h"

AnalogChannelCtrl::AnalogChannelCtrl(Device *dev, analog_t *analog, quint32 channelNum, QWidget *parent) : QGroupBox(parent)
{
	this->dev = dev;
	this->analog = analog;
	this->channel = &analog->channels[channelNum];
	QGridLayout *layout = new QGridLayout(this);
	enabled = new QCheckBox(tr("Enabled"));
	layout->addWidget(enabled, 0, 0);
	offset = new QDoubleSpinBox;
	offset->setMinimum(-INFINITY);
	offset->setMaximum(INFINITY);
	offset->setDecimals(3);
	offset->setSuffix("V");
	offset->setMinimumWidth(65);
	layout->addWidget(offset, 0, 1);
	scale = new ScaleValue(&channel->configure.scale, tr("V/div"));
	layout->addWidget(scale, 1, 1);
	connect(offset, SIGNAL(valueChanged(double)), this, SLOT(offsetChanged()));
	connect(scale, SIGNAL(valueChanged(float)), this, SIGNAL(valueChanged()));
	connect(scale, SIGNAL(valueChanged(float)), this, SLOT(scaleChanged()));
	connect(enabled, SIGNAL(toggled(bool)), this, SLOT(enabledChanged()));
	updateValue();
}

void AnalogChannelCtrl::updateValue()
{
	setTitle(tr("%2/%1").arg(channel->name).arg(channel->id));
	enabled->setChecked(channel->configure.enabled = channel->enabled);
	offset->setValue(channel->offset);
	scale->updateValue();
	scaleChanged();
}

void AnalogChannelCtrl::offsetChanged()
{
	channel->offset = offset->value();
	emit valueChanged();
}

void AnalogChannelCtrl::scaleChanged()
{
	if (channel->configure.scale.value() >= 10)
		offset->setSingleStep(1);
	else if (channel->configure.scale.value() >= 1)
		offset->setSingleStep(0.1);
	else if (channel->configure.scale.value() >= 0.1)
		offset->setSingleStep(0.01);
	else
		offset->setSingleStep(0.001);
}

void AnalogChannelCtrl::enabledChanged()
{
	if (channel->configure.enabled == enabled->isChecked())
		return;
	channel->configure.enabled = enabled->isChecked();
	if (!analog->calculate()) {
		updateValue();
		return;
	}

	message_t msg;
	msg.command = CMD_ANALOG;
	msg.id = analog->id;
	message_t::set_t set;
	set.id = CTRL_START;				// Stop ADC
	set.value = 0;
	set.bytes = 1;
	msg.settings.append(set);
	set.id = CTRL_SET;				// Set channel
	set.value = analog->channelsEnabledConfigure();
	set.bytes = analog->channelsBytes();
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
	//qDebug(tr("AnalogChannelCtrl: message %1").arg(msg.sequence).toLocal8Bit());
}
