#include <QThread>
#include <QMessageBox>
#include <QDebug>
#include <QString>
#include <QHostAddress>
#include <QSerialPortInfo>
#include <QList>
#include <QInputDialog>
#include <QProgressDialog>
#include <instructions.h>
#include "connection.h"
#include "conv.h"

#define DEFAULT_NETWORK_HOST	"192.168.0.36"
//#define DEFAULT_NETWORK_HOST	"192.168.6.48"
#define DEFAULT_NETWORK_PORT	1111
#define DEFAULT_SERIAL_PORT	"COM1"
#define DEFAULT_SERIAL_SPEED	115200
#define DATA_RATE_AVERAGE	10	// Need to be divisible by 60

ConnectionSelection::ConnectionSelection(QWidget *parent) : QDialog(parent)
{
	type = Connection::Network;
	host = DEFAULT_NETWORK_HOST;
	port = DEFAULT_NETWORK_PORT;
	serialPort = DEFAULT_SERIAL_PORT;
	serialSpeed = DEFAULT_SERIAL_SPEED;

	QVBoxLayout *vLayout = new QVBoxLayout(this);
	QHBoxLayout *hTypeLayout = new QHBoxLayout, *hSettingsLayout = new QHBoxLayout, *hButtonLayout = new QHBoxLayout;
	vLayout->addLayout(hTypeLayout);
	hTypeLayout->addWidget(rbTypes[Connection::Network] = new QRadioButton(tr("&Network")));
	rbTypes[Connection::Network]->setChecked(true);
	hTypeLayout->addWidget(rbTypes[Connection::SerialPort] = new QRadioButton(tr("&Serial Port")));
	vLayout->addLayout(hSettingsLayout);
	hSettingsLayout->addWidget(lHost = new QLabel);
	hSettingsLayout->addWidget(leHost = new QLineEdit, 2);
	hSettingsLayout->addWidget(pbScan = new QPushButton(tr("Sc&an")));
	pbScan->setVisible(false);
	hSettingsLayout->addWidget(lPort = new QLabel);
	hSettingsLayout->addWidget(lePort = new QLineEdit, 1);
	vLayout->addLayout(hButtonLayout);
	hButtonLayout->addWidget(pbOpen = new QPushButton(tr("&Open")));
	pbOpen->setDefault(true);
	hButtonLayout->addWidget(pbCancel = new QPushButton(tr("&Cancel")));
	setWindowTitle("Connection settings");
	setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);

	for (unsigned int i = 0; i < (sizeof(rbTypes) / sizeof(rbTypes[0])); i++)
		connect(rbTypes[i], SIGNAL(clicked()), this, SLOT(typeUpdate()));
	connect(pbScan, SIGNAL(clicked()), this, SLOT(scan()));
	connect(pbOpen, SIGNAL(clicked()), this, SLOT(accept()));
	connect(pbCancel, SIGNAL(clicked()), this, SLOT(reject()));

	typeUpdate();
}

void ConnectionSelection::scan(void)
{
	QList<QSerialPortInfo> ports = QSerialPortInfo::availablePorts();
	QStringList names;
	for (int i = 0; i < ports.count(); i++)
		names.append(ports.at(i).description() + " (" + ports.at(i).portName() + ")");
	if (names.count() == 0) {
		QMessageBox::information(this, tr("Serial port selection"), tr("No serial port available!"));
		return;
	}
	QString sel = QInputDialog::getItem(this, tr("Serial port selection"), tr("Please select a serial port hardware from the list:"), names, 0, false);
	if (!sel.isEmpty())
		for (int i = 0; i < ports.count(); i++)
			if (names.at(i) == sel) {
				leHost->setText(ports.at(i).portName());
				return;
			}
}

void ConnectionSelection::normalize(void)
{
	bool ok;
	QString tmpHost = leHost->text();
	int tmpPort = lePort->text().toInt(&ok);
	switch (type) {
	case Connection::Network:
		if (tmpHost.isEmpty())
			tmpHost = DEFAULT_NETWORK_HOST;
		if (!ok || tmpPort <= 0)
			tmpPort = DEFAULT_NETWORK_PORT;
		host = tmpHost;
		port = tmpPort;
		break;
	case Connection::SerialPort:
		if (tmpHost.isEmpty())
			tmpHost = DEFAULT_SERIAL_PORT;
		if (!ok || tmpPort <= 0)
			tmpPort = DEFAULT_SERIAL_SPEED;
		serialPort = tmpHost;
		serialSpeed = tmpPort;
		break;
	}
}

void ConnectionSelection::typeUpdate(void)
{
	normalize();
	if (rbTypes[Connection::Network]->isChecked()) {
		type = Connection::Network;
		lHost->setText(tr("Host:"));
		leHost->setText(host);
		lPort->setText(tr("Port:"));
		lePort->setText(QString::number(port));
		pbScan->setVisible(false);
	} else if (rbTypes[Connection::SerialPort]->isChecked()) {
		type = Connection::SerialPort;
		lHost->setText(tr("Serial port:"));
		leHost->setText(serialPort);
		lPort->setText(tr("Speed:"));
		lePort->setText(QString::number(serialSpeed));
		pbScan->setVisible(true);
	}
}

void ConnectionSelection::accept(void)
{
	normalize();
	if (!leHost->text().isEmpty())
		QDialog::accept();
}

// ********************************************************************************

Connection::Connection(QObject *parent) :
	QObject(parent)
{
	//queueLock = false;
	dataCount = 0;
	exit = false;
}

Connection::~Connection(void)
{
	while (!infos.isEmpty()) {
		delete infos.first();
		infos.removeFirst();
	}
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
	//QByteArray data = con->readAll();
	while ((count == -1 || count--) && con->waitForReadyRead(msec));
		//data.append(con->readAll());
	return count != 0;
	//return data;
}

void Connection::writeChar(const char c)
{
	con->write(&c, 1);
}

void Connection::writeRepeatedChar(const char c, const qint64 size)
{
	for (qint64 i = 0; i != size; i++)
		writeChar(c);
}

/*void Connection::pause(void)
{
}*/

void Connection::reset(void)
{
	//pause();
	//qDebug() << "Connection::reset";
	do {
		resync();
		//writeRepeatedChar(INVALID_ID, /*PKG_SIZE*/32);
		writeRepeatedChar(CMD_RESET, /*PKG_SIZE*/32);
		waitForWrite();
	} while (!waitForReadAll(3));
	QByteArray data = con->readAll();//waitForReadAll(3);
	if (!data.endsWith(CMD_ACK)) {
		//qDebug() << data;
		emit error(tr("Reset error, no acknowledge received: ") + data);
		return;
	}
	//sendChar(CMD_ACK);
	//waitForWrite();
}

void Connection::resync()
{
	writeRepeatedChar(INVALID_ID, /*PKG_SIZE*/32);
}

void Connection::requestInfo(void)
{
	struct message_t msg;
	msg.command = CMD_INFO;
	enqueue(msg);
}

bool Connection::init(void)
{
	QHostAddress host;
	ConnectionSelection *s = new ConnectionSelection;
	if (s->exec() != QDialog::Accepted)
		return false;
	type = s->type;
	switch (type) {
	case Network:
		qDebug() << "Connection::Network" << s->host << s->port;
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
	case SerialPort:
		qDebug() << "Connection::SerialPort" << s->serialPort << s->serialSpeed;
		con = new QSerialPort(this);
		serialPort()->setPortName(s->serialPort);
		serialPort()->setBaudRate(s->serialSpeed);
		serialPort()->open(QIODevice::ReadWrite);
		if (!serialPort()->isOpen()) {
			QMessageBox::critical(0, tr("Serial port error"), tr("Cannot open selected serial port!"));
			return false;
		}
		break;
	default:
		return false;
	}
	reset();
	return true;
}

void Connection::write(QByteArray &data)
{
	/*if (data.size() >= PKG_SIZE)
		return;*/
	con->write(data);
}

void Connection::writeValue(const quint32 value, const quint32 bytes)
{
	con->write((char *)&value, bytes);
}

int Connection::readChar(int msec)
{
	char c;
	waitForRead(1, msec);
	if (con->bytesAvailable() < 1)
		return -1;
	con->read(&c, 1);
	return c;
}

quint32 Connection::readValue(const quint32 bytes, int msec)
{
	quint32 value = 0;
	waitForRead(bytes, msec);
	if (con->bytesAvailable() < bytes)
		return -1;
	con->read((char *)&value, bytes);
	//qDebug() << "Connection::readValue" << bytes << msec << value;
	return value;
}

QString Connection::readString(int msec)
{
	char c;
	QString str;
	forever {
		c = readChar(msec);
		if (c == '\0' || c == -1)
			break;
		str.append(c);
	}
	return str;
}

void Connection::writeMessage(message_t &msg)
{
	//qDebug() << "writeMessage";
	//qDebug(tr("Sending message, sequence: %1, command: %2, id: %3").arg(msg.sequence).arg(msg.command).arg((quint32)msg.id).toLocal8Bit());
send:
	writeChar(msg.command);
	waitForWrite();
	int count = 10;
	char c;
	while ((count == -1 || count--) && (c = readData()) == 0);
	if (c == -1 || c == 0) {
		resync();
		goto send;
	}
	if (c != CMD_ACK) {
		emit error(QString(tr("No ACK received for command '%1': %2")).arg(msg.command).arg(c));
		return;
	}
	if (msg.id != INVALID_ID)
		writeChar(msg.id);
	while (msg.settings.count()) {
		struct message_t::set_t set = msg.settings.dequeue();
		//qDebug(tr("  Settings ID: %1, bytes: %2, value: %3").arg((quint32)set.id).arg(set.bytes).arg(set.value).toLocal8Bit());
		writeChar(set.id);
		if (set.id == INVALID_ID)
			break;
		writeValue(set.value, set.bytes);
	}
	emit messageSent(msg.sequence);
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

info_t *Connection::findInfo(const quint8 type, const quint8 id)
{
	for (int i = 0; i < infos.count(); i++) {
		info_t *info = infos[i];
		//qDebug(tr("Matching: %1(%2), %3(%4)").arg(type).arg((quint32)id).arg(info->type()).arg((quint32)info->id).toLocal8Bit());
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
		exit = true;
	return device;
}

controller_t* Connection::readController(void)
{
	//qDebug() << "Connection::readController";
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
		/*case CMD_END:
			return ctrl;*/
		}
		ctrl->controls.append(set);
	}
	pushInfo(ctrl);
	return ctrl;
}

timer_t Connection::readTimer(void)
{
	timer_t timer;
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
	quint8 channels = readChar();
	for (quint8 i = 0; i < channels; i++) {
		analog_t::channel_t channel;
		channel.id = readChar();
		channel.name = readString();
		channel.reference = conv::rawUInt32ToFloat(readValue(4));
		channel.offset = conv::rawUInt32ToFloat(readValue(4));
		analog->channels.append(channel);
	}
	analog->setChannelsEnabled(readValue(analog->channelsBytes()));
	analog->timer = readTimer();
	pushInfo(analog);
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
	int count = analog->channelsCount();
	switch (data.type) {
	case CTRL_DATA:
		for (int i = 0; i < count; i++)
			data.data.append(readValue(analog->bytes()));
		break;
	}
	return data;
}

char Connection::readData(int msec)
{
	quint8 c = readChar(msec);
	//qDebug() << "Connection::readData:" << c;
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
		qDebug(tr("%1: Received debug V").arg(QTime::currentTime().toString()).toLocal8Bit());
	case ((quint8)-1):
		return -1;
	default:
		qDebug(tr("Connection::readData: Unknown head: %1(%2)").arg(c).arg((char)c).toLocal8Bit());
		return c;
	}
	dataCount++;
	return 0;
}

void Connection::loop(void)
{
	static bool rateDisplay = false;
	bool send = false;
	message_t msg;
	while (!exit) {
		//qDebug() << "loop.";
		if (QTime::currentTime().second() % DATA_RATE_AVERAGE) {
			if (!rateDisplay) {
				qDebug(tr("Data rate: %1").arg((float)dataCount / (float)DATA_RATE_AVERAGE).toLocal8Bit());
				dataCount = 0;
				rateDisplay = true;
			}
		} else
			rateDisplay = false;
		if (queueLock.tryLock()) {
			if (!queue.isEmpty()) {
				msg = queue.dequeue();
				send = true;
			}
			queueLock.unlock();
			if (send)
				writeMessage(msg);
			send = false;
		}
		readData(10);
	}
	//qDebug() << "loop exit.";
	reset();
}

void Connection::enqueue(const message_t &msg)
{
	//qDebug() << "Connection::enqueue";
	//queueLock = true;
	queueLock.lock();
	if (!queue.isEmpty() && queue.last().similar(msg))
		queue.removeLast();
	queue.enqueue(msg);
	//queueLock = false;
	queueLock.unlock();
}
