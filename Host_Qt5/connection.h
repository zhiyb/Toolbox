#ifndef CONNECTION_H
#define CONNECTION_H

#include <QObject>
#include <QtWidgets>
#include <QTcpSocket>
#include <QSerialPort>
#include <QDialog>
#include "structures.h"

#define COMMUNICATION_WAIT	1000

class ConnectionSelection : public QDialog
{
	Q_OBJECT
	friend class Connection;
public:
	explicit ConnectionSelection(QWidget *parent = 0);

public slots:
	void accept(void);

private slots:
	void typeUpdate(void);
	void scan(void);

private:
	void normalize(void);

	QRadioButton *rbTypes[2];
	QLineEdit *leHost, *lePort;
	QLabel *lHost, *lPort;
	QPushButton *pbScan, *pbOpen, *pbCancel;
	int type;
	QString host, serialPort;
	int port, serialSpeed;
};

class Connection : public QObject
{
	Q_OBJECT
	friend class ConnectionSelection;
public:
	explicit Connection(QObject *parent = 0);
	~Connection(void);
	QTcpSocket *tcpSocket(void) {return (QTcpSocket *)con;}
	QSerialPort *serialPort(void) {return (QSerialPort *)con;}
	bool init(void);

signals:
	//void failed(void);
	void error(QString str);
	void device(device_t s);
	//void controller(controller_t s);
	//void analog(analog_t s);
	void analogData(analog_t::data_t s);
	void info(info_t *s);
	void messageSent(quint32 sequence);

public slots:
	void loop(void);
	void quit(void) {exit = true;}
	void enqueue(const message_t &msg);
	void requestInfo(void);

private slots:
	//void pause(void);
	void reset(void);
	void resync(void);

private:
	void pushInfo(info_t *s);
	info_t *findInfo(const quint8 type, const quint8 id);
	device_t readDeviceInfo(void);
	controller_t *readController(void);
	timer_t readTimer(void);
	analog_t *readAnalog(void);
	analog_t::data_t readAnalogData(void);
	void write(QByteArray &data);
	void writeChar(const char c);
	void writeValue(const quint32 value, const quint32 bytes);
	void writeRepeatedChar(const char c, const qint64 size);
	void writeMessage(message_t &msg);
	int readChar(int msec = COMMUNICATION_WAIT);
	quint32 readValue(const quint32 bytes, int msec = COMMUNICATION_WAIT);
	char readData(int msec = COMMUNICATION_WAIT);
	QString readString(int msec = COMMUNICATION_WAIT);
	void waitForWrite(int msec = COMMUNICATION_WAIT);
	void waitForRead(const qint64 size, int msec = COMMUNICATION_WAIT);
	bool waitForReadAll(int count = -1, int msec = COMMUNICATION_WAIT);

	enum Types {Network = 0, SerialPort = 1};

	QIODevice *con;
	QQueue<message_t> queue;
	QMutex queueLock;
	QVector<info_t *> infos;
	int type, dataCount;
	volatile bool exit;//, queueLock;
};

#endif // CONNECTION_H
