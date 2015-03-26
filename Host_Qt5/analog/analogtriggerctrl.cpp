#include "analogtriggerctrl.h"
#include "dial.h"

AnalogTriggerCtrl::AnalogTriggerCtrl(Device *dev, analog_t *analog, QWidget *parent) : QGroupBox(parent)
{
	this->dev = dev;
	this->analog = analog;
	setTitle("Trigger");

	QVBoxLayout *layout = new QVBoxLayout(this);

	layout->addWidget(source = new QComboBox);
	layout->addWidget(edge = new QComboBox);
	layout->addWidget(lLevel = new QLabel);
	layout->addWidget(dLevel = new Dial);

	source->setEditable(false);
	source->addItem(tr("Free running"));
	for (int i = 0; i < analog->channel.count(); i++) {
		const analog_t::channel_t &channel = analog->channel.at(i);
		source->addItem(tr("%2/%1").arg(channel.name).arg(channel.id));
	}

	edge->setEditable(false);
	edge->addItem(tr("Rising edge"));
	edge->addItem(tr("Falling edge"));

	reset();
	updateDisplay();
	connect(source, SIGNAL(currentIndexChanged(int)), this, SLOT(sourceChanged(int)));
	connect(edge, SIGNAL(currentIndexChanged(int)), this, SLOT(edgeChanged(int)));
	connect(dLevel, SIGNAL(moved(float)), this, SLOT(levelChanged(float)));
	connect(dLevel, SIGNAL(rightClicked()), this, SLOT(levelReset()));
}

void AnalogTriggerCtrl::sourceChanged(int idx)
{
	if (idx < 0 || idx > analog->channel.count()) {
		qDebug(tr("[WARNING] Trigger source index out of range").toLocal8Bit());
		return;
	}
	analog->trigger.configure.source = idx == 0 ? INVALID_ID : analog->channel.at(idx - 1).id;
	if (!analog->trigger.sourceUpdateRequired())
		return;
	if (!analog->calculate())
		reset();
	else
		emit updateRequest();
	updateDisplay();
}

void AnalogTriggerCtrl::edgeChanged(int idx)
{
	analog->trigger.configure.edge = idx ? analog_t::trigger_t::Falling : analog_t::trigger_t::Rising;
	if (!analog->calculate())
		reset();
	else
		emit updateRequest();
	updateDisplay();
}

void AnalogTriggerCtrl::levelChanged(float frac)
{
	if (!analog->trigger.enabled(true))
		return;
	analog->trigger.configure.dispLevel += frac;
	if (!analog->calculate())
		reset();
	else
		emit updateRequest();
	updateDisplay();
}

void AnalogTriggerCtrl::levelReset(void)
{
	if (analog->trigger.enabled(true)) {
		const analog_t::channel_t *ch = analog->triggerChannel(true);
		int min = 0, max = 0;
		for (quint32 i = 0; i < analog->buffer.validSize; i++) {
			int adc = ch->buffer.at(i);
			if (adc < min)
				min = adc;
			if (adc > max)
				max = adc;
		}
		analog->trigger.configure.dispLevel = analog->adcToScreenY(min + (max - min) / 2, ch);
	}
	if (!analog->calculate())
		reset();
	else {
		dLevel->reset();
		emit updateRequest();
	}
	updateDisplay();
}

void AnalogTriggerCtrl::reset(void)
{
	analog->trigger.reset();
	if (analog->trigger.enabled()) {
		analog->trigger.configure.dispLevel = analog->adcToScreenY(analog->trigger.level, analog->trigger.channel());
		analog->trigger.configure.dispPosition = analog->countToScreenX(analog->trigger.position);
	}
}

void AnalogTriggerCtrl::updateDisplay(void)
{
	int idx = analog->triggerChannelIndex(true);
	source->setCurrentIndex(idx < 0 ? 0 : idx + 1);
	edge->setCurrentIndex(analog->trigger.configure.edge == analog_t::trigger_t::Rising ? 0 : 1);
	if (analog->trigger.enabled(true))
		lLevel->setText(tr("Level: %1V").arg(scale_t::toString(analog->voltageFromScreenY(analog->trigger.configure.dispLevel, analog->trigger.channel(true)), true)));
	else
		lLevel->setText(tr("N/A"));
}
