#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets>
#include "device.h"

class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	MainWindow(QWidget *parent = 0);
	~MainWindow(void);
	bool initialise(void);

protected:
	bool event(QEvent *e);
	//void hideEvent(QHideEvent *e);
	void closeEvent(QCloseEvent *e);

private slots:
	void controller(controller_t *s);
	void analog(analog_t *s);

private:
	bool init;
	Device *dev;
	QVBoxLayout *dispLayout;
	QHBoxLayout *layout;
	QGroupBox *gbWaveforms;
};

#endif // MAINWINDOW_H
