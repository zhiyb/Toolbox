#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <inttypes.h>
#include <QtGlobal>
#include <QVector>
#include <QString>

struct message_t {
	char command;
	struct set_t {
		quint8 id, btyes;
		quint32 value;
	};
	QVector<set_t> settings;
};

struct control_t {
	quint8 id;
	struct set_t {
		enum Types {Toggle, Byte1, Byte2, Byte3, Byte4, Button, Radio, CheckBox, Selector, Selected, ReadOnly = 0x80};

		quint8 id, type, min, max;
		quint32 value;
		QString name;
	};
	QVector<set_t> controls;
};

#endif // STRUCTURES_H
