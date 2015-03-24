#include "analog.h"
#include "analogwaveform.h"
#include "analogchannelctrl.h"
#include "analogtriggerctrl.h"

#define CHANNEL_ROW_COUNT	4

Analog::Analog(Device *dev, analog_t *analog, QWidget *parent) : QWidget(parent)
{
	setWindowFlags(Qt::Window);
	//setObjectName(QString::number(analog.id));
	trigger = 0;
	timebase = 0;
	updateSequence = 0;
	this->dev = dev;
	layout = new QGridLayout(this);
	channelLayout = new QGridLayout;
	layout->addWidget(waveform = new AnalogWaveform(dev, this), 0, 0, -1, 1);
	layout->addLayout(channelLayout, 0, 1, -1, 1);
	rebuild(analog);
	connect(waveform, SIGNAL(updateAt(quint32)), this, SLOT(updateAt(quint32)));
	connect(waveform, SIGNAL(reset()), this, SLOT(updateMode()));
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
	resize(size());
	startADC(true);
	QWidget::showEvent(e);
}

void Analog::hideEvent(QHideEvent *e)
{
	startADC(false);
	analog->buffer.validSize = 0;
	analog->buffer.position = 0;
	QWidget::hideEvent(e);
}

void Analog::updateAt(quint32 sequence)
{
	updateSequence = sequence;
}

void Analog::configure(bool channel)
{
	message_t msg(CMD_ANALOG, analog->id);
	// Stop ADC
	msg.settings.append(message_t::set_t(CTRL_START, 1, 0));
	// Set channel
	if (channel)
		msg.settings.append(message_t::set_t(CTRL_SET, analog->channelsBytes(), analog->channelsEnabledConfigure()));
	// Set data format
	msg.settings.append(message_t::set_t(CTRL_DATA, 1, analog->scanModeConfigure()));
	// Set frame(buffer) length per channel
	if (!analog->scanModeConfigure())
		msg.settings.append(message_t::set_t(CTRL_FRAME, 4, analog->buffer.configure.sizePerChannel));
	// End settings
	msg.settings.append(message_t::set_t());
	dev->send(msg);
	//qDebug(tr("[DEBUG] Analog::channelChanged: Message %1").arg(msg.sequence).toLocal8Bit());

	configureTimer();
}

void Analog::rebuild(analog_t *analog)
{
	this->analog = analog;
	waveform->setAnalog(analog);
	setWindowTitle(analog->name);
	initADC();

	AnalogTriggerCtrl *origTrigger = trigger;
	TimebaseCtrl *origTimebase = timebase;
	trigger = new AnalogTriggerCtrl(dev, analog);
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
		connect(ctrl, SIGNAL(reset()), waveform, SLOT(update()));
		connect(ctrl, SIGNAL(changed()), this, SLOT(updateChannel()));
	}
}

void Analog::initADC(void)
{
	message_t msg(CMD_ANALOG, analog->id);
	// Stop ADC
	msg.settings.append(message_t::set_t(CTRL_START, 1, 0));
	// Set data format
	msg.settings.append(message_t::set_t(CTRL_DATA, 1, analog->scanMode()));
	// End settings
	msg.settings.append(message_t::set_t());
	dev->send(msg);

	analog->init();
	analog->calculate();
	//analog->update();

	configureTimer();
	//startADC();
}

void Analog::startADC(bool start)
{
	message_t msg(CMD_ANALOG, analog->id);
	// Start ADC
	msg.settings.append(message_t::set_t(CTRL_START, 1, start));
	// End settings
	msg.settings.append(message_t::set_t());
	dev->send(msg);

	analog->buffer.reset();
}

void Analog::configureTimer(void)
{
	message_t msg(CMD_TIMER, analog->timer.id);
	msg.update.id = analog->id;
	emit updateAt(msg.sequence);
	// Set timer period
	msg.settings.append(message_t::set_t(CTRL_SET, analog->timer.bytes(), analog->timer.configure.value));
	// End settings
	msg.settings.append(message_t::set_t());
	dev->send(msg);
	//qDebug(tr("[DEBUG] Analog::configureTimer: Message %1").arg(msg.sequence).toLocal8Bit());
}

void Analog::activate(void)
{
	if (isVisible())
		hide();
	else {
		show();
		activateWindow();
	}
}

void Analog::messageSent(quint32 sequence)
{
	if (!updateSequence || sequence != updateSequence)
		return;
	//qDebug(tr("Analog::messageSent: %1").arg(sequence).toLocal8Bit());
	updateSequence = 0;
	//analog->update();
	startADC(true);
	waveform->update();
}

void Analog::analogData(analog_t::data_t data)
{
	if (data.id != analog->id)
		return;
	/*if (analog->buffer.validSize == analog->buffer.sizePerChannel)
		return;*/
	//qDebug(tr("[%1] Analog data type: %2, count: %3").arg(QTime::currentTime().toString()).arg(data.type).arg(data.data.count()).toLocal8Bit());
	quint32 count = 0;
	switch (data.type) {
	case CTRL_DATA:
		if ((quint32)data.data.count() != analog->channelsCount()) {
			//qDebug(tr("Data size mismatch: %1/%2, ignored").arg(data.data.count()).arg(analog->channelsCount()).toLocal8Bit());
			return;
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
			//qDebug(tr("Data size mismatch: %1/%2, ignored").arg(data.data.count()).arg(analog->channelsCount()).toLocal8Bit());
			return;
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
