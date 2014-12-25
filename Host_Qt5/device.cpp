#include <QtCore>
#include <QDebug>
#include "device.h"

Device::Device(QObject *parent) :
	QObject(parent)
{
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
	connect(con, SIGNAL(responseInfo(QString)), this, SLOT(responseInfo(QString)));
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
