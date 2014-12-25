#ifndef DEVICE_H
#define DEVICE_H

#include <QObject>
#include <QThread>
#include "connection.h"

class Device : public QObject
{
	Q_OBJECT
public:
	explicit Device(QObject *parent = 0);
	~Device(void);
	bool init(void);
	QString name(void) const {return data.name;}

signals:
	void deviceNameChanged(QString name);

public slots:

private slots:
	void error(QString str);
	void responseInfo(QString str) {emit deviceNameChanged(str);}

private:
	QThread conThread;
	Connection *con;
	struct data_t {
		data_t(void) : name("Unknown") {}
		QString name;
	} data;
};

#endif // DEVICE_H
