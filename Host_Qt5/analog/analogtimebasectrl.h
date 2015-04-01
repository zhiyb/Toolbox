#ifndef ANALOGTIMEBASECTRL_H
#define ANALOGTIMEBASECTRL_H

#include <QtWidgets>
#include "scalevalue.h"
#include "device.h"
#include "structures.h"

class AnalogTimebaseCtrl : public QGroupBox
{
	Q_OBJECT
public:
	explicit AnalogTimebaseCtrl(Device *dev, analog_t *analog, QWidget *parent = 0);
	~AnalogTimebaseCtrl();

signals:
	void updateRequest(void);
	void updateDisplay(void);
	void information(QString name, QString content);

public slots:

private slots:
	void scaleChanged(void);

private:
	ScaleValue *scale;
	Device *dev;
	analog_t *analog;
};

#endif // ANALOGTIMEBASECTRL_H
