#ifndef ANALOGCHANNELCTRL_H
#define ANALOGCHANNELCTRL_H

#include <QtWidgets>
#include "structures.h"
#include "scalevalue.h"
#include "device.h"

class AnalogChannelCtrl : public QGroupBox
{
	Q_OBJECT
public:
	explicit AnalogChannelCtrl(Device *dev, analog_t *analog, quint32 channelNum, QWidget *parent = 0);

signals:
	void valueChanged(void);
	void updateAt(quint32 sequence);

public slots:
	void updateValue(void);

private slots:
	void offsetChanged(void);
	void scaleChanged(void);
	void enabledChanged(void);

private:
	QDoubleSpinBox *offset;
	QCheckBox *enabled;
	ScaleValue *scale;
	Device *dev;
	analog_t *analog;
	analog_t::channel_t *channel;
};

#endif // ANALOGCHANNELCTRL_H
