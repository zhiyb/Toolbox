#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <QGroupBox>
#include <QVBoxLayout>
#include "structures.h"

class Controller : public QGroupBox
{
	Q_OBJECT
public:
	explicit Controller(const controller_t &s, QWidget *parent = 0);
	~Controller();
	void rebuild(const controller_t &s);

signals:
	void message(message_t s);

public slots:

private slots:
	void valueChanged(void);

private:
	void valueChanged(quint8 id);

	QVBoxLayout *layout;
	controller_t ctrl;
};

#endif // CONTROLLER_H
