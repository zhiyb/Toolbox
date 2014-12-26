#include <QtCore>
#include <QDebug>
#include "device.h"

Device::Device(QObject *parent) :
	QObject(parent)
{
	qRegisterMetaType<struct message_t>("message_t");
	qRegisterMetaType<struct info_t>("info_t");
	qRegisterMetaType<struct controller_t>("controller_t");
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
	connect(con, SIGNAL(info(const struct info_t&)), this, SLOT(info(const struct info_t&)));
	connect(con, SIGNAL(controller(const struct controller_t&)), this, SLOT(controller(const struct controller_t&)));
	if (!con->init()) {
		qDebug() << "Connection::init failed";
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
	qWarning(QString("Fatal error: " + str).toLocal8Bit());
	//qApp->quit();
}
