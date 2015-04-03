#include <QtCore>
#include <QDebug>
#include "device.h"
#include "debug.h"

Device::Device(QObject *parent) :
	QObject(parent)
{
	qRegisterMetaType<message_t>("message_t");
	qRegisterMetaType<device_t>("device_t");
	qRegisterMetaType<analog_t::data_t>("analog_t::data_t");
	con = new Connection;
	connect(con, SIGNAL(information(QString,QString)), this, SIGNAL(information(QString,QString)));
}

Device::~Device(void)
{
	conThread.quit();
	conThread.wait();
}

bool Device::init(void)
{
	connect(&conThread, SIGNAL(started()), con, SLOT(start()));
	connect(&conThread, SIGNAL(finished()), con, SLOT(reset()));
	connect(con, SIGNAL(error(QString)), this, SLOT(error(QString)));
	connect(con, SIGNAL(info(info_t *)), this, SLOT(info(info_t *)));
	connect(con, SIGNAL(device(device_t)), this, SLOT(device(device_t)));
	connect(con, SIGNAL(messageSent(quint32)), this, SIGNAL(messageSent(quint32)));
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
	qWarning(tr("[ERROR] %1").arg(str).toLocal8Bit());
	//qApp->quit();
}

void Device::device(device_t s)
{
	pr_info(tr("Device name: %1, firmwave version: %2").arg(s.name).arg(s.version));
	if (s.version != FW_VERSION) {
		error(tr("Device firmware version mismatch"));
		qApp->quit();
	}
	emit deviceNameChanged(s.name);
}

void Device::info(info_t *s)
{
	if (!s)
		return;
	switch (s->type()) {
	case CMD_CONTROLLER:
		controllerInfo((controller_t *)s);
		break;
	case CMD_ANALOG:
		analogInfo((analog_t *)s);
		break;
	}
}

void Device::controllerInfo(controller_t *s)
{
	pr_info(tr("Controller ID: %1, name: %2, controls: %3").arg(s->id).arg(s->name).arg(s->controls.count()));
	for (int i = 0; i < s->controls.count(); i++) {
		const controller_t::set_t &set = s->controls.at(i);
		pr_info(tr("  Control ID: %1, name: %2, type: %3, value: %6").arg(set.id).arg(set.name).arg(set.type).arg(set.value));
	}
	emit controller(s);
}

void Device::analogInfo(analog_t *s)
{
	pr_info(tr("Analog ID: %1, name: %2, resolution: %3, scanFrequency: %4, maxFrequency: %5, channels: %6, enabled: %7")\
	       .arg(s->id).arg(s->name).arg(s->resolution).arg(s->scanFrequency).arg(s->maxFrequency).arg(s->channel.count()).arg(s->channelsEnabled(true), 0, 2));
	for (int i = 0; i < s->channel.count(); i++) {
		const analog_t::channel_t &channel = s->channel.at(i);
		pr_info(tr("  Channel ID: %1, name: %2, reference: %3, offset: %4").arg(channel.id).arg(channel.name).arg(channel.reference).arg(channel.offset));
	}
	pr_info(tr("  Buffer size: %1").arg(s->buffer.size));
	pr_info(tr("  Timer ID: %1, resolution: %2, clock frequency: %3").arg(s->timer.id).arg(s->timer.resolution).arg(s->timer.clockFrequency));
	emit analog(s);
}
