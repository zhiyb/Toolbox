#include "analog.h"
#include "analogwaveform.h"

Analog::Analog(Device *dev, analog_t *analog, QWidget *parent) : QDialog(parent)
{
	setWindowFlags(Qt::Window);
	//setObjectName(QString::number(analog.id));
	trigger = 0;
	timebase = 0;
	this->dev = dev;
	layout = new QGridLayout(this);
	channelLayout = new QVBoxLayout;
	layout->addWidget(waveform = new AnalogWaveform(this), 0, 0, -1, 1);
	layout->addLayout(channelLayout, 0, 1, -1, 1);
	rebuild(analog);
}

Analog::~Analog()
{
}

bool Analog::event(QEvent *e)
{
	switch (e->type()) {
	case QEvent::WindowActivate:
		setWindowOpacity(1.00);
		break;
	case QEvent::WindowDeactivate:
		setWindowOpacity(0.85);
		break;
	default:
		break;
	}
	return QDialog::event(e);
}

void Analog::showEvent(QShowEvent *e)
{
	startADC();
	QDialog::showEvent(e);
}

void Analog::hideEvent(QHideEvent *e)
{
	stopADC();
	QDialog::hideEvent(e);
}

void Analog::rebuild(analog_t *analog)
{
	this->analog = analog;
	waveform->setAnalog(analog);
	setWindowTitle(analog->name);
	initADC();

	QGroupBox *origTrigger = trigger, *origTimebase = timebase;
	trigger = buildTriggerCtrl();
	timebase = buildTimebaseCtrl();
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
	for (int i = 0; i < analog->channels.count(); i++)
		channelLayout->addWidget(buildChannelCtrl(analog->channels.at(i)));
}

void Analog::initADC(void)
{
	stopADC();
	analog->init();
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

void Analog::analogData(analog_t::data_t data)
{
	if (data.id != analog->id)
		return;
	//qDebug(tr("[%1] Analog data type: %2").arg(QTime::currentTime().toString()).arg(data.type).toLocal8Bit());
	quint32 count = 0;
	switch (data.type) {
	case CTRL_DATA:
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
		break;
	}
	waveform->update();
}

QGroupBox *Analog::buildChannelCtrl(const analog_t::channel_t &channel)
{
	QGroupBox *gb = new QGroupBox(tr("Channel %1").arg(channel.id));
	QVBoxLayout *lay = new QVBoxLayout(gb);
	QLabel *lID = new QLabel(channel.name);
	lay->addWidget(lID);
	/*QLabel *lValue = new QLabel(tr("NaN"));
	lValue->setObjectName(QString::number(channel.id));
	lay->addWidget(lValue);*/
	return gb;
}

QGroupBox *Analog::buildTimebaseCtrl(void)
{
	QGroupBox *gb = new QGroupBox(tr("Timebase"));
	QVBoxLayout *lay = new QVBoxLayout(gb);
	QLabel *lID = new QLabel(tr("Hello, world!"));
	lay->addWidget(lID);
	return gb;
}

QGroupBox *Analog::buildTriggerCtrl(void)
{
	QGroupBox *gb = new QGroupBox(tr("Trigger"));
	QVBoxLayout *lay = new QVBoxLayout(gb);
	QLabel *lID = new QLabel(tr("Hello, world!"));
	lay->addWidget(lID);
	return gb;
}
