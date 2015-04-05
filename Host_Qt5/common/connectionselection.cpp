#include <QMessageBox>
#include <QInputDialog>
#include "connectionselection.h"

ConnectionSelection::ConnectionSelection(QWidget *parent) : QDialog(parent)
{
#if defined(ENABLE_NETWORK)
	type = Network;
#elif defined(ENABLE_SERIALPORT)
	type = SerialPort;
#endif

#ifdef ENABLE_NETWORK
	host = DEFAULT_NETWORK_HOST;
	port = DEFAULT_NETWORK_PORT;
#endif

#ifdef ENABLE_SERIALPORT
	serialPort = DEFAULT_SERIAL_PORT;
	serialSpeed = DEFAULT_SERIAL_BAUD;
	serialPorts = QSerialPortInfo::availablePorts();
#endif

	QVBoxLayout *vLayout = new QVBoxLayout(this);
	QHBoxLayout *hTypeLayout = new QHBoxLayout, *hSettingsLayout = new QHBoxLayout, *hButtonLayout = new QHBoxLayout;
	vLayout->addLayout(hTypeLayout);
#ifdef ENABLE_NETWORK
	hTypeLayout->addWidget(rbTypes[Network] = new QRadioButton(tr("&Network")));
#endif
#ifdef ENABLE_SERIALPORT
	hTypeLayout->addWidget(rbTypes[SerialPort] = new QRadioButton(tr("&Serial Port")));
#endif
	vLayout->addLayout(hSettingsLayout);
	hSettingsLayout->addWidget(lHost = new QLabel);
	hSettingsLayout->addWidget(leHost = new QLineEdit, 2);
	hSettingsLayout->addWidget(pbScan = new QPushButton(tr("Sc&an")));
	hSettingsLayout->addWidget(lPort = new QLabel);
	hSettingsLayout->addWidget(lePort = new QLineEdit, 1);
	vLayout->addLayout(hButtonLayout);
	hButtonLayout->addWidget(pbOpen = new QPushButton(tr("&Open")));
	pbOpen->setDefault(true);
	hButtonLayout->addWidget(pbCancel = new QPushButton(tr("&Cancel")));
	setWindowTitle("Connection settings");
	setWindowFlags(Qt::Dialog | Qt::MSWindowsFixedSizeDialogHint);

#if defined(ENABLE_NETWORK)
	rbTypes[Network]->setChecked(true);
	pbScan->setVisible(false);
#elif defined(ENABLE_SERIALPORT)
	rbTypes[SerialPort]->setChecked(true);
#endif

#ifdef ENABLE_NETWORK
	connect(rbTypes[Network], SIGNAL(clicked()), this, SLOT(typeUpdate()));
#endif
#ifdef ENABLE_SERIALPORT
	connect(rbTypes[SerialPort], SIGNAL(clicked()), this, SLOT(typeUpdate()));
	connect(pbScan, SIGNAL(clicked()), this, SLOT(scan()));
#endif
	connect(pbOpen, SIGNAL(clicked()), this, SLOT(accept()));
	connect(pbCancel, SIGNAL(clicked()), this, SLOT(reject()));

	typeUpdate();
}

#ifdef ENABLE_SERIALPORT
void ConnectionSelection::scan(void)
{
	serialPorts = QSerialPortInfo::availablePorts();
	QStringList names;
	for (int i = 0; i < serialPorts.count(); i++)
		names.append(serialPorts.at(i).description() + " (" + serialPorts.at(i).portName() + ")");
	if (names.count() == 0) {
		QMessageBox::information(this, tr("Serial port selection"), tr("No serial port available!"));
		return;
	}
	QString sel = QInputDialog::getItem(this, tr("Serial port selection"), tr("Please select a serial port hardware from the list:"), names, 0, false);
	if (!sel.isEmpty())
		for (int i = 0; i < serialPorts.count(); i++)
			if (names.at(i) == sel) {
				leHost->setText(serialPorts.at(i).portName());
				return;
			}
}
#endif

void ConnectionSelection::normalize(void)
{
	bool ok;
	QString tmpHost = leHost->text();
	int tmpPort = lePort->text().toInt(&ok);
	switch (type) {
#ifdef ENABLE_NETWORK
	case Network:
		if (tmpHost.isEmpty())
			tmpHost = DEFAULT_NETWORK_HOST;
		if (!ok || tmpPort <= 0)
			tmpPort = DEFAULT_NETWORK_PORT;
		host = tmpHost;
		port = tmpPort;
		break;
#endif
#ifdef ENABLE_SERIALPORT
	case SerialPort:
		if (tmpHost.isEmpty())
			tmpHost = DEFAULT_SERIAL_PORT;
		if (!ok || tmpPort <= 0)
			tmpPort = DEFAULT_SERIAL_BAUD;
		serialPort = tmpHost;
		serialSpeed = tmpPort;
		break;
#endif
	default:
		break;
	}
}

void ConnectionSelection::typeUpdate(void)
{
	normalize();
#ifdef ENABLE_NETWORK
	if (rbTypes[Network]->isChecked()) {
		type = Network;
		lHost->setText(tr("Host:"));
		leHost->setText(host);
		lPort->setText(tr("Port:"));
		lePort->setText(QString::number(port));
		pbScan->setVisible(false);
	}
#endif
#ifdef ENABLE_SERIALPORT
	if (rbTypes[SerialPort]->isChecked()) {
		type = SerialPort;
		lHost->setText(tr("Serial port:"));
		leHost->setText(serialPort);
		lPort->setText(tr("Speed:"));
		lePort->setText(QString::number(serialSpeed));
		pbScan->setVisible(true);
	}
#endif
}

void ConnectionSelection::accept(void)
{
	normalize();
	if (!leHost->text().isEmpty()) {
#ifdef ENABLE_SERIALPORT
		if (type == SerialPort)
			for (int i = 0; i < serialPorts.count(); i++)
				if (serialPorts.at(i).portName() == leHost->text())
					serialInfo = serialPorts.at(i);
#endif
		QDialog::accept();
	}
}
