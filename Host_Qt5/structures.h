#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <inttypes.h>
#include <QtGlobal>
#include <QVector>
#include <QQueue>
#include <QString>
#include <instructions.h>

struct message_t {
	message_t(void) : id(-1) {}
	bool similar(const message_t &msg);

	char command;
	quint8 id;
	struct set_t {
		quint8 id, bytes;
		quint32 value;
	};
	QQueue<set_t> settings;
};

struct info_t {
	QString name;
};

struct controller_t {
	quint8 id;
	QString name;
	struct set_t {
		bool readOnly(void) const {return type & CTRL_READONLY;}
		quint8 bytes(void) const {return type & 0x07;}

		quint8 id, type;
		quint32 min, max, value;
		QString name;
	};
	QVector<set_t> controls;
};

#endif // STRUCTURES_H
