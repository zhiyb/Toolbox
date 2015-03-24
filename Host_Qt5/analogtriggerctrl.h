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

private slots:
	void sourceChanged(int idx);

private:
	QComboBox *source;
	//ScaleValue *scale;
	Device *dev;
	analog_t *analog;
};

#endif // ANALOGTRIGGERCTRL_H
