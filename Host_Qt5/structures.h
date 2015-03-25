#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <QtWidgets>
#include <inttypes.h>
#include <instructions.h>
#include "conv.h"

#define DEFAULT_CHANNEL_COLOURS	8

extern quint32 messageCount;

struct message_t {
	message_t(void) : id(INVALID_ID) {sequence = messageCount++;}
	message_t(char cmd, quint8 id) : command(cmd), id(id) {sequence = messageCount++;}
	bool similar(const message_t &msg) const;

	struct update_t {
		update_t(void) : id(INVALID_ID) {}
		quint8 id;
	} update;

	char command;
	quint8 id;
	quint32 sequence;
	struct set_t {
		set_t(void) : id(INVALID_ID), bytes(0), value(0) {}
		set_t(quint8 id, quint8 bytes, quint32 value) : id(id), bytes(bytes), value(value) {}

		quint8 id, bytes;
		quint32 value;
	};
	QQueue<set_t> settings;
};

struct info_t {
	info_t(void) : id(INVALID_ID) {}
	virtual ~info_t(void) {}
	virtual quint8 type(void) const {return INVALID_ID;}

	quint8 id;
};

struct device_t {
	quint32 version;
	QString name;
};

struct controller_t : public info_t {
	virtual quint8 type(void) const {return CMD_CONTROLLER;}

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

struct resolution_t {
	quint8 bytes(void) const {return (resolution + 7) / 8;}
	quint32 maximum(void) const {return ((quint64)1 << resolution) - 1;}

	quint8 resolution;
};

struct hwtimer_t : public info_t, public resolution_t {
	virtual quint8 type(void) const {return CMD_TIMER;}

	bool setFrequency(const qreal freq);
	void update(void) {value = configure.value;}
	float frequency(bool conf = false) const {return (float)clockFrequency / (float)(conf ? configure.value : value);}

	quint32 clockFrequency;
	quint32 value;

	struct configure_t {
		quint32 value;
	} configure;
};

struct scale_t {
	scale_t(void) : base(10), divide(10), seq(0) {}
	qreal value(void) const {return (qreal)1.f / (qreal)divide * (qreal)base * (qreal)factor[seq];}
	QString toString(void) const {return toString(this->value());}
	static QString toString(const qreal value);
	bool decrease(void);
	bool increase(void);
	bool operator==(const scale_t &s) const {return base == s.base && divide == s.divide && seq == s.seq;}
	bool operator!=(const scale_t &s) const {return !(*this == s);}

	static const qreal factor[3];
	quint32 base, divide, seq;
};

struct colour_t : QVector4D {
	colour_t(const QVector4D &clr = QVector4D()) : QVector4D(clr) {}
	colour_t(const QColor &clr) {*this = clr;}
	colour_t& operator=(const QColor &clr) {return *this = conv::colorToVector4D(clr);}
	operator QColor(void) {return conv::vector4DToColor(*this);}
};

struct analog_t : public info_t, public resolution_t {
	virtual quint8 type(void) const {return CMD_ANALOG;}
	void init(void);
	bool calculate(void);
	void update(void);
	qreal gridTotalTime(void) {return timebase.scale.value() * (qreal)grid.count.width();}
	qreal gridTotalTimeConfigure(void) {return timebase.configure.scale.value() * (qreal)grid.count.width();}
	quint32 channelsCount(bool conf = false) const;
	bool channelEnabled(int i, bool conf = false) const {return conf ? (channels.at(i).configure.enabled || channels.at(i).id == trigger.configure.source) : (channels.at(i).enabled || channels.at(i).id == trigger.source);}
	void setChannelsEnabled(quint32 enabled);
	quint32 channelsEnabled(bool conf = false) const;
	quint32 channelsBytes(void) const {return (channels.count() + 7) / 8;}
	bool scanMode(bool conf = false) const {return timer.frequency(conf) * channelsCount(conf) < scanFrequency;}
	bool updateRequired(void) const;

	QString name;
	quint32 scanFrequency, maxFrequency;

	struct channel_t {
		channel_t(void);
		void update(const int bufferSize);
		static const QColor defaultColours[DEFAULT_CHANNEL_COLOURS];

		// Device information
		quint8 id;
		float reference, offset;
		QString name;
		bool enabled;

		// Buffer space
		QVector<quint32> buffer;

		// Configure
		struct configure_t {
			bool enabled;
			qreal displayOffset;
			colour_t colour;
			scale_t scale;
		} configure;
	};
	QVector<channel_t> channels;
	channel_t *findChannel(quint8 id);
	const channel_t *findChannel(quint8 id) const;

	hwtimer_t timer;

	struct buffer_t {
		void reset(void);
		bool updateRequired(void) const {return sizePerChannel != configure.sizePerChannel;}

		// Device information
		quint32 size;

		// Configure
		quint32 sizePerChannel;

		// Current
		quint32 position, validSize;

		struct configure_t {
			quint32 sizePerChannel;
		} configure;
	} buffer;

	struct grid_t {
		grid_t(void) : gridPointSize(2), bgColour(Qt::black), gridColour(Qt::gray) {}

		static const quint32 preferredPointsPerGrid;
		static const quint32 minimumPointsPerGrid;
		static const quint32 maximumVerticalCount;
		static const quint32 minimumVerticalCount;

		quint32 pointsPerGrid;
		QSize count, displaySize;

		// Preference
		int gridPointSize;
		colour_t bgColour, gridColour;
	} grid;

	struct timebase_t {
		void update(void) {scale = configure.scale;}
		bool updateRequired(void) const {return scale != configure.scale;}

		scale_t scale;

		struct configure_t {
			scale_t scale;
		} configure;
	} timebase;

	struct trigger_t {
		trigger_t(void) : view(false) {update();}
		void update(void);
		bool enabled(void) const {return source != INVALID_ID;}
		bool updateRequired(void) const;

		bool view;
		quint8 source;
		quint32 level;
		qint32 position;

		struct configure_t {
			configure_t(void) : source(INVALID_ID), dispLevel(0), dispPosition(0), colour(Qt::blue) {}
			bool enabled(void) const {return source != INVALID_ID;}

			quint8 source;
			qreal dispLevel, dispPosition;
			colour_t colour;

			// Cache
			qint32 level, position;
		} configure;
	} trigger;

	struct data_t {
		data_t(void) : id(INVALID_ID) {}

		quint8 id ,type;
		QVector<quint32> data;
	};
};

#endif // STRUCTURES_H
