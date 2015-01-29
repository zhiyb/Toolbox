#include "mainwindow.h"
#include "controller.h"
#include "analogwaveform.h"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	QWidget *w = new QWidget(this);
	setCentralWidget(w);
	layout = new QHBoxLayout(w);
	dev = new Device(this);
	gbWaveforms = 0;
	//setWindowTitle(dev->name());
	setAttribute(Qt::WA_QuitOnClose);
	connect(dev, SIGNAL(deviceNameChanged(QString)), this, SLOT(setWindowTitle(QString)));
	connect(dev, SIGNAL(controllerInfo(controller_t)), this, SLOT(controller(controller_t)));
	connect(dev, SIGNAL(analogWaveform(analog_t)), this, SLOT(analogWaveform(analog_t)));
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
		setWindowOpacity(0.75);
		break;
	default:
		break;
	}
	return QMainWindow::event(e);
}

void MainWindow::closeEvent(QCloseEvent *e)
{
	qApp->closeAllWindows();
	//qApp->quit();
	QMainWindow::closeEvent(e);
}

void MainWindow::controller(controller_t s)
{
	Controller *w = findChild<Controller *>(QString::number(s.id), Qt::FindDirectChildrenOnly);
	if (!w) {
		Controller *c = new Controller(s, this);
		connect(c, SIGNAL(message(message_t)), dev, SLOT(send(message_t)));
		layout->addWidget(c);
	} else
		w->rebuild(s);
}

void MainWindow::analogWaveform(analog_t s)
{
	if (!gbWaveforms) {
		gbWaveforms = new QGroupBox(tr("Waveforms"), this);
		new QVBoxLayout(gbWaveforms);
		layout->insertWidget(0, gbWaveforms);
	}
	QPushButton *pbWaveform = new QPushButton(s.name);
	gbWaveforms->layout()->addWidget(pbWaveform);
	AnalogWaveform *waveform = new AnalogWaveform;
	waveform->setObjectName(QString::number(s.id));
	waveform->setWindowTitle(s.name);
	connect(pbWaveform, SIGNAL(clicked()), waveform, SLOT(show()));
}
