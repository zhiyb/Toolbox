#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <QtWidgets>
#include <inttypes.h>
#include <instructions.h>

#define DEFAULT_CHANNEL_COLOURS	8

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
	void update(void) {value = configure.value;}
	float frequency(void) const {return (float)clockFrequency / (float)value;}
	float frequencyConfigure(void) const {return (float)clockFrequency / (float)configure.value;}

	quint32 clockFrequency;
	quint32 value;

	struct configure_t {
		quint32 value;
	} configure;
};

struct scale_t {
	scale_t(void) : base(10), divide(10), seq(0) {}
	qreal value(void) const {return (qreal)1 / divide * base * factor[seq];}
	QString toString(void) const {return toString(this->value());}
	static QString toString(const qreal value);
	bool decrease(void);
	bool increase(void);

	static const qreal factor[3];
	quint32 base, divide, seq;
};

struct analog_t : public info_t, public resolution_t {
	virtual quint8 type(void) const {return CMD_ANALOG;}
	void init(void);
	bool calculate(void);
	void update(void);
	qreal gridTotalTime(void) {return timebase.scale.value() * (float)grid.count.width();}
	quint32 channelsCount(void) const;
	void setChannelsEnabled(quint32 enabled);
	quint32 channelsEnabledConfigure(void) const;
	quint32 channelsBytes(void) const {return (channels.count() + 7) / 8;}

	QString name;
	quint32 scanFrequency;
	struct channel_t {
		channel_t(void);
		void update(const int bufferSize);
		static const QColor defaultColours[DEFAULT_CHANNEL_COLOURS];

		// Device information
		quint8 id;
		float reference, offset;
		QString name;
		bool enabled;

		// Configure
		struct configure_t {
			bool enabled;
			float displayOffset;
			QVector4D colour;
			scale_t scale;
		} configure;
		QVector<quint32> buffer;
	};
	QVector<channel_t> channels;
	timer_t timer;

	/*struct configure_t {
		;
	} configure;*/

	struct buffer_t {
		// Device information
		quint32 size;

		// Configure
		struct configure_t {
			quint32 sizePerChannel;
		} configure;
		quint32 sizePerChannel, maximunSize;
		quint32 position, validSize;
	} buffer;

	struct grid_t {
		struct preference_t {
			preference_t(void) : bgColour(0.f, 0.f, 0.f, 1.f), \
				gridColour(0.5f, 0.5f, 0.5f, 1.f), gridPointSize(2) {}

			QVector4D bgColour, gridColour;
			int gridPointSize;
		} preference;

		static const quint32 preferredPointsPerGrid;
		static const quint32 minimumPointsPerGrid;
		static const quint32 maximumVerticalCount;
		static const quint32 minimumVerticalCount;

		quint32 pointsPerGrid;
		QSize count, displaySize;
	} grid;

	struct timebase_t {
		void update(void) {scale = configure.scale;}
		bool scanMode(void) {return scale.value() >= 1;}

		scale_t scale;

		struct configure_t {
			bool scanMode(void) {return scale.value() >= 1;}

			scale_t scale;
		} configure;
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
