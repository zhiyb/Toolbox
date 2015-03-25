#ifndef COLOURSELECTION_H
#define COLOURSELECTION_H

#include <QWidget>

class ColourSelection : public QWidget
{
	Q_OBJECT
public:
	explicit ColourSelection(QColor clr, QWidget *parent = 0);
	QColor colour(void) const {return clr;}

signals:
	void colourChanged(QColor clr);

public slots:
	void setColour(QColor clr);

protected:
	void mouseReleaseEvent(QMouseEvent *e);

private:
	QColor clr;
};

#endif // COLOURSELECTION_H
