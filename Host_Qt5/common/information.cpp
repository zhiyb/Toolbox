#include "information.h"

Information::Information(QWidget *parent) : QWidget(parent)
{
	layout = new QVBoxLayout(this);
	layout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding));
	setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
}

void Information::information(QString name, QString content)
{
	if (!isVisible())
		show();
	QLabel *l = findChild<QLabel *>(name);
	if (!l) {
		layout->insertWidget(layout->count() - 1, l = new QLabel);
		l->setObjectName(name);
		//l->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
	}
	l->setText(tr("%2").arg(content));
}
