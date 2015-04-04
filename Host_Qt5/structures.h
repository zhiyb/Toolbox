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

	hwtimer_t(void) : clockFrequency(0), value(0) {}
	bool setFrequency(const qreal freq);
	void update(void) {value = configure.value;}
	float frequency(bool conf = false) const {return (float)clockFrequency / (float)(conf ? configure.value : value);}

	quint32 clockFrequency;
	quint32 value;

	struct configure_t {
		configure_t(void) : value(0) {}

		quint32 value;
	} configure;
};

struct scale_t {
	scale_t(void) : base(10), divide(10), seq(0) {}
	qreal value(void) const {return (qreal)1.f / (qreal)divide * (qreal)base * (qreal)factor[seq];}
	QString toString(void) const {return toString(this->value());}
	static QString toString(const qreal value, bool prec = false);
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

	struct channel_t;
	struct data_t;

	void init(void);
	bool calculate(void);
	bool updateRequired(void) const;
	void update(void);
	void updateBufferInfo(void);

	bool scanMode(bool conf = false) const {return timer.frequency(conf) * channelsCount(conf) < scanFrequency;}

	channel_t *findChannel(quint8 id);
	const channel_t *findChannel(quint8 id) const;
	int findChannelIndex(quint8 id) const;
	quint32 channelsCount(bool conf = false) const;
	bool channelEnabled(int i, bool conf = false) const {return conf ? (channel.at(i).configure.enabled() || channel.at(i).id == trigger.configure.source) : (channel.at(i).enabled || channel.at(i).id == trigger.source);}
	void setChannelsEnabled(quint32 enabled);
	quint32 channelsEnabled(bool conf = false, bool chOnly = false) const;
	quint32 channelsBytes(void) const {return (channel.count() + 7) / 8;}

	qreal gridTotalTime(bool conf = false) const {return (qreal)grid.count.width() * timebase.value(conf);}
	qreal gridTotalVoltage(const channel_t *ch) const {return (qreal)grid.count.height() * ch->configure.scale.value();}
	qreal gridTotalVoltage(quint8 id) const {return gridTotalVoltage(findChannel(id));}
	// Convert screen x-coordinate to buffer index
	int screenXToIndex(qreal x, bool conf = false) const;
	// Convert buffer index to screen x-coordinate
	qreal indexToScreenX(int cnt, bool conf = false) const;

	bool triggerValid(bool conf = false) const;
	channel_t *triggerChannel(bool conf = false) {return findChannel(trigger.channel(conf));}
	const channel_t *triggerChannel(bool conf = false) const {return findChannel(trigger.channel(conf));}
	int triggerChannelIndex(bool conf = false) const {return findChannelIndex(trigger.channel(conf));}
	bool dataHandler(const data_t &data);

	QString name;
	quint32 scanFrequency, maxFrequency;

	struct channel_t {
		channel_t(void);
		void resetBufferInfo(void);
		void update(const int bufferSize);
		void updateMode(void) {updateMode(enabled);}
		void updateMode(const bool e);
		qreal totalOffset(void) const {return (configure.mode == configure_t::AC ? 0 : offset) + configure.displayOffset;}
		qreal adcToVoltage(const qint32 adc) const;
		qint32 voltageToADC(const qreal voltage) const;
		qreal voltageToScreenY(const qreal voltage) const;
		qreal screenYToVoltage(const qreal y) const;
		qreal adcToScreenY(const qint32 adc) const {return voltageToScreenY(adcToVoltage(adc));}
		qint32 screenYToADC(const qreal y) const {return voltageToADC(screenYToVoltage(y));}

		static const QColor defaultColours[DEFAULT_CHANNEL_COLOURS];

		// Device information
		quint8 id;
		float reference, offset;
		QString name;
		bool enabled;
		analog_t *analog;

		// Buffer space
		QVector<qint32> buffer;
		struct bufferInfo_t {
			qreal mean;
			qint32 min, max;
		} bufferInfo;

		// Configure
		struct configure_t {
			configure_t(void) : mode(DC), displayOffset(0) {}
			bool enabled(void) const {return mode != Off;}

			enum Modes {Off = 0, DC, AC} mode;
			qreal displayOffset;
			colour_t colour;
			scale_t scale;
		} configure;
	};
	QVector<channel_t> channel;

	hwtimer_t timer;

	struct buffer_t {
		buffer_t(void) : size(0), sizePerChannel(0) {reset();}
		void reset(void);
		bool updateRequired(void) const {return sizePerChannel != configure.sizePerChannel;}

		// Device information
		quint32 size;

		quint32 sizePerChannel;
		quint32 position, validSize;

		struct configure_t {
			configure_t(void) : sizePerChannel(0) {}

			quint32 sizePerChannel;
		} configure;
	} buffer;

	struct grid_t {
		grid_t(void) : pointsPerGrid(0), gridPointSize(2), bgColour(Qt::black), gridColour(Qt::gray) {}

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
		float value(bool conf = false) const {return conf ? configure.scale.value() : scale.value();}

		scale_t scale;

		struct configure_t {
			scale_t scale;
		} configure;
	} timebase;

	struct trigger_t {
		trigger_t(void) : view(false), source(INVALID_ID), edge(Rising), level(0), position(0) {update();}
		quint8 channel(bool conf = false) const {return conf ? configure.source : source;}
		bool enabled(bool conf = false) const {return channel(conf) != INVALID_ID;}
		bool sourceUpdateRequired(void) const {return source != configure.source;}
		bool settingsUpdateRequired(void) const;
		void update(void);
		void reset(void);

		bool view;
		quint8 source;
		enum Edge {Rising = 0, Falling} edge;
		quint32 level;
		qint32 position;

		struct state_t {
			state_t(void) : status(Pre), bufferSize(0), position(0), bufferIndex(-1) {}
			void shiftBuffer(void);
			quint32 currentData(void);

			enum Status {Pre = 0, Waiting, Post, Done} status;
			quint32 bufferSize;
			qint32 position;
			int bufferIndex;
			struct buffer_t {
				buffer_t(void) : enabled(false) {}

				bool enabled;
				QVector<qint32> buffer;
			};
			QVector<buffer_t> buffer;
		} state;

		void resetBuffer(const int size);
		bool beforeEdge(void);
		state_t::Status dataHandler(const QVector<quint32> &data);

		struct configure_t {
			configure_t(void) : source(INVALID_ID), edge(Rising), dispLevel(0), dispPosition(0), colour(Qt::blue), level(0), position(0) {}

			quint8 source;
			Edge edge;
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
