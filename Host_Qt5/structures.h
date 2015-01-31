#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <QtWidgets>
#include <inttypes.h>
#include <instructions.h>

extern quint32 messageCount;

struct message_t {
	message_t(void) : id(INVALID_ID) {sequence = messageCount++;}
	bool similar(const message_t &msg) const;

	char command;
	quint8 id;
	quint32 sequence;
	struct set_t {
		set_t(void) : id(INVALID_ID), bytes(0), value(0) {}

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

struct timer_t : public info_t, public resolution_t {
	virtual quint8 type(void) const {return CMD_TIMER;}

	bool setFrequency(const float freq);
	float frequency(void) const {return (float)clockFrequency / (float)configure.value;}

	quint32 clockFrequency;

	struct configure_t {
		quint32 value;
	} configure;
};

struct scale_t {
	scale_t(void) : base(10), divide(10), seq(0) {}
	qreal value(void) const {return (qreal)1 / divide * base * factor[seq];}
	QString toString(void) const {return toString(this->value());}
	static QString toString(const qreal value);
	void decrease(void);
	void increase(void);

	static const qreal factor[3];
	quint32 base, divide, seq;
};

struct analog_t : public info_t, public resolution_t {
	virtual quint8 type(void) const {return CMD_ANALOG;}
	void init(void);
	void update(void);
	qreal gridTotalTime(void) {return timebase.scale.value() * (float)grid.count.width();}
	quint32 channelCount(void) const;
	quint32 channelEnabled(void) const;
	quint32 channelEnabledBytes(void) const {return (channels.count() + 7) / 8;}

	QString name;
	quint32 scanFrequency;
	struct channel_t {
		channel_t(void) : id(INVALID_ID), enabled(true) {}

		quint8 id;
		bool enabled;
		QString name;
		QVector<quint32> buffer;
		scale_t scale;
	};
	QVector<channel_t> channels;
	timer_t timer;

	struct configure_t {
		;//bool scanMode;
	} configure;

	struct buffer_t {
		quint32 sizePerChannel, maximunSize;
		quint32 position, validSize;
	} buffer;

	struct grid_t {
		struct preference_t {
			preference_t(void);

			QVector4D bgColour, gridColour;
			int gridPointSize;
		} preference;

		static const quint32 preferredPointsPerGrid;
		static const quint32 minimumVerticalCount;
		static const quint32 maximumVerticalCount;

		quint32 pointsPerGrid;
		QSize count, displaySize;
		//quint32 horizontalCount, verticalCount;
	} grid;

	struct timebase_t {
		bool scanMode(void) {return scale.value() >= 1;}

		scale_t scale;
	} timebase;

	struct trigger_t {
		;
	} trigger;

	struct data_t {
		data_t(void) : id(INVALID_ID) {}

		quint8 id ,type;
		QVector<quint32> data;
	};
};

#endif // STRUCTURES_H
