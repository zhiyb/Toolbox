#ifndef ANALOGTRIGGERCTRL_H
#define ANALOGTRIGGERCTRL_H

#include <QtWidgets>
#include "device.h"
#include "structures.h"
#include "dial.h"

class AnalogTriggerCtrl : public QGroupBox
{
	Q_OBJECT
public:
	explicit AnalogTriggerCtrl(Device *dev, analog_t *analog, QWidget *parent = 0);

signals:
	void updateTrigger(void);
	void updateTriggerSettings(void);

private slots:
	void sourceChanged(int idx);
	void levelChanged(float frac);
	void levelReset(void);

private:
	void reset(void);

	QComboBox *source;
	QLabel *lLevel;
	Dial *dLevel;
	Device *dev;
	analog_t *analog;
};

#endif // ANALOGTRIGGERCTRL_H
