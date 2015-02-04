#include "analogwaveform.h"

#define MINIMUM_SIZE_SQUARE	480

AnalogWaveform::AnalogWaveform(QWidget *parent) : QOpenGLWidget(parent)
{
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	setMinimumSize(MINIMUM_SIZE_SQUARE, MINIMUM_SIZE_SQUARE);

	QSurfaceFormat fmt = format();
	fmt.setSamples(8);
	setFormat(fmt);

	stencil.vertices.append(QVector2D(-1, -1));
	stencil.vertices.append(QVector2D(1, -1));
	stencil.vertices.append(QVector2D(1, 1));
	stencil.vertices.append(QVector2D(-1, 1));
}

AnalogWaveform::~AnalogWaveform()
{
}

void AnalogWaveform::setAnalog(analog_t *analog)
{
	this->analog = analog;
	init();
}

bool AnalogWaveform::init(void)
{
	analog_t::grid_t &grid = analog->grid;
	QSize prevCount = grid.count;
	grid.displaySize = size();
	quint32 vSize = grid.displaySize.height() / grid.preferredPointsPerGrid * 2;
	vSize -= vSize % 2;
	if (vSize < grid.minimumVerticalCount)
		vSize = grid.minimumVerticalCount;
	if (vSize > grid.maximumVerticalCount)
		vSize = grid.maximumVerticalCount;
	grid.pointsPerGrid = grid.displaySize.height() / vSize;
	grid.count.setHeight(vSize);
	grid.count.setWidth(grid.displaySize.width() / grid.pointsPerGrid);
	grid.count.setWidth(grid.count.width() - grid.count.width() % 2);
	if (grid.count.width() <= 0)
		grid.count.setWidth(2);
	if (grid.count != prevCount) {
		generateGrid();
		return true;
	}
	return false;
}

void AnalogWaveform::generateGrid(void)
{
	grid.vertices.clear();
	QSize &count = analog->grid.count;
	for (int i = 0; i < count.width(); i++)
		for (int j = 0; j < 5; j++) {
			float x = 2.f * i / count.width() + 2.f / count.width() * j / 5.f - 1.f;
			grid.vertices.append(QVector2D(x, -1.f));
			grid.vertices.append(QVector2D(x, 0.f));
			grid.vertices.append(QVector2D(x, 1.f));
		}
	for (int i = 0; i < count.height(); i++)
		for (int j = 0; j < 5; j++) {
			float y = 2.f * i / count.height() + 2.f / count.height() * j / 5.f - 1.f;
			grid.vertices.append(QVector2D(-1.f, y));
			grid.vertices.append(QVector2D(0.f, y));
			grid.vertices.append(QVector2D(1.f, y));
		}
	grid.vertices.append(QVector2D(1.f, 1.f));
	grid.largePoints = grid.vertices.count();
	for (int i = 0; i < count.width(); i++)
		for (int j = 0; j < 5; j++)
			for (int y = 0; y < count.height(); y++) {
				float x = 2.f * i / count.width() + 2.f / count.width() * j / 5.f - 1.f;
				grid.vertices.append(QVector2D(x, 2.f * y / count.height() - 1.f));
			}
	for (int i = 0; i < count.height(); i++)
		for (int j = 0; j < 5; j++)
			for (int x = 0; x < count.width(); x++) {
				float y = 2.f * i / count.height() + 2.f / count.height() * j / 5.f - 1.f;
				grid.vertices.append(QVector2D(2.f * x / count.width() - 1.f, y));
			}
	analog->grid.preference.gridPointSize = analog->grid.displaySize.height() / count.height() / 2 / 15;
}

void AnalogWaveform::initializeGL(void)
{
	initializeOpenGLFunctions();
	qDebug() << format();
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
	//glEnable(GL_POINT_SPRITE);
	glEnable(GL_MULTISAMPLE);
	//glEnable(GL_LINE_SMOOTH);
	glEnable(GL_STENCIL_TEST);

	try {
		general.vsh = loadShaderFile(GL_VERTEX_SHADER, "vertex.vsh");
		grid.vsh = loadShaderFile(GL_VERTEX_SHADER, "gridvertex.vsh");
		wave.vshYT = loadShaderFile(GL_VERTEX_SHADER, "ytvertex.vsh");
		general.fsh = loadShaderFile(GL_FRAGMENT_SHADER, "fragment.fsh");
	}
	catch (QString str) {
		qFatal(tr("Cannot load shader:\n%1").arg(str).toLocal8Bit());
	}

	try {
		general.program = createProgram(general.vsh, general.fsh);
		grid.program = createProgram(grid.vsh, general.fsh);
		wave.programYT = createProgram(wave.vshYT, general.fsh);//glCreateProgram();
	}
	catch (QString str) {
		qFatal(tr("Cannot create program:\n%1").arg(str).toLocal8Bit());
	}

	general.location.colour = glGetUniformLocation(general.program, "colour");
	general.location.modelView = glGetUniformLocation(general.program, "modelView");
	general.location.projection = glGetUniformLocation(general.program, "projection");
	general.location.vertex = glGetAttribLocation(general.program, "vertex");

	//glUseProgram(grid.program);
	grid.location.colour = glGetUniformLocation(grid.program, "colour");
	grid.location.modelView = glGetUniformLocation(grid.program, "modelView");
	grid.location.pointSize = glGetUniformLocation(grid.program, "pointSize");
	grid.location.projection = glGetUniformLocation(grid.program, "projection");
	grid.location.vertex = glGetAttribLocation(grid.program, "vertex");

	//glUseProgram(wave.programYT);
	wave.locationYT.colour = glGetUniformLocation(wave.programYT, "colour");
	wave.locationYT.data = glGetAttribLocation(wave.programYT, "data");
	wave.locationYT.frequency = glGetUniformLocation(wave.programYT, "frequency");
	wave.locationYT.hCount = glGetUniformLocation(wave.programYT, "hCount");
	wave.locationYT.vCount = glGetUniformLocation(wave.programYT, "vCount");
	wave.locationYT.index = glGetAttribLocation(wave.programYT, "index");
	wave.locationYT.maxValue = glGetUniformLocation(wave.programYT, "maxValue");
	wave.locationYT.modelView = glGetUniformLocation(wave.programYT, "modelView");
	wave.locationYT.projection = glGetUniformLocation(wave.programYT, "projection");
	wave.locationYT.timebase = glGetUniformLocation(wave.programYT, "timebase");
	wave.locationYT.reference = glGetUniformLocation(wave.programYT, "reference");
	wave.locationYT.offset = glGetUniformLocation(wave.programYT, "offset");
	wave.locationYT.scale = glGetUniformLocation(wave.programYT, "scale");
	//qDebug() << wave.programYT << wave.vshYT << fsh << wave.locationYT.colour << wave.locationYT.data << wave.locationYT.frequency << wave.locationYT.hCount << wave.locationYT.index << wave.locationYT.maxValue << wave.locationYT.modelView << wave.locationYT.projection << wave.locationYT.timebase;
}

void AnalogWaveform::resizeGL(int w, int h)
{
	if (init())
		analog->update();
	glViewport(0, 0, w, h);
	QSize count = analog->grid.count;
	float x = (float)w / ((float)h / count.height() * count.width());
	projection = QMatrix4x4();
	projection.ortho(-x, x, -1, 1, -1, 1);
	projection.scale(0.9, 0.9, 0.9);
}

void AnalogWaveform::paintGL(void)
{
	analog_t::grid_t::preference_t &pref = analog->grid.preference;
	glClearColor(pref.bgColour.x(), pref.bgColour.y(), pref.bgColour.z(), pref.bgColour.w());
	QMatrix4x4 mv;
	//mv.scale(0.5, 0.5, 0.5);
	glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	// Draw grid
	glUseProgram(grid.program);
	glEnableVertexAttribArray(grid.location.vertex);
	glVertexAttribPointer(grid.location.vertex, 2, GL_FLOAT, GL_TRUE, 0, grid.vertices.constData());
	glUniformMatrix4fv(grid.location.projection, 1, GL_FALSE, projection.constData());
	glUniformMatrix4fv(grid.location.modelView, 1, GL_FALSE, mv.constData());
	glUniform4fv(grid.location.colour, 1, (GLfloat *)&pref.gridColour);
	glUniform1i(grid.location.pointSize, pref.gridPointSize * 2);
	glDrawArrays(GL_POINTS, 0, grid.largePoints);
	glUniform1i(grid.location.pointSize, pref.gridPointSize);
	glDrawArrays(GL_POINTS, grid.largePoints, grid.vertices.count() - grid.largePoints);
	glDisableVertexAttribArray(grid.location.vertex);

	// Draw waveforms
	updateIndices();
	// Draw stencil
	glUseProgram(general.program);
	glEnableVertexAttribArray(general.location.vertex);
	glVertexAttribPointer(grid.location.vertex, 2, GL_FLOAT, GL_TRUE, 0, stencil.vertices.constData());
	glUniformMatrix4fv(grid.location.projection, 1, GL_FALSE, projection.constData());
	glUniformMatrix4fv(grid.location.modelView, 1, GL_FALSE, mv.constData());
	glUniform4f(grid.location.colour, 1.f, 1.f, 1.f, 1.f);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
	glStencilFunc(GL_NEVER, 1, 0xFF);
	glStencilOp(GL_REPLACE, GL_KEEP, GL_KEEP);
	glStencilMask(0xFF);
	glDrawArrays(GL_TRIANGLE_FAN, 0, stencil.vertices.count());
	glDisableVertexAttribArray(general.location.vertex);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glStencilMask(0x00);
	glStencilFunc(GL_EQUAL, 1, 0xFF);
	// Draw YT waveforms
	glUseProgram(wave.programYT);
	glEnableVertexAttribArray(wave.locationYT.data);
	glEnableVertexAttribArray(wave.locationYT.index);
	glVertexAttribPointer(wave.locationYT.index, 1, GL_INT, GL_TRUE, 0, indices.constData());
	glUniformMatrix4fv(wave.locationYT.projection, 1, GL_FALSE, projection.constData());
	glUniformMatrix4fv(wave.locationYT.modelView, 1, GL_FALSE, mv.constData());
	glUniform1i(wave.locationYT.maxValue, analog->maximum());
	glUniform1i(wave.locationYT.hCount, analog->grid.count.width());
	glUniform1i(wave.locationYT.vCount, analog->grid.count.height());
	glUniform1f(wave.locationYT.timebase, analog->timebase.scale.value());
	glUniform1f(wave.locationYT.frequency, analog->timer.frequency());
	for (int i = 0; i < analog->channels.count(); i++) {
		analog_t::channel_t &channel = analog->channels[i];
		if (channel.enabled) {
			glVertexAttribPointer(wave.locationYT.data, 1, GL_INT, GL_TRUE, 0, channel.buffer.constData());
			glUniform1f(wave.locationYT.reference, channel.reference);
			glUniform1f(wave.locationYT.offset, channel.offset + channel.configure.displayOffset);
			glUniform1f(wave.locationYT.scale, channel.configure.scale.value());
			glUniform4fv(wave.locationYT.colour, 1, (GLfloat *)&channel.configure.colour);
#if 0
			glDrawArrays(GL_LINE_STRIP, 0, analog->buffer.position);
			glDrawArrays(GL_LINE_STRIP, analog->buffer.position, analog->buffer.validSize - analog->buffer.position);
			//glDrawArrays(GL_LINE_STRIP, 0, analog->buffer.sizePerChannel);
#else
			glDrawArrays(GL_POINTS, 0, analog->buffer.validSize);
#endif
		}
	}
	glDisableVertexAttribArray(wave.locationYT.data);
	glDisableVertexAttribArray(wave.locationYT.index);
	glClear(GL_STENCIL_BUFFER_BIT);
}

void AnalogWaveform::updateIndices()
{
	while (indices.count() < (int)analog->buffer.sizePerChannel)
		indices.append(indices.count());
}

GLuint AnalogWaveform::loadShader(GLenum type, const QByteArray& context)
{
	GLuint shader = glCreateShader(type);
	const char *p = context.constData();
	int length = context.length();
	glShaderSource(shader, 1, &p, &length);
	glCompileShader(shader);

	int status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_TRUE)
		return shader;

	int logLength;
	glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
	char log[logLength];
	glGetShaderInfoLog(shader, logLength, &logLength, log);
	glDeleteShader(shader);
	throw QString(log);
	return 0;
}

GLuint AnalogWaveform::loadShaderFile(GLenum type, const char *path)
{
	QFile f(path);
	if (!f.open(QIODevice::ReadOnly)) {
		qWarning(QString("Cannot open file %1").arg(path).toLocal8Bit());
		return 0;
	}
	QByteArray context = f.readAll();
	f.close();
	return loadShader(type, context);
}

GLuint AnalogWaveform::createProgram(GLuint vsh, GLuint fsh)
{
	GLuint program = glCreateProgram();
	glAttachShader(program, vsh);
	glAttachShader(program, fsh);
	glLinkProgram(program);

	int status;
	glGetProgramiv(program, GL_LINK_STATUS, &status);
	if (status == GL_TRUE)
		return program;

	int logLength;
	glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
	char log[logLength];
	glGetProgramInfoLog(program, logLength, &logLength, log);
	glDeleteProgram(program);
	throw QString(log);
	return 0;
}
