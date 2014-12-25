#ifndef CONNECTION_H
#define CONNECTION_H

#include <QObject>
#include <QtWidgets>
#include <QTcpSocket>
#include <QSerialPort>
#include <QDialog>
#include "structures.h"

#define DEFAULT_NETWORK_HOST	"192.168.0.36"
#define DEFAULT_NETWORK_PORT	1111
#define DEFAULT_SERIAL_PORT	"COM1"
#define DEFAULT_SERIAL_SPEED	115200

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
	QTcpSocket *tcpSocket(void) {return (QTcpSocket *)con;}
	QSerialPort *serialPort(void) {return (QSerialPort *)con;}
	bool init(void);

signals:
	//void failed(void);
	void error(QString str);
	void responseInfo(QString str);

public slots:
	void loop(void);
	void quit(void) {exit = true;}
	void enqueue(const struct message_t& msg) {queue.enqueue(msg);}
	void requestInfo(void);

private slots:
	void pause(void);
	void reset(void);

private:
	void write(QByteArray& data);
	void writeChar(const char c);
	void writeRepeatedChar(const char c, const qint64 size);
	void writeMessage(const struct message_t& msg);
	int readChar(void);
	char readData(void);
	QString readString(void);
	void waitForWrite(int msec = COMMUNICATION_WAIT);
	void waitForRead(const qint64 size, int msec = COMMUNICATION_WAIT);
	void waitForReadAll(int msec = COMMUNICATION_WAIT) {while (con->waitForReadyRead(msec));}

	enum Types {Network = 0, SerialPort = 1};

	QIODevice *con;
	QQueue<struct message_t> queue;
	int type;
	bool ready;
	volatile bool exit;
};

#endif // CONNECTION_H
