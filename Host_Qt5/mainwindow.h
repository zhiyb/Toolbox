#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QHBoxLayout>
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

private slots:
	void controller(controller_t s);

private:
	bool init;
	Device *dev;
	QHBoxLayout *layout;
};

#endif // MAINWINDOW_H
