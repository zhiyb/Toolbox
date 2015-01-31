#ifndef ANALOG_H
#define ANALOG_H

#include <QtWidgets>
#include "structures.h"
#include "device.h"
#include "analogwaveform.h"

class Analog : public QDialog
{
	Q_OBJECT
public:
	explicit Analog(Device *dev, analog_t *analog, QWidget *parent = 0);
	~Analog();
	void rebuild(analog_t *analog);

signals:

public slots:
	void activate(void);
	void messageSent(quint32 sequence) {Q_UNUSED(sequence);}
	void analogData(analog_t::data_t data);

protected:
	bool event(QEvent *e);
	void showEvent(QShowEvent *e);
	void hideEvent(QHideEvent *e);

private:
	void initADC(void);
	void startADC(void);
	void stopADC(void);
	//void startTimer(void);
	//void stopTimer(void);
	void configureTimer(void);
	QGroupBox *buildChannelCtrl(const analog_t::channel_t &channel);
	QGroupBox *buildTimebaseCtrl(void);
	QGroupBox *buildTriggerCtrl(void);

	QGridLayout *layout;
	QVBoxLayout *channelLayout;
	QGroupBox *trigger, *timebase;
	AnalogWaveform *waveform;
	Device *dev;
	analog_t *analog;
};

#endif // ANALOGWAVEFORM_H
