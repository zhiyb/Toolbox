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

quint32 analog_t::channelsCount(void) const
{
	quint32 count = 0;
	for (int i = 0; i < channels.count(); i++)
		count += channels.at(i).enabled;
	return count;
}

quint32 analog_t::channelsCountConfigure() const
{
	quint32 count = 0;
	for (int i = 0; i < channels.count(); i++)
		count += channels.at(i).configure.enabled;
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

quint32 analog_t::channelsEnabledConfigure(void) const
{
	quint32 enabled = 0, mask = 1;
	for (int i = 0; i < channels.count(); i++) {
		if (channels.at(i).configure.enabled)
			enabled |= mask;
		mask <<= 1;
	}
	return enabled;
}

void analog_t::init(void)
{
	//update();
}

bool analog_t::calculate(void)
{
	qDebug() << "[DEBUG] Analog calculate";
	if (!timer.setFrequency((float)grid.preferredPointsPerGrid / timebase.configure.scale.value())) {
		qDebug(QObject::tr("Analog calculate: Failed to configure timer, set to maximum").toLocal8Bit());
		timer.configure.value = timer.maximum();
	}
	if (!scanModeConfigure()) {
		quint32 sizePerChannel = buffer.size / channelsCountConfigure();
		if (sizePerChannel / grid.count.width() < grid.minimumPointsPerGrid) {
			qDebug(QObject::tr("Analog calculate: Buffer too small").toLocal8Bit());
			return false;
		}
		if (sizePerChannel / grid.count.width() > grid.preferredPointsPerGrid)
			sizePerChannel = grid.preferredPointsPerGrid * grid.count.width();
		if (!timer.setFrequency((float)sizePerChannel / (float)grid.count.width() / timebase.configure.scale.value())) {
			qDebug(QObject::tr("Analog calculate: Failed to configure timer, set to maximum").toLocal8Bit());
			timer.configure.value = timer.maximum();
		}
		if (timer.frequencyConfigure() * channelsCountConfigure() >= maxFrequency) {
			timer.setFrequency((maxFrequency - 1) / channelsCountConfigure());
			sizePerChannel = gridTotalTimeConfigure() * timer.frequencyConfigure();
			if (sizePerChannel / grid.count.width() < grid.minimumPointsPerGrid) {
				qDebug(QObject::tr("Analog calculate: Reached maximum ADC speed").toLocal8Bit());
				return false;
			} else
				qDebug(QObject::tr("Analog calculate: At maximum ADC speed").toLocal8Bit());
		}
		buffer.configure.sizePerChannel = sizePerChannel;
	}
	qDebug() << scanModeConfigure() << channelsCountConfigure() << timer.frequencyConfigure() << timer.configure.value << timebase.configure.scale.value() << buffer.configure.sizePerChannel;
	return true;
}

void analog_t::update(void)
{
	qDebug() << "[DEBUG] Analog update";
	timer.update();
	timebase.update();
	for (int i = 0; i < channels.count(); i++)
		channels[i].enabled = channels[i].configure.enabled;
	if (scanMode())
		buffer.sizePerChannel = gridTotalTime() * timer.frequency();
	else
		buffer.sizePerChannel = buffer.configure.sizePerChannel;
	grid.pointsPerGrid = timer.frequency() * timebase.scale.value();
	buffer.position = 0;
	buffer.validSize = 0;
	qDebug() << scanMode() << channelsCount() << timer.frequency() << timer.value << timebase.scale.value() << buffer.sizePerChannel;
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
	configure.colour = conv::colorToVector4D(clr);
	configure.displayOffset = 0;
	configure.enabled = enabled;
}

void analog_t::channel_t::update(const int bufferSize)
{
	//enabled = configure.enabled;
	buffer.resize(bufferSize);
}
