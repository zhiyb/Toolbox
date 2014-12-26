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
//#include "structures.h"

ConnectionSelection::ConnectionSelection(QWidget *parent) :
	QDialog(parent)
{
	type = Connection::Network;
	host = DEFAULT_NETWORK_HOST;
	port = DEFAULT_NETWORK_PORT;
	serialPort = DEFAULT_SERIAL_PORT;
	serialSpeed = DEFAULT_SERIAL_SPEED;

	QVBoxLayout *vLayout = new QVBoxLayout(this);
	QHBoxLayout *hTypeLayout = new QHBoxLayout, *hSettingsLayout = new QHBoxLayout, *hButtonLayout = new QHBoxLayout;
	vLayout->addLayout(hTypeLayout);
	hTypeLayout->addWidget(rbTypes[Connection::Network] = new QRadioButton("&Network"));
	rbTypes[Connection::Network]->setChecked(true);
	hTypeLayout->addWidget(rbTypes[Connection::SerialPort] = new QRadioButton("&Serial Port"));
	vLayout->addLayout(hSettingsLayout);
	hSettingsLayout->addWidget(lHost = new QLabel);
	hSettingsLayout->addWidget(leHost = new QLineEdit, 2);
	hSettingsLayout->addWidget(pbScan = new QPushButton("S&can"));
	pbScan->setVisible(false);
	hSettingsLayout->addWidget(lPort = new QLabel);
	hSettingsLayout->addWidget(lePort = new QLineEdit, 1);
	vLayout->addLayout(hButtonLayout);
	hButtonLayout->addWidget(pbOpen = new QPushButton("Open"));
	pbOpen->setDefault(true);
	hButtonLayout->addWidget(pbCancel = new QPushButton("Cancel"));
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
		QMessageBox::information(this, "Serial port selection", "No serial port available!");
		return;
	}
	QString sel = QInputDialog::getItem(this, "Serial port selection", "Please select a serial port hardware from the list:", names, 0, false);
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
		lHost->setText("Host:");
		leHost->setText(host);
		lPort->setText("Port:");
		lePort->setText(QString::number(port));
		pbScan->setVisible(false);
	} else if (rbTypes[Connection::SerialPort]->isChecked()) {
		type = Connection::SerialPort;
		lHost->setText("Serial port:");
		leHost->setText(serialPort);
		lPort->setText("Speed:");
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
	queueLock = false;
	exit = false;
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

void Connection::writeChar(const char c)
{
	con->write(&c, 1);
}

void Connection::writeRepeatedChar(const char c, const qint64 size)
{
	for (qint64 i = 0; i != size; i++)
		writeChar(c);
}

void Connection::pause(void)
{
}

void Connection::reset(void)
{
	//pause();
	//qDebug() << "Connection::reset";
	writeRepeatedChar(CMD_RESET, /*PKG_SIZE*/32);
	waitForWrite();
	waitForReadAll();
	QByteArray data = con->readAll();
	if (!data.endsWith(CMD_ACK)) {
		//qDebug() << data;
		emit error("Package sync error: " + data);
		return;
	}
	//sendChar(CMD_ACK);
	//waitForWrite();
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
			QMessageBox::critical(0, "Serial port error", "Cannot open selected serial port!");
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

QString Connection::readString(void)
{
	char c;
	QString str;
	forever {
		c = readChar(-1);
		if (c == '\0')
			break;
		str.append(c);
	}
	return str;
}

void Connection::writeMessage(message_t msg)
{
	//qDebug() << "writeMessage";
	writeChar(msg.command);
	waitForWrite();
	char c;
	while ((c = readData()) == 0);
	if (c != CMD_ACK) {
		emit error(QString("No ACK received for command '%1': %2").arg(msg.command).arg((int)c, 0, 16));
		return;
	}
	if (msg.id != (quint8)-1)
		writeChar(msg.id);
	while (msg.settings.count()) {
		struct message_t::set_t set = msg.settings.dequeue();
		writeChar(set.id);
		writeValue(set.value, set.bytes);
	}
}

struct info_t Connection::readInfo(void)
{
	struct info_t info;
	info.name = readString();
	return info;
}

struct controller_t Connection::readController(void)
{
	//qDebug() << "Connection::readController";
	struct controller_t ctrl;
	ctrl.id = readChar();
	if (ctrl.id == (quint8)-1) {
		emit error("Invalid controller ID");
		return ctrl;
	}
	ctrl.name = readString();
	forever {
		struct controller_t::set_t set;
		set.id = readChar();
		if (set.id == (quint8)-1)
			return ctrl;
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
		ctrl.controls.append(set);
	}
}

char Connection::readData(int msec)
{
	//qDebug() << "readData";
	char c;
	switch (c = readChar(msec)) {
	case CMD_ACK:
		return CMD_ACK;
	case CMD_INFO:
		emit info(readInfo());
		break;
	case CMD_CONTROLLER:
		emit controller(readController());
		break;
	default:
		return c;
	}
	return 0;
}

void Connection::loop(void)
{
	while (!exit) {
		//qDebug() << "loop.";
		if (!queueLock && queue.count() != 0)
			writeMessage(queue.dequeue());
		readData(10);
	}
	reset();
}

void Connection::enqueue(const message_t &msg)
{
	queueLock = true;
	if (!queue.isEmpty() && queue.last().similar(msg))
		queue.removeLast();
	queueLock = false;
	queue.enqueue(msg);
}
