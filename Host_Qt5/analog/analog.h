#ifndef ANALOG_H
#define ANALOG_H

#include <QtWidgets>
#include "structures.h"
#include "device.h"
#include "analogwaveform.h"
#include "analogtimebasectrl.h"
#include "analogtriggerctrl.h"

class Analog : public QWidget
{
	Q_OBJECT
public:
	explicit Analog(Device *dev, analog_t *analog, QWidget *parent = 0);
	void rebuild(analog_t *analog);

signals:
	void information(QString name, QString content);

public slots:
	void activate(void);
	void messageSent(quint32 sequence);
	void analogData(analog_t::data_t data);

protected:
	bool event(QEvent *e);
	void showEvent(QShowEvent *e);
	void hideEvent(QHideEvent *e);

private slots:
	void updateConfigure(void);
	void updateTriggerSettings(void) {}
	void updateTimer(void);

private:
	void initADC(void);
	void startADC(bool start);
	void updateAt(quint32 sequence) {updateSequence = sequence;}
	bool timerUpdateRequired(void) {return analog->timer.value != analog->timer.configure.value;}

	QGridLayout *layout;
	QGridLayout *channelLayout;
	AnalogTriggerCtrl *trigger;
	AnalogWaveform *waveform;
	AnalogTimebaseCtrl *timebase;
	Device *dev;
	analog_t *analog;
	quint32 updateSequence;
};

#endif // ANALOGWAVEFORM_H
