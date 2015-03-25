#include "analogtriggerctrl.h"

AnalogTriggerCtrl::AnalogTriggerCtrl(Device *dev, analog_t *analog, QWidget *parent) : QGroupBox(parent)
{
	this->dev = dev;
	this->analog = analog;
	setTitle("Trigger");

	QVBoxLayout *layout = new QVBoxLayout(this);

	source = new QComboBox;
	source->setEditable(false);
	source->addItem(tr("Free running"));
	for (int i = 0; i < analog->channels.count(); i++) {
		const analog_t::channel_t &channel = analog->channels.at(i);
		source->addItem(tr("%2/%1").arg(channel.name).arg(channel.id));
	}
	layout->addWidget(source);

	QLabel *lID = new QLabel(tr("Hello, world!"));
	layout->addWidget(lID);

	reset();
	connect(source, SIGNAL(currentIndexChanged(int)), this, SLOT(sourceChanged(int)));
}

void AnalogTriggerCtrl::sourceChanged(int idx)
{
	if (idx < 0 || idx > analog->channels.count()) {
		qDebug(tr("[WARNING] Trigger source index out of range").toLocal8Bit());
		return;
	}
	analog->trigger.configure.source = idx == 0 ? INVALID_ID : analog->channels.at(idx - 1).id;
	if (analog->trigger.source == analog->trigger.configure.source)
		return;
	if (!analog->calculate()) {
		reset();
		return;
	}
	emit updateTrigger();
}

void AnalogTriggerCtrl::reset(void)
{
	int idx = analog->triggerChannelIndex();
	source->setCurrentIndex(idx < 0 ? 0 : idx + 1);
}
