#include <cmath>
#include <QDebug>
#include <QObject>
#include "structures.h"
#include "conv.h"

const qreal scale_t::factor[3] = {1, 2.5, 5};
const quint32 analog_t::grid_t::preferredPointsPerGrid = 150;
const quint32 analog_t::grid_t::minimumPointsPerGrid = 10;
const quint32 analog_t::grid_t::maximumVerticalCount = 10;
const quint32 analog_t::grid_t::minimumVerticalCount = 8;
const QColor analog_t::channel_t::defaultColours[DEFAULT_CHANNEL_COLOURS] = {
	Qt::red, Qt::green, Qt::blue, Qt::yellow,
	Qt::cyan, Qt::magenta, Qt::gray, Qt::white,
};

quint32 messageCount = 0;
static quint32 colourCount = 0;

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

bool hwtimer_t::setFrequency(const qreal freq)
{
	quint32 value = std::ceil((qreal)clockFrequency / freq);
	//qDebug(QObject::tr("Timer set frequency: %1, clock frequency: %2, value: %3, maximum: %4").arg(freq).arg(clockFrequency).arg(value).arg(maximum()).toLocal8Bit());
	if (value == 0)
		value++;
	if (value > maximum())
		return false;
	configure.value = value;
	return true;
}

bool scale_t::increase(void)
{
	if (divide == 1 && seq == 2)
		return false;
	seq = seq == 2 ? 0 : seq + 1;
	divide /= seq == 0 ? 10 : 1;
	return true;
}

bool scale_t::decrease(void)
{
	 seq = seq == 0 ? 2 : seq - 1;
	 divide *= seq == 2 ? 10 : 1;
	 return true;
}

QString scale_t::toString(const qreal value, bool prec)
{
	const QString units[] = {"%1", "%1m", "%1μ", "%1n", "%1p"};
	quint32 unit = 0;
	qreal v = value;
	while (abs(v) < 1 && unit++ < sizeof(units) / sizeof(units[0]))
		v *= 1000;
	if (unit >= sizeof(units) / sizeof(units[0]))
		return QString(units[0]).arg(0);
	if (prec)
		return QString(units[unit]).arg(v, 0, 'f', 3);
	else
		return QString(units[unit]).arg(v);
}

quint32 analog_t::channelsCount(bool conf) const
{
	quint32 count = 0;
	for (int i = 0; i < channels.count(); i++)
		count += channelEnabled(i, conf);
	return count;
}

void analog_t::setChannelsEnabled(quint32 enabled)
{
	quint32 mask = 1;
	for (int i = 0; i < channels.count(); i++) {
		channels[i].enabled = channels[i].configure.enabled = enabled & mask;
		mask <<= 1;
	}
}

quint32 analog_t::channelsEnabled(bool conf) const
{
	quint32 enabled = 0, mask = 1;
	for (int i = 0; i < channels.count(); i++) {
		if (channelEnabled(i, conf))
			enabled |= mask;
		mask <<= 1;
	}
	return enabled;
}

bool analog_t::updateRequired() const
{
	bool upd = channelsEnabled(false) != channelsEnabled(true);
	if (!scanMode(true))
		upd |= buffer.updateRequired();
	upd |= timebase.updateRequired();
	upd |= trigger.updateRequired();
	//qDebug(QObject::tr("[DEBUG] analog_t::updateRequired: %1").arg(upd).toLocal8Bit());
	return upd;
}

int analog_t::findChannelIndex(quint8 id) const
{
	if (id == INVALID_ID)
		return -1;
	for (int i = 0; i < channels.count(); i++)
		if (channels.at(i).id == id)
			return i;
	return -1;
}

analog_t::channel_t *analog_t::findChannel(quint8 id)
{
	int idx = findChannelIndex(id);
	return idx < 0 ? 0 : &channels[idx];
}

const analog_t::channel_t *analog_t::findChannel(quint8 id) const
{
	int idx = findChannelIndex(id);
	return idx < 0 ? 0 : &channels.at(idx);
}

void analog_t::init(void)
{
	//update();
}

bool analog_t::calculate(void)
{
	//qDebug() << "[DEBUG] analog_t::calculate";
	if (!timer.setFrequency((float)grid.preferredPointsPerGrid / timebase.configure.scale.value())) {
		//qDebug(QObject::tr("[INFO] analog_t::calculate: Failed to configure timer, set to maximum").toLocal8Bit());
		timer.configure.value = timer.maximum();
	}
	if (!scanMode(true)) {
		quint32 sizePerChannel = buffer.size / channelsCount(true);
		if (sizePerChannel / grid.count.width() < grid.minimumPointsPerGrid) {
			qDebug(QObject::tr("[WARNING] analog_t::calculate: Buffer too small").toLocal8Bit());
			return false;
		}
		if (sizePerChannel / grid.count.width() > grid.preferredPointsPerGrid)
			sizePerChannel = grid.preferredPointsPerGrid * grid.count.width();
		if (!timer.setFrequency((float)sizePerChannel / (float)grid.count.width() / timebase.configure.scale.value())) {
			//qDebug(QObject::tr("[INFO] analog_t::calculate: Failed to configure timer, set to maximum").toLocal8Bit());
			timer.configure.value = timer.maximum();
		}
		if (timer.frequency(true) * channelsCount(true) >= maxFrequency) {
			timer.setFrequency((maxFrequency - 1) / channelsCount(true));
			sizePerChannel = gridTotalTimeConfigure() * timer.frequency(true);
			if (sizePerChannel / grid.count.width() < grid.minimumPointsPerGrid) {
				qDebug(QObject::tr("[WARNING] analog_t::calculate: Reached maximum ADC speed").toLocal8Bit());
				return false;
			} else
				qDebug(QObject::tr("[INFO] analog_t::calculate: At maximum ADC speed").toLocal8Bit());
		}
		buffer.configure.sizePerChannel = sizePerChannel;
	}

	// Calculate trigger parameters
	if (trigger.configure.source != INVALID_ID) {
		qint32 level, position;
		const analog_t::channel_t *ch = findChannel(trigger.configure.source);
		if (!ch) {
			qDebug(QObject::tr("[WARNING] analog_t::calculate: Cannot find trigger channel %1").arg(trigger.configure.source).toLocal8Bit());
			return false;
		}
		//			      Screen				   Grid							     Voltage			     ADC
		level = (trigger.configure.dispLevel * (qreal)grid.count.height() / 2.f * ch->configure.scale.value() - ch->configure.displayOffset) / ch->reference * maximum();
		//					   Screen			       Grid			Time		       Count
		position = (trigger.configure.dispPosition + 1.f) * (qreal)grid.count.width() / 2.f * timebase.scale.value() * timer.frequency(true);
		if (level < 0 || level > (qint32)maximum())
			trigger.configure.source = INVALID_ID;
		trigger.configure.level = level;
		trigger.configure.position = position;
	}

	//qDebug() << scanModeConfigure() << channelsCountConfigure() << timer.frequencyConfigure() << timer.configure.value << timebase.configure.scale.value() << buffer.configure.sizePerChannel;
	return true;
}

void analog_t::update(void)
{
	timer.update();
	timebase.update();
	trigger.update();
	for (int i = 0; i < channels.count(); i++)
		channels[i].enabled = channels[i].configure.enabled;
	if (scanMode())
		buffer.sizePerChannel = gridTotalTime() * timer.frequency();
	else
		buffer.sizePerChannel = buffer.configure.sizePerChannel;
	buffer.reset();
	grid.pointsPerGrid = timer.frequency() * timebase.scale.value();
	//qDebug() << "[DEBUG] Analog update:" << scanMode() << channelsCount() << timer.frequency() << timer.value << timebase.scale.value() << buffer.sizePerChannel;
	for (int i = 0; i < channels.count(); i++)
		channels[i].update(buffer.sizePerChannel);
}

analog_t::channel_t::channel_t(void) : id(INVALID_ID), enabled(true)
{
	QColor clr;
	if (colourCount != sizeof(defaultColours) / sizeof(defaultColours[0]))
		clr = defaultColours[colourCount++];
	else
		clr = QColor(qrand() % 256, qrand() % 256, qrand() % 256);
	configure.colour = clr;
	configure.displayOffset = 0;
	configure.enabled = enabled;
}

void analog_t::channel_t::update(const int bufferSize)
{
	//enabled = configure.enabled;
	buffer.resize(bufferSize);
}

void analog_t::buffer_t::reset(void)
{
	position = 0;
	validSize = 0;
}

void analog_t::trigger_t::update(void)
{
	source = configure.source;
	level = configure.level;
	position = configure.position;
}

bool analog_t::trigger_t::updateRequired(void) const
{
	bool upd = source != configure.source;
	if (!upd && configure.source != INVALID_ID) {
		upd |= level != (quint32)configure.level;
		upd |= position != configure.position;
	}
	return upd;
}
