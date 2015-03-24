#include "mainwindow.h"
#include "controller.h"
#include "analog.h"

MainWindow::MainWindow(QWidget *parent)
	: QMainWindow(parent)
{
	QWidget *w = new QWidget(this);
	setCentralWidget(w);
	QVBoxLayout *vLayout = new QVBoxLayout(w);
	vLayout->addLayout(dispLayout = new QVBoxLayout, 3);
	vLayout->addLayout(layout = new QHBoxLayout, 1);

	dev = new Device(this);
	gbWaveforms = 0;
	//setWindowTitle(dev->name());
	setAttribute(Qt::WA_QuitOnClose);

	connect(dev, SIGNAL(deviceNameChanged(QString)), this, SLOT(setWindowTitle(QString)));
	connect(dev, SIGNAL(controller(controller_t *)), this, SLOT(controller(controller_t *)));
	connect(dev, SIGNAL(analog(analog_t *)), this, SLOT(analog(analog_t *)));
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
		setWindowOpacity(0.90);
		break;
	case QEvent::WindowDeactivate:
		setWindowOpacity(0.85);
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

void MainWindow::controller(controller_t *s)
{
	Controller *w = findChild<Controller *>(QString::number(s->id), Qt::FindDirectChildrenOnly);
	if (!w) {
		Controller *c = new Controller(s);
		connect(c, SIGNAL(message(message_t)), dev, SLOT(send(message_t)));
		connect(dev, SIGNAL(messageSent(quint32)), c, SLOT(messageSent(quint32)));
		layout->addWidget(c);
	} else
		w->rebuild(s);
}

void MainWindow::analog(analog_t *s)
{
	if (!gbWaveforms) {
		gbWaveforms = new QGroupBox(tr("Waveforms"), this);
		new QVBoxLayout(gbWaveforms);
		layout->insertWidget(0, gbWaveforms);
	}
	Analog *analogCtrl = findChild<Analog *>(QString::number(s->id));
	if (analogCtrl)
		analogCtrl->rebuild(s);
	else {
		analogCtrl = new Analog(dev, s);
		dispLayout->addWidget(analogCtrl);
		analogCtrl->setObjectName(QString::number(s->id));
		connect(dev, SIGNAL(messageSent(quint32)), analogCtrl, SLOT(messageSent(quint32)));
	}
	QPushButton *pbWaveform = gbWaveforms->findChild<QPushButton *>(QString::number(s->id));
	if (pbWaveform)
		pbWaveform->setText(analogCtrl->windowTitle());
	else {
		pbWaveform = new QPushButton(analogCtrl->windowTitle());
		pbWaveform->setObjectName(QString::number(s->id));
		gbWaveforms->layout()->addWidget(pbWaveform);
		connect(pbWaveform, SIGNAL(clicked()), analogCtrl, SLOT(activate()));
		connect(dev->connection(), SIGNAL(analogData(analog_t::data_t)), analogCtrl, SLOT(analogData(analog_t::data_t)));
	}
}
