#include <QThread>
#include <QDebug>
#include <QString>
#include <QList>
#include <QHostAddress>
#include <instructions.h>
#include "connectionselection.h"
#include "connection.h"
#include "conv.h"
#include "debug.h"

#define REPORT_INTERVAL		3

Connection::Connection(QObject *parent) : QObject(parent) {}

Connection::~Connection(void)
{
	while (!infos.isEmpty()) {
		delete infos.first();
		infos.removeFirst();
	}
}

bool Connection::init(void)
{
	QHostAddress host;
	ConnectionSelection *s = new ConnectionSelection;
	if (s->exec() != QDialog::Accepted)
		return false;
	type = s->type;
	switch (type) {
#ifdef ENABLE_NETWORK
	case ConnectionSelection::Network:
		pr_debug(tr("Connection::Network: %1, %2").arg(s->host).arg(s->port), LV_MSG);
		con = new QTcpSocket(this);
		host = QHostAddress(s->host);
		if (!host.isNull())
			tcpSocket()->connectToHost(host, s->port);
		else
			tcpSocket()->connectToHost(s->host, s->port);
		if (!tcpSocket()->waitForConnected()) {
			emit error("Network error: " + tcpSocket()->errorString());
			return false;
		}
		break;
#endif
#ifdef ENABLE_SERIALPORT
	case ConnectionSelection::SerialPort:
		pr_debug(tr("Connection::SerialPort: %1, %2").arg(s->serialPort).arg(s->serialSpeed), LV_MSG);
		con = new QSerialPort(s->serialInfo, this);
		//serialPort()->setPortName(s->serialPort);
		serialPort()->setBaudRate(s->serialSpeed);
		serialPort()->open(QIODevice::ReadWrite);
		if (!serialPort()->isOpen()) {
			QMessageBox::critical(0, tr("Serial port error"), tr("Cannot open selected serial port!"));
			return false;
		}
		break;
#endif
	default:
		return false;
	}
	return reset();
}

void Connection::waitForWrite(int msec)
{
	while (con->bytesToWrite() > 0)
		con->waitForBytesWritten(msec);
}

void Connection::waitForRead(const qint64 size, int msec)
{
	qint64 avail = 0;
	while (true) {
		if ((avail = con->bytesAvailable()) >= size)
			return;
		con->waitForReadyRead(msec);
		if (avail == con->bytesAvailable())
			return;
	}
}

bool Connection::waitForReadAll(int count, int msec)
{
	while ((count == -1 || count--) && con->waitForReadyRead(msec));
	return count != 0;
}

void Connection::report(void)
{
	QTime now = QTime::currentTime();
	int sec = counter.prev.secsTo(now);
	int msec = counter.prev.msecsTo(now);

	// Data transmission rate
	if (sec) {
		emit information("RATE_TX", tr("Tx: %1 bytes/s (%2 packages/s)").arg((float)counter.tx / msec * 1000).arg((float)counter.txPackage / msec * 1000));
		emit information("RATE_RX", tr("Rx: %1 bytes/s (%2 packages/s)").arg((float)counter.rx / msec * 1000).arg((float)counter.rxPackage / msec * 1000));
		counter.reset(now);
	}
}

void Connection::report(info_t *info)
{
	switch (info->type()) {
	case CMD_ANALOG: {
		analog_t *analog = (analog_t *)info;
		emit information(QString("ANALOG%1_INFO").arg(analog->id), tr("%1: %2 S/s, %3 S/f").arg(analog->name).arg(analog->timer.frequency()).arg(analog->buffer.sizePerChannel));
		emit information(QString("ANALOG%1_HWINFO").arg(analog->id), tr("%1 hw: freq(%2), timer(%3)").arg(analog->name).arg(analog->timer.frequency() * analog->channelsCount()).arg(analog->timer.value));
		break;
	}
	default:
		emit error(tr("reportData not supported for type: %1").arg(info->type()));
	}
}

void Connection::writeChar(const char c)
{
	con->write(&c, 1);
	pr_debug(tr("%1(%2)").arg((quint8)c).arg(c), LV_BYTE);
	counter.tx++;
}

void Connection::writeRepeatedChar(const char c, const qint64 size)
{
	for (qint64 i = 0; i != size; i++)
		writeChar(c);
}

void Connection::write(QByteArray &data)
{
	con->write(data);
	counter.tx += data.count();
}

void Connection::writeValue(const quint32 value, const quint32 bytes)
{
	pr_debug(tr("%1/%2").arg(value).arg(bytes), LV_BYTE);
	con->write((char *)&value, bytes);
	counter.tx += bytes;
}

int Connection::readChar(int msec, bool debug)
{
	char c;
	waitForRead(1, msec);
	if (con->bytesAvailable() < 1) {
		//pr_debug(tr("timed out (%1ms)").arg(msec), LV_BYTE);
		return -1;
	}
	con->read(&c, 1);
	if (debug)
		pr_debug(tr("%1(%2)").arg((quint8)c).arg(c), LV_BYTE);
	counter.rx++;
	return (quint8)c;
}

quint32 Connection::readValue(const quint32 bytes, int msec)
{
	quint32 value = 0;
	waitForRead(bytes, msec);
	if (con->bytesAvailable() < bytes)
		return -1;
	con->read((char *)&value, bytes);
	counter.rx += bytes;
	pr_debug(tr("%1/%2").arg(value).arg(bytes), LV_BYTE);
	return value;
}

QString Connection::readString(int msec)
{
	int c;
	QString str;
	forever {
		c = readChar(msec, false);
		if (c == '\0' || c == -1)
			break;
		str.append(c);
	}
	pr_debug(str, LV_BYTE);
	return str;
}

void Connection::writeMessage(message_t &msg)
{
	pr_debug(tr("Sequence: %1, command: %2, ID: %3").arg(msg.sequence).arg(msg.command).arg((quint32)msg.id), LV_PKG);
send:
	writeChar(msg.command);
	waitForWrite();
	QTime t(QTime::currentTime());
	char c = 0;
	while (t.msecsTo(QTime::currentTime()) < 1000 && (c = readData()) == 0);
	if (c == -1 || c == 0) {
		pr_debug(tr("Quick resync"), LV_MSG);
		quickResync();
		goto send;
	} else if (c != CMD_ACK) {
		emit error(tr("Message %1: No ACK received for command '%2': %3(%4)").arg(msg.sequence).arg(msg.command).arg((quint8)c).arg(c));
		return;
	}
	if (msg.id != INVALID_ID)
		writeChar(msg.id);
	while (msg.settings.count()) {
		struct message_t::set_t set = msg.settings.dequeue();
		pr_debug(tr("  Settings ID: %1, bytes: %2, value: 0x%3").arg((quint32)set.id).arg(set.bytes).arg(set.value, 2, 16, QChar('0')), LV_PKG);
		writeChar(set.id);
		if (set.id == INVALID_ID)
			break;
		writeValue(set.value, set.bytes);
	}
	this->counter.txPackage++;
	if (msg.update.id != INVALID_ID) {
		pr_debug(tr("Message sent, update %1").arg(msg.update.id), LV_PKG);
		info_t *info = (analog_t *)findInfo(CMD_ANALOG, msg.update.id);
		if (!info)
			emit error(tr("No matching info data ID: %1").arg(msg.update.id));
		switch (info->type()) {
		case CMD_ANALOG:
			((analog_t *)info)->update();
			break;
		default:
			emit error(tr("update not supported for type: %1").arg(info->type()));
		}
		report(info);
	}
	emit messageSent(msg.sequence);
}

bool Connection::reset(void)
{
	pr_debug("Reset", LV_INFO);
	//resync();
	writeRepeatedChar(CMD_RESET, 16);
	counter.tx += 16;
	waitForWrite();
	waitForReadAll(3);
	QByteArray data = con->readAll();
	counter.rx += data.size();
	if (!data.endsWith(CMD_ACK)) {
		emit error(tr("Reset error, no acknowledge received: ") + data);
		return false;
	}
	return true;
}

void Connection::pushInfo(info_t *s)
{
	for (int i = 0; i < infos.count(); i++) {
		info_t *info = infos[i];
		if (info->type() == s->type() && info->id == s->id) {
			infos.replace(i, s);
			delete info;
			return;
		}
	}
	infos.append(s);
}

QVector<info_t *> Connection::findInfos(const quint8 type)
{
	QVector<info_t *> infos;
	for (int i = 0; i != this->infos.count(); i++) {
		info_t *info = this->infos[i];
		//pr_debug(tr("Matching: %1, %2").arg(type).arg(info->type()), LV_PKG);
		if (info->type() == type)
			infos.append(info);
	}
	return infos;
}

info_t *Connection::findInfo(const quint8 type, const quint8 id)
{
	for (int i = 0; i != infos.count(); i++) {
		info_t *info = infos[i];
		//pr_debug(tr("Matching: %1(%2), %3(%4)").arg(type).arg((quint32)id).arg(info->type()).arg((quint32)info->id), LV_PKG);
		if (info->type() == type && info->id == id)
			return info;
	}
	return 0;
}

device_t Connection::readDeviceInfo(void)
{
	device_t device;
	device.version = readValue(4);
	device.name = readString();
	if (device.version != FW_VERSION)
		emit error(tr("Firmware version mismatch: %1/%2").arg(device.version).arg(FW_VERSION));
	return device;
}

controller_t* Connection::readController(void)
{
	controller_t *ctrl = new controller_t;
	ctrl->id = readChar();
	if (ctrl->id == INVALID_ID) {
		emit error(tr("Invalid controller ID"));
		delete ctrl;
		return 0;
	}
	ctrl->name = readString();
	forever {
		struct controller_t::set_t set;
		set.id = readChar();
		if (set.id == INVALID_ID)
			break;
		set.type = readChar();
		switch (set.type & ~CTRL_READONLY) {
		case CTRL_BYTE1:
		case CTRL_BYTE2:
		case CTRL_BYTE3:
		case CTRL_BYTE4:
			if (!set.readOnly()) {
				set.min = readValue(set.bytes());
				set.max = readValue(set.bytes());
			}
			set.value = readValue(set.bytes());
			set.name = readString();
			break;
		}
		ctrl->controls.append(set);
	}
	pushInfo(ctrl);
	return ctrl;
}

hwtimer_t Connection::readTimer(void)
{
	hwtimer_t timer;
	if (readChar() != CMD_TIMER) {
		emit error(tr("Invalid timer command"));
		return timer;
	}
	timer.id = readChar();
	if (timer.id == INVALID_ID) {
		emit error(tr("Invalid timer ID"));
		return timer;
	}
	timer.resolution = readChar();
	timer.clockFrequency = readValue(4);
	return timer;
}

analog_t *Connection::readAnalog(void)
{
	analog_t *analog = new analog_t;
	analog->id = readChar();
	if (analog->id == INVALID_ID) {
		emit error(tr("Invalid analog ID"));
		delete analog;
		return 0;
	}
	analog->name = readString();
	analog->resolution = readChar();
	analog->scanFrequency = readValue(4);
	analog->maxFrequency = readValue(4);
	quint8 channels = readChar();
	for (quint8 i = 0; i < channels; i++) {
		analog_t::channel_t channel;
		channel.id = readChar();
		channel.name = readString();
		channel.reference = conv::rawUInt32ToFloat(readValue(4));
		channel.offset = conv::rawUInt32ToFloat(readValue(4));
		channel.analog = analog;
		analog->channel.append(channel);
	}
	analog->trigger.state.buffer.resize(analog->channel.size());
	analog->setChannelsEnabled(readValue(analog->channelsBytes()));
	analog->buffer.size = readValue(4);
	analog->timer = readTimer();
	pushInfo(analog);
	pr_debug("", LV_INFO) << analog;
	return analog;
}

analog_t::data_t Connection::readAnalogData(void)
{
	analog_t::data_t data;
	data.id = readChar();
	if (data.id == INVALID_ID) {
		emit error(tr("Invalid analog data ID"));
		return data;
	}
	analog_t *analog = (analog_t *)findInfo(CMD_ANALOG, data.id);
	if (!analog) {
		emit error(tr("No matching analog data ID: %1").arg(data.id));
		data.id = INVALID_ID;
		return data;
	}
	data.type = readChar();
	quint32 count = analog->channelsCount();
	data.data.resize(count);
	switch (data.type) {
	case CTRL_FRAME:
		data.data.resize(analog->buffer.sizePerChannel * count);
		count = readValue(4);
		for (quint32 i = count; i < analog->buffer.sizePerChannel * analog->channelsCount(); i++)
			data.data[i] = readValue(analog->bytes());
	case CTRL_DATA:
		for (quint32 i = 0; i < count; i++)
			data.data[i] = readValue(analog->bytes());
		break;
	}
	pr_debug(tr("%1").arg(data.data.size()), LV_BYTE);
	return data;
}

char Connection::readData(int msec)
{
	int c = readChar(msec);
	//pr_debug(tr("%1(%2)").arg(c).arg((char)c), LV_BYTE);
	switch (c) {
	case CMD_ACK:
		return CMD_ACK;
	case CMD_INFO:
		emit device(readDeviceInfo());
		break;
	case CMD_ANALOG:
		emit info(readAnalog());
		break;
	case CMD_CONTROLLER:
		emit info(readController());
		break;
	case CMD_ANALOGDATA:
		emit analogData(readAnalogData());
		break;
	case 'V':
		pr_debug(tr("%1: Received debug V, %2").arg(QTime::currentTime().toString()).arg((quint8)readChar(msec)), LV_MSG);
		return -1;
	case -1:
		if (msec > 100)
			pr_debug(tr("Timed out"), LV_PKG);
		return -1;
	default:
		pr_warning(tr("Unknown head: %1(%2)").arg(c).arg((char)c));
		return c;
	}
	counter.rxPackage++;
	return 0;
}

void Connection::start(void)
{
	timer.report = startTimer(REPORT_INTERVAL * 1000);
	timer.update = startTimer(0);
	report();
}

void Connection::enqueue(const message_t &msg)
{
	queueLock.lock();
	if (!queue.isEmpty() && queue.last().similar(msg))
		queue.removeLast();
	queue.enqueue(msg);
	queueLock.unlock();
}

void Connection::timerEvent(QTimerEvent *e)
{
	if (e->timerId() == timer.report) {
		report();
		return;
	}
	if (!queue.isEmpty() && queueLock.tryLock()) {
		bool send = false;
		message_t msg;
		if (!queue.isEmpty()) {
			msg = queue.dequeue();
			send = true;
		}
		queueLock.unlock();
		if (send)
			writeMessage(msg);
	}
	readData(10);
}

void Connection::counter_t::reset(const QTime &now)
{
	prev = now;
	txPackage = 0;
	rxPackage = 0;
	tx = 0;
	rx = 0;
}
