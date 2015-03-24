#ifndef TIMEBASECTRL_H
#define TIMEBASECTRL_H

#include <QtWidgets>
#include "scalevalue.h"
#include "device.h"
#include "structures.h"

class TimebaseCtrl : public QGroupBox
{
	Q_OBJECT
public:
	explicit TimebaseCtrl(Device *dev, analog_t *analog, QWidget *parent = 0);
	~TimebaseCtrl();

signals:
	void updateConfigure(void);
	void updateDisplay(void);

public slots:

private slots:
	void scaleChanged(void);

private:
	ScaleValue *scale;
	Device *dev;
	analog_t *analog;
};

#endif // TIMEBASECTRL_H
