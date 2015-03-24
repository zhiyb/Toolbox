#ifndef ANALOGTRIGGERCTRL_H
#define ANALOGTRIGGERCTRL_H

#include <QtWidgets>
#include "device.h"
#include "structures.h"

class AnalogTriggerCtrl : public QGroupBox
{
	Q_OBJECT
public:
	explicit AnalogTriggerCtrl(Device *dev, analog_t *analog, QWidget *parent = 0);
	~AnalogTriggerCtrl();

private:
	//ScaleValue *scale;
	Device *dev;
	analog_t *analog;
};

#endif // ANALOGTRIGGERCTRL_H
