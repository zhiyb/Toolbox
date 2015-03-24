#ifndef ANALOG_H
#define ANALOG_H

#include <QtWidgets>
#include "structures.h"
#include "device.h"
#include "analogwaveform.h"
#include "timebasectrl.h"
#include "analogtriggerctrl.h"

class Analog : public QWidget
{
	Q_OBJECT
public:
	explicit Analog(Device *dev, analog_t *analog, QWidget *parent = 0);
	~Analog();
	void rebuild(analog_t *analog);

signals:

public slots:
	void activate(void);
	void messageSent(quint32 sequence);
	void analogData(analog_t::data_t data);

protected:
	bool event(QEvent *e);
	void showEvent(QShowEvent *e);
	void hideEvent(QHideEvent *e);

private slots:
	void updateAt(quint32 sequence);

private:
	void initADC(void);
	void startADC(bool start);
	void configureTimer(void);

	QGridLayout *layout;
	QGridLayout *channelLayout;
	AnalogTriggerCtrl *trigger;
	AnalogWaveform *waveform;
	TimebaseCtrl *timebase;
	Device *dev;
	analog_t *analog;
	quint32 updateSequence;
};

#endif // ANALOGWAVEFORM_H
