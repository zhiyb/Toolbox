#include <QtWidgets>
#include <QDebug>
#include "controller.h"

Controller::Controller(controller_t *s, QWidget *parent) : QGroupBox(parent)
{
	setLayout(layout = new QHBoxLayout(this));
	rebuild(s);
}

Controller::~Controller()
{
}

void Controller::valueChanged(quint8 id)
{
	for (int i = 0; i < ctrl->controls.count(); i++) {
		controller_t::set_t &set = ctrl->controls[i];
		if (set.id != id)
			continue;
		if (set.readOnly()) {
			qWarning(tr("Value changed for read-only %1 at %2: %3").arg(set.id).arg(__FILE__).arg(__LINE__).toLocal8Bit());
			break;
		}
		struct message_t msg;
		msg.command = CMD_CONTROLLER;
		msg.id = ctrl->id;
		struct message_t::set_t msgSet;
		msgSet.id = set.id;
		msgSet.bytes = set.bytes();
		msgSet.value = set.value;
		msg.settings.enqueue(msgSet);
		msg.settings.enqueue(message_t::set_t());	// End of settings
		emit message(msg);
		break;
	}
}

void Controller::valueChanged(void)
{
	for (int i = 0; i < ctrl->controls.count(); i++) {
		struct controller_t::set_t &set = ctrl->controls[i];
		switch (set.type & ~CTRL_READONLY) {
		case CTRL_BYTE1:
		case CTRL_BYTE2:
		case CTRL_BYTE3:
		case CTRL_BYTE4: {
			QLabel *l = findChild<QLabel *>(QString::number(set.id), Qt::FindDirectChildrenOnly);
			if (!l) {
				qWarning(tr("QLabel not found for %1 at %2: %3").arg(set.id).arg(__FILE__).arg(__LINE__).toLocal8Bit());
				continue;
			}
			quint32 current = l->text().mid(l->text().lastIndexOf(':') + 1).toLong();
			if (!set.readOnly()) {
				QSlider *s = findChild<QSlider *>(QString::number(set.id), Qt::FindDirectChildrenOnly);
				if (!s) {
					qWarning(tr("QSlider not found for %1 at %2: %3").arg(set.id).arg(__FILE__).arg(__LINE__).toLocal8Bit());
					continue;
				}
				if (set.value != current)
					s->setValue(set.value);
				else if ((quint32)s->value() != current) {
					set.value = s->value();
					valueChanged(set.id);
				}
			}
			if (set.value != current)
				l->setText(tr("%1: %2").arg(set.name).arg(set.value));
		}
		}
	}
}

void Controller::rebuild(controller_t *s)
{
	while (layout->count())
		layout->removeItem(layout->itemAt(0));
	ctrl = s;
	setObjectName(QString::number(ctrl->id));
	setTitle(ctrl->name);
	QVBoxLayout *lay = new QVBoxLayout;
	layout->addLayout(lay);
	for (int i = 0; i < ctrl->controls.count(); i++) {
		struct controller_t::set_t set = ctrl->controls.at(i);
		switch (set.type & ~CTRL_READONLY) {
		case CTRL_NEW_COLUMN: {
			QFrame *f = new QFrame;
			f->setFrameShape(QFrame::VLine);
			f->setLineWidth(0);
			f->setMidLineWidth(0);
			layout->addWidget(f);
			layout->addLayout(lay = new QVBoxLayout);
			break;
		}
		case CTRL_BYTE1:
		case CTRL_BYTE2:
		case CTRL_BYTE3:
		case CTRL_BYTE4: {
			QLabel *l = new QLabel(tr("%1: %2").arg(set.name).arg(set.value));
			l->setObjectName(QString::number(set.id));
			lay->addWidget(l, 0, Qt::AlignHCenter);
			if (!set.readOnly()) {
				QSlider *s = new QSlider(Qt::Vertical);
				s->setObjectName(QString::number(set.id));
				s->setMinimum(set.min);
				s->setMaximum(set.max);
				s->setValue(set.value);
				s->setMaximumHeight(QWIDGETSIZE_MAX);
				connect(s, SIGNAL(valueChanged(int)), this, SLOT(valueChanged()));
				lay->addWidget(s, 0, Qt::AlignHCenter);
			}
			break;
		}
		}
	}
	valueChanged();
}
