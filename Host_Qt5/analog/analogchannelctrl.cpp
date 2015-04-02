#include "analogchannelctrl.h"

AnalogChannelCtrl::AnalogChannelCtrl(Device *dev, analog_t *analog, quint32 channelNum, QWidget *parent) : QGroupBox(parent)
{
	this->dev = dev;
	this->analog = analog;
	this->channel = &analog->channel[channelNum];
	setTitle(tr("%2/%1").arg(channel->name).arg(channel->id));

	QHBoxLayout *hlayout = new QHBoxLayout(this);
	QVBoxLayout *layout = new QVBoxLayout;
	hlayout->addLayout(layout);
	layout->addWidget(mode = new QComboBox);
	layout->addWidget(colour = new ColourSelection(channel->configure.colour));
	layout->addWidget(scale = new ScaleValue(&channel->configure.scale, tr("V/div")));

	hlayout->addLayout(layout = new QVBoxLayout);
	layout->addWidget(lOffset = new QLabel);
	layout->addWidget(offset = new Dial);

	mode->setEditable(false);
	mode->addItem(tr("Off"));
	mode->addItem(tr("DC"));
	mode->addItem(tr("AC"));
	mode->setCurrentIndex(analog_t::channel_t::configure_t::DC);
	lOffset->setAlignment(Qt::AlignCenter);

	connect(mode, SIGNAL(currentIndexChanged(int)), this, SLOT(modeChanged(int)));
	connect(offset, SIGNAL(moved(float)), this, SLOT(offsetMoved(float)));
	connect(offset, SIGNAL(rightClicked()), this, SLOT(offsetReset()));
	connect(colour, SIGNAL(colourChanged(QColor)), this, SLOT(colourChanged(QColor)));
	connect(scale, SIGNAL(valueChanged(float)), this, SLOT(scaleChanged()));
	updateValue();
}

void AnalogChannelCtrl::updateValue(void)
{
	channel->updateMode();
	mode->setCurrentIndex(channel->configure.mode);
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

void AnalogChannelCtrl::modeChanged(int mode)
{
	analog_t::channel_t::configure_t::Modes m = (analog_t::channel_t::configure_t::Modes)mode;
	if (channel->configure.mode == m)
		return;
	channel->configure.mode = m;
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
