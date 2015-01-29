#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <inttypes.h>
#include <QtGlobal>
#include <QVector>
#include <QQueue>
#include <QString>
#include <instructions.h>

struct message_t {
	message_t(void) : id(INVALID_ID) {}
	bool similar(const message_t &msg) const;

	char command;
	quint8 id;
	struct set_t {
		set_t(void) : id(INVALID_ID) {}

		quint8 id, bytes;
		quint32 value;
	};
	QQueue<set_t> settings;
};

struct info_t {
	quint32 version;
	QString name;
};

struct controller_t {
	controller_t(void) : id(INVALID_ID) {}

	quint8 id;
	QString name;
	struct set_t {
		set_t(void) : id(INVALID_ID) {}

		bool readOnly(void) const {return type & CTRL_READONLY;}
		quint8 bytes(void) const {return type & 0x07;}

		quint8 id, type;
		quint32 min, max, value;
		QString name;
	};
	QVector<set_t> controls;
};

struct timer_t {
	timer_t(void) : id(INVALID_ID) {}

	quint8 id;
	quint8 resolution;
	quint32 frequency;

	struct configure_t {
		;
	} configure;
};

struct analog_t {
	analog_t(void) : id(INVALID_ID) {}

	quint8 id;
	QString name;
	quint8 resolution;
	struct channel_t {
		channel_t(void) : id(INVALID_ID) {}

		quint8 id;
		QString name;
	};
	QVector<channel_t> channels;
	timer_t timer;

	struct configure_t {
		;
	} configure;
};

#endif // STRUCTURES_H
