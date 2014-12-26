#include "mainwindow.h"
#include "controller.h"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	QWidget *w = new QWidget(this);
	setCentralWidget(w);
	layout = new QHBoxLayout(w);
	dev = new Device(this);
	setWindowTitle(dev->name());
	connect(dev, SIGNAL(deviceNameChanged(QString)), this, SLOT(setWindowTitle(QString)));
	connect(dev, SIGNAL(controllerInfo(controller_t)), this, SLOT(controller(controller_t)));
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
	return dev->init();
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

void MainWindow::controller(controller_t s)
{
	Controller *w = layout->findChild<Controller *>(QString::number(s.id), Qt::FindDirectChildrenOnly);
	if (!w) {
		Controller *c = new Controller(s, this);
		connect(c, SIGNAL(message(message_t)), dev, SLOT(send(message_t)));
		layout->addWidget(c);
	}
}
