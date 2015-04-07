#include <cmath>
#include <QDebug>
#include <QObject>
#include "structures.h"
#include "conv.h"
#include "debug.h"

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
		return QString(units[unit]).arg(v, 0, 'f', 2);
	else
		return QString(units[unit]).arg(v);
}

quint32 analog_t::channelsCount(bool conf) const
{
	quint32 count = 0;
	for (int i = 0; i < channel.count(); i++)
		count += channelEnabled(i, conf);
	return count;
}

void analog_t::setChannelsEnabled(quint32 enabled)
{
	quint32 mask = 1;
	for (int i = 0; i < channel.count(); i++) {
		channel[i].updateMode(enabled & mask);
		mask <<= 1;
	}
}

quint32 analog_t::channelsEnabled(bool conf, bool chOnly) const
{
	quint32 enabled = 0, mask = 1;
	for (int i = 0; i < channel.count(); i++) {
		if (chOnly ? (conf ? channel.at(i).configure.enabled() : channel.at(i).enabled) : channelEnabled(i, conf))
			enabled |= mask;
		mask <<= 1;
	}
	return enabled;
}

int analog_t::screenXToIndex(qreal x, bool conf) const
{
	//	  Screen		       Grid		      Time		     Count
	return (x + 1.f) * (grid.count.width() / 2) * timebase.value(conf) * timer.frequency(conf);
}

qreal analog_t::indexToScreenX(int cnt, bool conf) const
{
	//	    Count		     Time		    Grid			   Screen
	return (qreal)cnt / timer.frequency(conf) / timebase.value(conf) / (grid.count.width() / 2) - 1.f;
}

bool analog_t::triggerValid(bool conf) const
{
	int level = conf ? trigger.level : trigger.configure.level;
	return level >= 0 && level <= (int)maximum();
}

bool analog_t::dataHandler(const data_t &data)
{
	if (data.id != id)
		return false;
	bool trig = trigger.enabled() && triggerValid();
	//qDebug(tr("[DEBUG] [%1] Analog data type: %2, count: %3").arg(QTime::currentTime().toString()).arg(data.type).arg(data.data.count()).toLocal8Bit());

	quint32 count = 0;
	switch (data.type) {
	case CTRL_DATA:
		if ((quint32)data.data.count() != channelsCount()) {
			//qDebug(tr("[DEBUG] Data size mismatch: %1/%2, ignored").arg(data.data.count()).arg(channelsCount()).toLocal8Bit());
			return false;
		}
		if (trig) {
			if (trigger.dataHandler(data.data) != trigger_t::state_t::Done)
				return false;
			for (int i = 0; i < channel.count(); i++)
				channel[i].buffer = trigger.state.buffer[i].buffer;
			buffer.validSize = buffer.sizePerChannel;
		} else {
			for (int i = 0; i < channel.count(); i++)
				if (channelEnabled(i, false))
					channel[i].buffer[buffer.position] = data.data.at(count++);
			buffer.position++;
			if (buffer.validSize < buffer.position)
				buffer.validSize = buffer.position;
			if (buffer.position == buffer.sizePerChannel)
				buffer.position = 0;
		}
		break;
	case CTRL_FRAME:
		if ((quint32)data.data.count() != channelsCount() * buffer.sizePerChannel) {
			//qDebug(tr("[DEBUG] Data size mismatch: %1/%2, ignored").arg(data.data.count()).arg(channelsCount()).toLocal8Bit());
			return false;
		}
		for (int i = 0; i < channel.count(); i++)
			channel[i].buffer.resize(buffer.sizePerChannel);
		for (quint32 pos = 0; pos < buffer.sizePerChannel; pos++)
			for (int i = 0; i < channel.count(); i++)
				if (channelEnabled(i, false))
					channel[i].buffer[pos] = data.data.at(count++);
		buffer.validSize = buffer.sizePerChannel;
		break;
	default:
		return false;
	}
	updateBufferInfo();
	return true;
}

bool analog_t::updateRequired() const
{
	bool upd = channelsEnabled(false, true) != channelsEnabled(true, true);
	upd |= buffer.updateRequired();
	upd |= timebase.updateRequired();
	upd |= trigger.sourceUpdateRequired();
	if (trigger.enabled()) {
		upd |= triggerValid(true) != triggerValid(false);
		upd |= triggerValid() && trigger.settingsUpdateRequired();
	}
	pr_debug(QObject::tr("%1").arg(upd), LV_PKG);
	return upd;
}

int analog_t::findChannelIndex(quint8 id) const
{
	if (id == INVALID_ID)
		return -1;
	for (int i = 0; i < channel.count(); i++)
		if (channel.at(i).id == id)
			return i;
	return -1;
}

analog_t::channel_t *analog_t::findChannel(quint8 id)
{
	int idx = findChannelIndex(id);
	return idx < 0 ? 0 : &channel[idx];
}

const analog_t::channel_t *analog_t::findChannel(quint8 id) const
{
	int idx = findChannelIndex(id);
	return idx < 0 ? 0 : &channel.at(idx);
}

void analog_t::init(void)
{
	//update();
}

bool analog_t::calculate(void)
{
	//pr_debug("Start", LV_TMP);
	if (!timer.setFrequency((float)grid.preferredPointsPerGrid / timebase.value(true))) {
		pr_debug(QObject::tr("Failed to configure timer, set to maximum"), LV_INFO);
		timer.configure.value = timer.maximum();
	}
	if (!scanMode(true)) {
		quint32 sizePerChannel = buffer.size / channelsCount(true);
		if (sizePerChannel / grid.count.width() < grid.minimumPointsPerGrid) {
			pr_debug(QObject::tr("Buffer too small"), LV_INFO);
			return false;
		}
		if (sizePerChannel / grid.count.width() > grid.preferredPointsPerGrid)
			sizePerChannel = grid.preferredPointsPerGrid * grid.count.width();
		if (!timer.setFrequency((float)sizePerChannel / (float)grid.count.width() / timebase.value(true))) {
			pr_debug(QObject::tr("Failed to configure timer, set to maximum"), LV_INFO);
			timer.configure.value = timer.maximum();
		}
		if (timer.frequency(true) * channelsCount(true) >= maxFrequency) {
			timer.setFrequency((maxFrequency - 1) / channelsCount(true));
			sizePerChannel = gridTotalTime(true) * timer.frequency(true);
			if (sizePerChannel / grid.count.width() < grid.minimumPointsPerGrid) {
				pr_debug(QObject::tr("Reached maximum ADC speed"), LV_INFO);
				return false;
			} else
				pr_debug(QObject::tr("At maximum ADC speed"), LV_INFO);
		}
		buffer.configure.sizePerChannel = sizePerChannel;
	}

	// Calculate trigger parameters
	if (trigger.configure.source != INVALID_ID) {
		trigger.configure.level = findChannel(trigger.channel(true))->screenYToADC(trigger.configure.dispLevel);
		trigger.configure.position = screenXToIndex(trigger.configure.dispPosition, true);
	}

	pr_debug("Results:", LV_INFO) << scanMode(true) << channelsCount(true) \
				      << timer.frequency(true) << timer.configure.value \
				      << timebase.value(true) << buffer.configure.sizePerChannel;
	return true;
}

void analog_t::update(void)
{
	//pr_debug("Start", LV_TMP);
	timer.update();
	timebase.update();
	trigger.update();
	for (int i = 0; i < channel.count(); i++)
		channel[i].enabled = channel[i].configure.enabled();
	if (scanMode())
		buffer.sizePerChannel = gridTotalTime() * timer.frequency();
	else
		buffer.sizePerChannel = buffer.configure.sizePerChannel;
	buffer.reset();
	grid.pointsPerGrid = timer.frequency() * timebase.value();
	for (int i = 0; i < channel.count(); i++) {
		channel[i].update(buffer.sizePerChannel);
		trigger.state.buffer[i].enabled = channelEnabled(i);
	}
	trigger.state.bufferIndex = triggerChannelIndex();
	trigger.resetBuffer(buffer.sizePerChannel);
	pr_debug("Updated:", LV_INFO) << scanMode() << channelsCount() \
				      << timer.frequency() << timer.value \
				      << timebase.scale.value() << buffer.sizePerChannel;
}

void analog_t::updateBufferInfo(void)
{
	for (int idx = 0; idx < channel.count(); idx++) {
		if (!channelEnabled(idx))
			continue;
		channel_t &ch = channel[idx];
		qint32 min = maximum(), max = 0;
		quint64 sum = 0;
		for (quint32 i = 0; i < buffer.validSize; i++) {
			qint32 data = ch.buffer.at(i);
			sum += data;
			if (data < min)
				min = data;
			if (data > max)
				max = data;
		}
		ch.bufferInfo.mean = buffer.validSize ? (qreal)sum / buffer.validSize : (maximum() / 2);
		ch.bufferInfo.min = min;
		ch.bufferInfo.max = max;
	}
}

analog_t::channel_t::channel_t(void) : id(INVALID_ID), reference(0), offset(0), enabled(true), analog(0)
{
	QColor clr;
	if (colourCount != sizeof(defaultColours) / sizeof(defaultColours[0]))
		clr = defaultColours[colourCount++];
	else
		clr = QColor(qrand() % 256, qrand() % 256, qrand() % 256);
	configure.colour = clr;
	resetBufferInfo();
}

void analog_t::channel_t::resetBufferInfo(void)
{
	bufferInfo.mean = analog ? analog->maximum() : 0;
	bufferInfo.min = 0;
	bufferInfo.max = analog ? analog->maximum() : 0;
}

void analog_t::channel_t::update(const int bufferSize)
{
	//enabled = configure.enabled;
	buffer.resize(bufferSize);
	resetBufferInfo();
}

void analog_t::channel_t::updateMode(const bool e)
{
	enabled = e;
	if (configure.enabled() && !e)
		configure.mode = channel_t::configure_t::Off;
	else if (!configure.enabled() && e)
		configure.mode = channel_t::configure_t::DC;
}

qreal analog_t::channel_t::adcToVoltage(const qint32 adc) const
{
	return ((qreal)adc - (configure.mode == configure_t::AC ? bufferInfo.mean : 0)) / analog->maximum() * reference + totalOffset();
}

qint32 analog_t::channel_t::voltageToADC(const qreal voltage) const
{
	return voltage / reference * analog->maximum() + (configure.mode == configure_t::AC ? bufferInfo.mean : 0);
}

qreal analog_t::channel_t::voltageToScreenY(const qreal voltage) const
{
	//     Voltage			    Grid			      Screen
	return voltage / configure.scale.value() / (analog->grid.count.height() / 2);
}

qreal analog_t::channel_t::screenYToVoltage(const qreal y) const
{
	//Screen				Grid				       Voltage
	return y * (analog->grid.count.height() / 2) * configure.scale.value() - totalOffset();
}

void analog_t::buffer_t::reset(void)
{
	position = 0;
	validSize = 0;
}

void analog_t::trigger_t::update(void)
{
	source = configure.source;
	edge = configure.edge;
	level = configure.level;
	position = configure.position;
}

void analog_t::trigger_t::reset(void)
{
	configure.source = source;
	configure.level = level;
	configure.position = position;
}

void analog_t::trigger_t::resetBuffer(const int size)
{
	//qDebug() << "[DEBUG] trigger_t::resetBuffer: " << size << position << (position > size ? position + 1 : size);
	state.bufferSize = size;
	state.position = position <= 0 ? position : 0;
	state.status = position <= 0 ? state_t::Waiting : state_t::Pre;
	for (int i = 0; i < state.buffer.count(); i++)
		state.buffer[i].buffer.resize(position > size ? position + 1 : size);
}

bool analog_t::trigger_t::beforeEdge(void)
{
	return (edge == Rising && state.currentData() <= level) || (edge == Falling && state.currentData() >= level);
}

analog_t::trigger_t::state_t::Status analog_t::trigger_t::dataHandler(const QVector<quint32> &data)
{
	// Before saving
	switch (state.status) {
	case state_t::Pre:
		if (state.position >= position) {
			if (beforeEdge())
				state.status = state_t::Waiting;
			state.shiftBuffer();
		} else
			state.position++;
		break;
	case state_t::Waiting:
		state.shiftBuffer();
		break;
	case state_t::Post:
		break;
	case state_t::Done:
		// Reset
		state.position = position <= 0 ? position : 0;
		state.status = state_t::Pre;
		break;
	};

	// Save into buffer
	//qDebug(QObject::tr("[DEBUG] trigger_t::dataHandler: %1, %2, %3").arg(state.position).arg(state.bufferSize).arg((int)state.status).toLocal8Bit());
	quint32 count = 0;
	for (int i = 0; i < state.buffer.count(); i++)
		if (state.buffer[i].enabled)
			state.buffer[i].buffer[state.position <= 0 ? 0 : state.position] = data.at(count++);

	// After save
	switch (state.status) {
	case state_t::Pre:
		break;
	case state_t::Waiting:
		if (!beforeEdge()) {
			state.position++;
			state.status = state_t::Post;
		}
		break;
	case state_t::Post:
		state.position++;
		break;
	case state_t::Done:
		break;
	};

	// Done checking
	if (state.status == state_t::Post && state.position >= (qint32)state.bufferSize) {
		state.status = state_t::Done;
		// Reset
		state.position = position <= 0 ? position : 0;
	}
	return state.status;
}

bool analog_t::trigger_t::settingsUpdateRequired(void) const
{
	bool upd = sourceUpdateRequired();
	if (!upd && enabled(true)) {
		upd |= edge != configure.edge;
		upd |= level != (quint32)configure.level;
		upd |= position != configure.position;
	}
	return upd;
}

void analog_t::trigger_t::state_t::shiftBuffer(void)
{
	for (int i = 0; i < buffer.count(); i++)
		if (buffer[i].enabled) {
			buffer[i].buffer.pop_front();
			buffer[i].buffer.push_back(0);
		}
}

quint32 analog_t::trigger_t::state_t::currentData(void)
{
	return buffer.at(bufferIndex).buffer.at(position <= 0 ? 0 : position);
}
