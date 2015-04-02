#ifndef ANALOGCHANNELCTRL_H
#define ANALOGCHANNELCTRL_H

#include <QtWidgets>
#include "colourselection.h"
#include "structures.h"
#include "scalevalue.h"
#include "device.h"
#include "dial.h"

class AnalogChannelCtrl : public QGroupBox
{
	Q_OBJECT
public:
	explicit AnalogChannelCtrl(Device *dev, analog_t *analog, quint32 channelNum, QWidget *parent = 0);

signals:
	void updateRequest(void);
	void information(QString name, QString content);

public slots:
	void updateValue(void);
	void updateColour(void);

private slots:
	void offsetReset(void);
	void offsetMoved(float frac);
	void scaleChanged(void);
	void modeChanged(int mode);
	void colourChanged(QColor clr);

private:
	void updateOffset(void);

	QLabel *lOffset;
	Dial *offset;
	QComboBox *mode;
	//QCheckBox *enabled, *ac;
	ColourSelection *colour;
	ScaleValue *scale;
	Device *dev;
	analog_t *analog;
	analog_t::channel_t *channel;
};

#endif // ANALOGCHANNELCTRL_H
