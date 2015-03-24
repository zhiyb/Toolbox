#include "analogchannelctrl.h"

AnalogChannelCtrl::AnalogChannelCtrl(Device *dev, analog_t *analog, quint32 channelNum, QWidget *parent) : QGroupBox(parent)
{
	this->dev = dev;
	this->analog = analog;
	this->channel = &analog->channels[channelNum];
	setTitle(tr("%2/%1").arg(channel->name).arg(channel->id));

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

	colour = new ColourSelection(channel->configure.colour());
	layout->addWidget(colour, 1, 0);

	scale = new ScaleValue(&channel->configure.scale, tr("V/div"));
	layout->addWidget(scale, 1, 1);

	connect(offset, SIGNAL(valueChanged(double)), this, SLOT(offsetChanged()));
	connect(colour, SIGNAL(colourChanged(QColor)), this, SLOT(colourChanged(QColor)));
	connect(scale, SIGNAL(valueChanged(float)), this, SLOT(scaleChanged()));
	connect(enabled, SIGNAL(toggled(bool)), this, SLOT(enabledChanged()));
	updateValue();
}

void AnalogChannelCtrl::updateValue(void)
{
	enabled->setChecked(channel->configure.enabled = channel->enabled);
	offset->setValue(channel->offset);
	scale->updateValue();
	scaleChanged();
}

void AnalogChannelCtrl::updateColour(void)
{
	colour->setColour(channel->configure.colour());
}

void AnalogChannelCtrl::offsetChanged(void)
{
	channel->offset = offset->value();
	emit reset();
}

void AnalogChannelCtrl::scaleChanged(void)
{
	if (channel->configure.scale.value() >= 10)
		offset->setSingleStep(1);
	else if (channel->configure.scale.value() >= 1)
		offset->setSingleStep(0.1);
	else if (channel->configure.scale.value() >= 0.1)
		offset->setSingleStep(0.01);
	else
		offset->setSingleStep(0.001);
	emit reset();
}

void AnalogChannelCtrl::enabledChanged(void)
{
	if (channel->configure.enabled == enabled->isChecked())
		return;
	channel->configure.enabled = enabled->isChecked();
	if (!analog->calculate()) {
		updateValue();
		return;
	}
	emit changed();
}

void AnalogChannelCtrl::colourChanged(QColor clr)
{
	channel->configure.setColour(clr);
	emit reset();
}
