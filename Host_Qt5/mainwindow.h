#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
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

private:
	Device *dev;
	struct {
		bool init;
	} data;
};

#endif // MAINWINDOW_H
