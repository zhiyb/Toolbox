#ifndef DEVICE_H
#define DEVICE_H

#include <QObject>
#include <QThread>
#include "connection.h"
#include "structures.h"

class Device : public QObject
{
	Q_OBJECT
public:
	explicit Device(QObject *parent = 0);
	~Device(void);
	bool init(void);
	QString name(void) const {return data.name;}
	Connection *connection(void) {return con;}

signals:
	void deviceNameChanged(QString name);
	void controller(controller_t *s);
	void analog(analog_t *s);
	void messageSent(quint32 sequence);

public slots:
	void send(message_t s) {con->enqueue(s);}

private slots:
	void error(QString str);
	void info(info_t *s);
	void device(device_t s);
	void message(quint32 sequence) {emit messageSent(sequence);}

private:
	void controllerInfo(controller_t *s);
	void analogInfo(analog_t *s);

	QThread conThread;
	Connection *con;
	struct data_t {
		data_t(void) : name("Unknown") {}
		QString name;
	} data;
};

#endif // DEVICE_H
