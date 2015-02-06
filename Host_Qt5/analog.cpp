#include "analog.h"
#include "analogwaveform.h"
#include "analogchannelctrl.h"

#define CHANNEL_ROW_COUNT	4

Analog::Analog(Device *dev, analog_t *analog, QWidget *parent) : QWidget(parent)
{
	setWindowFlags(Qt::Window);
	//setObjectName(QString::number(analog.id));
	trigger = 0;
	timebase = 0;
	this->dev = dev;
	layout = new QGridLayout(this);
	channelLayout = new QGridLayout;
	layout->addWidget(waveform = new AnalogWaveform(dev, this), 0, 0, -1, 1);
	layout->addLayout(channelLayout, 0, 1, -1, 1);
	rebuild(analog);
	connect(waveform, SIGNAL(updateAt(quint32)), this, SLOT(updateAt(quint32)));
}

Analog::~Analog()
{
}

bool Analog::event(QEvent *e)
{
	switch (e->type()) {
	case QEvent::WindowActivate:
		setWindowOpacity(0.90);
		break;
	case QEvent::WindowDeactivate:
		setWindowOpacity(0.85);
		break;
	default:
		break;
	}
	return QWidget::event(e);
}

void Analog::showEvent(QShowEvent *e)
{
	startADC();
	QWidget::showEvent(e);
}

void Analog::hideEvent(QHideEvent *e)
{
	stopADC();
	analog->buffer.validSize = 0;
	analog->buffer.position = 0;
	QWidget::hideEvent(e);
}

void Analog::updateAt(quint32 sequence)
{
	updateSequence = sequence;
}

void Analog::rebuild(analog_t *analog)
{
	this->analog = analog;
	waveform->setAnalog(analog);
	setWindowTitle(analog->name);
	initADC();

	QGroupBox *origTrigger = trigger;
	TimebaseCtrl *origTimebase = timebase;
	trigger = buildTriggerCtrl();
	timebase = new TimebaseCtrl(dev, analog);
	connect(timebase, SIGNAL(updateAt(quint32)), this, SLOT(updateAt(quint32)));
	if (!origTrigger)
		layout->addWidget(trigger, 0, 2);
	else
		delete layout->replaceWidget(origTrigger, trigger);
	if (!origTimebase)
		layout->addWidget(timebase, 1, 2);
	else
		delete layout->replaceWidget(origTimebase, timebase);

	while (channelLayout->count()) {
		QLayoutItem *item = layout->itemAt(0);
		channelLayout->removeItem(item);
		delete item;
	}
	for (int i = 0; i < analog->channels.count(); i++) {
		AnalogChannelCtrl *ctrl = new AnalogChannelCtrl(dev, analog, i);
		channelLayout->addWidget(ctrl, i % CHANNEL_ROW_COUNT, i / CHANNEL_ROW_COUNT);
		connect(ctrl, SIGNAL(updateAt(quint32)), this, SLOT(updateAt(quint32)));
	}
}

void Analog::initADC(void)
{
	//stopADC();
	message_t msg;
	msg.command = CMD_ANALOG;
	msg.id = analog->id;
	message_t::set_t set;
	set.id = CTRL_START;				// Stop ADC
	set.value = 0;
	set.bytes = 1;
	msg.settings.append(set);
	set.id = CTRL_DATA;				// Set data format
	set.value = analog->timebase.scanMode() ? CTRL_DATA : CTRL_FRAME;
	set.bytes = 1;
	msg.settings.append(set);
	dev->send(msg);

	analog->init();
	analog->calculate();
	analog->update();

	configureTimer();
	//startADC();
}

void Analog::startADC(void)
{
	message_t msg;
	msg.command = CMD_ANALOG;
	msg.id = analog->id;
	message_t::set_t set;
	set.id = CTRL_START;
	set.value = 1;
	set.bytes = 1;
	msg.settings.append(set);
	msg.settings.append(message_t::set_t());	// END
	dev->send(msg);
}

void Analog::stopADC(void)
{
	message_t msg;
	msg.command = CMD_ANALOG;
	msg.id = analog->id;
	message_t::set_t set;
	set.id = CTRL_START;
	set.value = 0;
	set.bytes = 1;
	msg.settings.append(set);
	msg.settings.append(message_t::set_t());	// END
	dev->send(msg);
}

void Analog::configureTimer(void)
{
	message_t msg;
	msg.command = CMD_TIMER;
	msg.id = analog->timer.id;
	message_t::set_t set;
	set.id = CTRL_SET;
	set.bytes = analog->timer.bytes();
	set.value = analog->timer.configure.value;
	msg.settings.append(set);
	/*set.id = CTRL_START;
	set.value = 1;
	msg.settings.append(set);*/
	msg.settings.append(message_t::set_t());	// END
	dev->send(msg);
}

void Analog::activate(void)
{
	show();
	activateWindow();
}

void Analog::messageSent(quint32 sequence)
{
	if (!updateSequence || sequence != updateSequence)
		return;
	//qDebug(tr("Analog::messageSent: %1").arg(sequence).toLocal8Bit());
	updateSequence = 0;
	analog->update();
	startADC();
}

void Analog::analogData(analog_t::data_t data)
{
	if (data.id != analog->id)
		return;
	//qDebug(tr("[%1] Analog data type: %2").arg(QTime::currentTime().toString()).arg(data.type).toLocal8Bit());
	quint32 count = 0;
	switch (data.type) {
	case CTRL_DATA:
		if ((quint32)data.data.count() != analog->channelsCount()) {
			qDebug(tr("Data size mismatch: %1/%2, ignored").arg(data.data.count()).arg(analog->channelsCount()).toLocal8Bit());
			break;
		}
		for (int i = 0; i < analog->channels.count(); i++)
			if (analog->channels.at(i).enabled)
				analog->channels[i].buffer[analog->buffer.position] = data.data.at(count++);
		analog->buffer.position++;
		if (analog->buffer.validSize < analog->buffer.position)
			analog->buffer.validSize = analog->buffer.position;
		if (analog->buffer.position == analog->buffer.sizePerChannel)
			analog->buffer.position = 0;
		break;
	case CTRL_FRAME:
		if ((quint32)data.data.count() != analog->channelsCount() * analog->buffer.sizePerChannel) {
			qDebug(tr("Data size mismatch: %1/%2, ignored").arg(data.data.count()).arg(analog->channelsCount()).toLocal8Bit());
			break;
		}
		for (quint32 pos = 0; pos < analog->buffer.sizePerChannel; pos++)
			for (int i = 0; i < analog->channels.count(); i++)
				if (analog->channels.at(i).enabled)
					analog->channels[i].buffer[pos] = data.data.at(count++);
		analog->buffer.validSize = analog->buffer.sizePerChannel;
		break;
	}
	waveform->update();
}

QGroupBox *Analog::buildTriggerCtrl(void)
{
	QGroupBox *gb = new QGroupBox(tr("Trigger"));
	QVBoxLayout *lay = new QVBoxLayout(gb);
	QLabel *lID = new QLabel(tr("Hello, world!"));
	lay->addWidget(lID);
	return gb;
}
