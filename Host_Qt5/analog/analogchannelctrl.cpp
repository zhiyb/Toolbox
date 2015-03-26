#include "analogchannelctrl.h"

AnalogChannelCtrl::AnalogChannelCtrl(Device *dev, analog_t *analog, quint32 channelNum, QWidget *parent) : QGroupBox(parent)
{
	this->dev = dev;
	this->analog = analog;
	this->channel = &analog->channel[channelNum];
	setTitle(tr("%2/%1").arg(channel->name).arg(channel->id));

	QGridLayout *layout = new QGridLayout(this);

	layout->addWidget(enabled = new QCheckBox(tr("Enabled")), 0, 0);
	layout->addWidget(lOffset = new QLabel, 0, 1);
	layout->addWidget(offset = new Dial, 1, 1, 2, 1);
	layout->addWidget(colour = new ColourSelection(channel->configure.colour), 1, 0);
	layout->addWidget(scale = new ScaleValue(&channel->configure.scale, tr("V/div")), 2, 0);

	connect(offset, SIGNAL(moved(float)), this, SLOT(offsetMoved(float)));
	connect(offset, SIGNAL(rightClicked()), this, SLOT(offsetReset()));
	connect(colour, SIGNAL(colourChanged(QColor)), this, SLOT(colourChanged(QColor)));
	connect(scale, SIGNAL(valueChanged(float)), this, SLOT(scaleChanged()));
	connect(enabled, SIGNAL(toggled(bool)), this, SLOT(enabledChanged()));
	updateValue();
}

void AnalogChannelCtrl::updateValue(void)
{
	enabled->setChecked(channel->configure.enabled = channel->enabled);
	scale->updateValue();
	updateOffset();
	scaleChanged();
}

void AnalogChannelCtrl::updateColour(void)
{
	colour->setColour(channel->configure.colour);
}

void AnalogChannelCtrl::offsetReset()
{
	offset->reset();
	channel->configure.displayOffset = 0;
	updateOffset();
	emit updateRequest();
}

void AnalogChannelCtrl::offsetMoved(float frac)
{
	qreal step = channel->configure.scale.value() * (qreal)analog->grid.count.height() / 2.f / 2.f;
	channel->configure.displayOffset += step * frac;
	updateOffset();
	emit updateRequest();
}

void AnalogChannelCtrl::scaleChanged(void)
{
	emit updateRequest();
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
	emit updateRequest();
}

void AnalogChannelCtrl::colourChanged(QColor clr)
{
	channel->configure.colour = clr;
	emit updateRequest();
}

void AnalogChannelCtrl::updateOffset()
{
	lOffset->setText(tr("%1V").arg(scale_t::toString(channel->configure.displayOffset, true)));
}
