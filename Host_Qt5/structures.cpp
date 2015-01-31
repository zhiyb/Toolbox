#include "structures.h"
#include <QDebug>
#include <QObject>

const qreal scale_t::factor[3] = {1, 2.5, 5};
const quint32 analog_t::grid_t::preferredPointsPerGrid = 100;
const quint32 analog_t::grid_t::minimumVerticalCount = 8;
const quint32 analog_t::grid_t::maximumVerticalCount = 12;

quint32 messageCount = 0;

bool message_t::similar(const message_t &msg) const
{
	if (msg.command != command || msg.id != id || msg.settings.count() != settings.count())
		return false;
	for (int i = 0; i < msg.settings.count(); i++) {
		const struct message_t::set_t *set = &settings[i], *setIn = &msg.settings.at(i);
		if (set->id != setIn->id /*|| set->bytes != setIn->bytes*/)
			return false;
	}
	return true;
}

bool timer_t::setFrequency(const float freq)
{
	quint32 value = clockFrequency / freq;
	//qDebug(QObject::tr("Timer set frequency: %1, clock frequency: %2, value: %3, maximum: %4").arg(freq).arg(clockFrequency).arg(value).arg(maximum()).toLocal8Bit());
	if (value == 0 || value > maximum())
		return false;
	configure.value = value;
	return true;
}

void scale_t::increase(void)
{
	if (divide == 1 && seq == 2)
		return;
	seq = seq == 2 ? 0 : seq + 1;
	divide /= seq == 0 ? 10 : 1;
}

void scale_t::decrease(void)
{
	 seq = seq == 0 ? 2 : seq - 1;
	 divide *= seq == 2 ? 10 : 1;
}

QString scale_t::toString(const qreal value)
{
	const QString units[] = {"%1", "%1m", "%1μ", "%1n", "%1p"};
	quint32 unit = 0;
	qreal v = value;
	while (abs(v) < 1 && unit++ < sizeof(units) / sizeof(units[0]))
		v *= 1000;
	if (unit >= sizeof(units) / sizeof(units[0]))
		return QString(units[0]).arg(0);
	return QString(units[unit]).arg(v);
}

quint32 analog_t::channelCount(void) const
{
	quint32 count = 0;
	for (int i = 0; i < channels.count(); i++)
		count += channels.at(i).enabled;
	return count;
}

quint32 analog_t::channelEnabled(void) const
{
	quint32 enabled = 0, mask = 1;
	for (int i = 0; i < channels.count(); i++) {
		if (channels.at(i).enabled)
			enabled |= mask;
		mask <<= 1;
	}
	return enabled;
}

void analog_t::init(void)
{
	//update();
}

void analog_t::update(void)
{
	if (timebase.scanMode()) {
		quint32 multiple = 1;
		while (!timer.setFrequency(timebase.scale.value() * grid.preferredPointsPerGrid * multiple))
			multiple++;
		buffer.sizePerChannel = gridTotalTime() * timer.frequency();
		buffer.position = 0;
		buffer.validSize = 0;
	} else {
		buffer.position = 0;
		buffer.validSize = buffer.sizePerChannel;
	}
	for (int i = 0; i < channels.count(); i++)
		channels[i].buffer.resize(buffer.sizePerChannel);
}

analog_t::grid_t::preference_t::preference_t(void) : bgColour(0.f, 0.f, 0.f, 1.f), \
	gridColour(0.5f, 0.5f, 0.5f, 1.f), gridPointSize(3) {}
