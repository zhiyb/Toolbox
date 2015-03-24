#include <QMouseEvent>
#include <QColorDialog>
#include "colourselection.h"

ColourSelection::ColourSelection(QColor clr, QWidget *parent) : QWidget(parent)
{
	setAutoFillBackground(true);
	setFocusPolicy(Qt::ClickFocus);

	QPalette pal(palette());
	pal.setColor(QPalette::Window, clr);
	setPalette(pal);
	this->clr = clr;
}

void ColourSelection::setColour(QColor clr)
{
	QPalette pal(palette());
	pal.setColor(QPalette::Window, clr);
	setPalette(pal);
	this->clr = clr;
	emit colourChanged(clr);
}

void ColourSelection::mouseReleaseEvent(QMouseEvent *e)
{
	QWidget::mouseReleaseEvent(e);
	//if (!e->isAccepted())
	//	return;
	QColor clr = QColorDialog::getColor(this->clr, this, QString());
	if (!clr.isValid())
		return;
	setColour(clr);
}
