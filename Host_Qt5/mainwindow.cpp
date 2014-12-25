#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	dev = new Device(this);
	data.init = dev->init();
	setWindowTitle(dev->name());
	connect(dev, SIGNAL(deviceNameChanged(QString)), this, SLOT(setWindowTitle(QString)));
#if 0
	setWindowFlags(Qt::FramelessWindowHint);
	setAttribute(Qt::WA_TranslucentBackground);
	QLabel *l = new QLabel(this);
	setCentralWidget(l);
	l->setPixmap(QPixmap("2D_.png"));
#endif
}

MainWindow::~MainWindow(void)
{
}

bool MainWindow::initialise(void)
{
	return data.init;
}

bool MainWindow::event(QEvent *e)
{
	switch (e->type()) {
	case QEvent::WindowActivate:
		setWindowOpacity(1.00);
		break;
	case QEvent::WindowDeactivate:
		setWindowOpacity(0.50);
		break;
	default:
		break;
	}
	return QMainWindow::event(e);
}
