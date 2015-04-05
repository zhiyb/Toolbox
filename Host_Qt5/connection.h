#ifndef CONNECTION_H
#define CONNECTION_H

#include <QtWidgets>
#include "connectionselection.h"
#include "structures.h"

#ifdef ENABLE_NETWORK
#include <QTcpSocket>
#endif
#ifdef ENABLE_SERIALPORT
#include <QSerialPort>
#endif

#define COMMUNICATION_WAIT	1000

class Connection : public QObject
{
	Q_OBJECT
	friend class ConnectionSelection;
public:
	explicit Connection(QObject *parent = 0);
	~Connection(void);
#ifdef ENABLE_NETWORK
	QTcpSocket *tcpSocket(void) {return (QTcpSocket *)con;}
#endif
#ifdef ENABLE_SERIALPORT
	QSerialPort *serialPort(void) {return (QSerialPort *)con;}
#endif
	bool init(void);

signals:
	void error(QString str);
	void device(device_t s);
	void analogData(analog_t::data_t s);
	void info(info_t *s);
	void messageSent(quint32 sequence);
	void information(QString name, QString content);

public slots:
	void start(void);
	bool reset(void);
	void enqueue(const message_t &msg);
	void requestInfo(void) {enqueue(message_t(CMD_INFO, INVALID_ID));}

protected:
	void timerEvent(QTimerEvent *e);

private:
	void resync(void) {writeRepeatedChar(INVALID_ID, 16); counter.tx += 16;}
	void quickResync(void) {writeRepeatedChar(INVALID_ID, 4); counter.tx += 4;}
	void pushInfo(info_t *s);
	QVector<info_t *> findInfos(const quint8 type);
	info_t *findInfo(const quint8 type, const quint8 id);
	device_t readDeviceInfo(void);
	controller_t *readController(void);
	hwtimer_t readTimer(void);
	analog_t *readAnalog(void);
	analog_t::data_t readAnalogData(void);
	void write(QByteArray &data);
	void writeChar(const char c);
	void writeValue(const quint32 value, const quint32 bytes);
	void writeRepeatedChar(const char c, const qint64 size);
	void writeMessage(message_t &msg);
	int readChar(int msec = COMMUNICATION_WAIT, bool debug = true);
	quint32 readValue(const quint32 bytes, int msec = COMMUNICATION_WAIT);
	char readData(int msec = COMMUNICATION_WAIT);
	QString readString(int msec = COMMUNICATION_WAIT);
	void waitForWrite(int msec = COMMUNICATION_WAIT);
	void waitForRead(const qint64 size, int msec = COMMUNICATION_WAIT);
	bool waitForReadAll(int counter = -1, int msec = COMMUNICATION_WAIT);
	void report(void);
	void report(info_t *info);

	struct counter_t {
		counter_t(void) {reset();}
		void reset(const QTime &now = QTime::currentTime());

		QTime prev;
		int txPackage, rxPackage, tx, rx;
	} counter;
	struct timer_t {
		timer_t(void) : update(0), report(0) {}

		int update, report;
	} timer;

	QIODevice *con;
	QQueue<message_t> queue;
	QMutex queueLock;
	QVector<info_t *> infos;
	int type;
};

#endif // CONNECTION_H
