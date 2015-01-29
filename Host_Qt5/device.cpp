#include <QtCore>
#include <QDebug>
#include "device.h"

Device::Device(QObject *parent) :
	QObject(parent)
{
	qRegisterMetaType<message_t>("message_t");
	qRegisterMetaType<info_t>("info_t");
	qRegisterMetaType<controller_t>("controller_t");
	qRegisterMetaType<analog_t>("analog_t");
	con = new Connection;
}

Device::~Device(void)
{
	con->quit();
	conThread.quit();
	conThread.wait();
}

bool Device::init(void)
{
	connect(&conThread, SIGNAL(started()), con, SLOT(loop()));
	connect(con, SIGNAL(error(QString)), this, SLOT(error(QString)));
	connect(con, SIGNAL(info(info_t)), this, SLOT(info(info_t)));
	connect(con, SIGNAL(controller(controller_t)), this, SLOT(controller(controller_t)));
	connect(con, SIGNAL(analog(analog_t)), this, SLOT(analog(analog_t)));
	if (!con->init()) {
		error(tr("Connection initialise failed"));
		return false;
	}
	con->moveToThread(&conThread);
	//connect(con, SIGNAL(failed()), qApp, SLOT(quit()));
	conThread.start();
	con->requestInfo();
	return true;
}

void Device::error(QString str)
{
	qWarning(tr("Error: %1").arg(str).toLocal8Bit());
	//qApp->quit();
}

void Device::info(info_t s)
{
	qDebug(tr("Device version: %1, name: %2").arg(s.version).arg(s.name).toLocal8Bit());
	if (s.version != FW_VERSION) {
		error(tr("Device firmware version mismatch"));
		qApp->quit();
	}
	emit deviceNameChanged(s.name);
}

void Device::controller(controller_t s)
{
	qDebug(tr("Controller id: %1, name: %2, controls: %3").arg(s.id).arg(s.name).arg(s.controls.count()).toLocal8Bit());
	for (int i = 0; i < s.controls.count(); i++) {
		struct controller_t::set_t set = s.controls.at(i);
		qDebug(tr("  Control id: %1, name: %2, type: %3, value: %6").arg(set.id).arg(set.name).arg(set.type).arg(set.value).toLocal8Bit());
	}
	emit controllerInfo(s);
}

void Device::analog(analog_t s)
{
	qDebug(tr("Analog id: %1, name: %2, resolution: %3, channels: %4").arg(s.id).arg(s.name).arg(s.resolution).arg(s.channels.count()).toLocal8Bit());
	for (int i = 0; i < s.channels.count(); i++) {
		struct analog_t::channel_t channel = s.channels.at(i);
		qDebug(tr("  Channel id: %1, name: %2").arg(channel.id).arg(channel.name).toLocal8Bit());
	}
	qDebug(tr("  Timer id: %1, resolution: %2, frequency: %3").arg(s.timer.id).arg(s.timer.resolution).arg(s.timer.frequency).toLocal8Bit());
	emit analogWaveform(s);
}
