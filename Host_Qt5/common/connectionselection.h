#ifndef CONNECTIONSELECTION_H
#define CONNECTIONSELECTION_H

#define ENABLE_NETWORK
#define ENABLE_SERIALPORT

#if !defined(ENABLE_NETWORK) && !defined(ENABLE_SERIALPORT)
#error At lease one connection method should be enabled!
#endif

#ifdef ENABLE_NETWORK
#include <QTcpSocket>
#include <QHostAddress>
#define DEFAULT_NETWORK_HOST	"192.168.6.48"
#define DEFAULT_NETWORK_PORT	1111
#endif

#ifdef ENABLE_SERIALPORT
#include <QSerialPort>
#include <QSerialPortInfo>
#define DEFAULT_SERIAL_PORT	"COM1"
#define DEFAULT_SERIAL_BAUD	UART_BAUD
#endif

#include <QtWidgets>
#include <instructions.h>

class ConnectionSelection : public QDialog
{
	Q_OBJECT
	friend class Connection;
public:
	explicit ConnectionSelection(QWidget *parent = 0);
	enum Types {Network = 0, SerialPort};

public slots:
	void accept(void);

private slots:
	void typeUpdate(void);
#ifdef ENABLE_SERIALPORT
	void scan(void);
#endif

private:
	void normalize(void);

	QRadioButton *rbTypes[2];
	QLineEdit *leHost, *lePort;
	QLabel *lHost, *lPort;
	QPushButton *pbScan, *pbOpen, *pbCancel;
	Types type;
#ifdef ENABLE_NETWORK
	QString host;
	int port;
#endif
#ifdef ENABLE_SERIALPORT
	QList<QSerialPortInfo> serialPorts;
	QSerialPortInfo serialInfo;
	QString serialPort;
	int serialSpeed;
#endif
};

#endif // CONNECTIONSELECTION_H
