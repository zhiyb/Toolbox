#ifndef SCALEVALUE_H
#define SCALEVALUE_H

#include <QtWidgets>
#include "structures.h"

class ScaleValue : public QToolButton
{
	Q_OBJECT
public:
	explicit ScaleValue(scale_t *scale, QString unit, QWidget *parent = 0);
	~ScaleValue();

signals:
	void valueChanged(float value);

public slots:
	void updateValue(void);

protected:
	void mouseReleaseEvent(QMouseEvent *e);

private:
	QString unit;
	scale_t *scale;
};

#endif // SCALEVALUE_H
