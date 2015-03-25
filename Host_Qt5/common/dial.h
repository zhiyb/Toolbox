#ifndef DIAL_H
#define DIAL_H

#include <QtWidgets>

class Dial : public QDial
{
	Q_OBJECT
public:
	explicit Dial(QWidget *parent = 0);
	void setPrecision(int v);
	int precision(void) const {return prec;}

signals:
	void moved(float frac);
	void rightClicked(void);

public slots:
	void reset(void);

protected:
	void mouseReleaseEvent(QMouseEvent *me);

private slots:
	void valueChanged(int v);

private:
	int stepsFrom(int v);
	int prev, prec;
};

#endif // DIAL_H
