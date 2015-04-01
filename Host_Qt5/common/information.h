#ifndef INFORMATION_H
#define INFORMATION_H

#include <QtWidgets>

class Information : public QWidget
{
	Q_OBJECT
public:
	explicit Information(QWidget *parent = 0);

signals:

public slots:
	void information(QString name, QString content);

private:
	QVBoxLayout *layout;
};

#endif // INFORMATION_H
