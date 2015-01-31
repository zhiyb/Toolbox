#include "analogwaveform.h"

AnalogWaveform::AnalogWaveform(QWidget *parent) : QOpenGLWidget(parent)
{
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	setMinimumSize(480, 480);
}

AnalogWaveform::~AnalogWaveform()
{
}

void AnalogWaveform::setAnalog(analog_t *analog)
{
	this->analog = analog;
	init();
}

void AnalogWaveform::init(void)
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
	if (grid.count != prevCount)
		generateGrid();
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
	grid.vertices.append(QVector2D(0.5, -0.5));
	grid.vertices.append(QVector2D(-0.5, 0.5));
	analog->grid.preference.gridPointSize = analog->grid.displaySize.height() / count.height() / 2 / 20;
}

void AnalogWaveform::initializeGL(void)
{
	initializeOpenGLFunctions();
	glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
	glEnable(GL_POINT_SPRITE);

	if (!(grid.vsh = loadShaderFile(GL_VERTEX_SHADER, "gridvertex.vsh")))
		qFatal(tr("Cannot load grid vertex shader").toLocal8Bit());
	if (!(fsh = loadShaderFile(GL_FRAGMENT_SHADER, "fragment.fsh")))
		qFatal(tr("Cannot load fragment shader").toLocal8Bit());
	program = glCreateProgram();
	//glUseProgram(program);
}

void AnalogWaveform::resizeGL(int w, int h)
{
	init();
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
	glClear(GL_COLOR_BUFFER_BIT);

	// Draw grid
	glAttachShader(program, grid.vsh);
	glAttachShader(program, fsh);
	glLinkProgram(program);
	glUseProgram(program);
	glEnableVertexAttribArray(glGetAttribLocation(program, "vertex"));
	glVertexAttribPointer(glGetAttribLocation(program, "vertex"), 2, GL_FLOAT, GL_TRUE, 0, grid.vertices.constData());
	glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE, projection.constData());
	glUniformMatrix4fv(glGetUniformLocation(program, "modelView"), 1, GL_FALSE, mv.constData());
	glUniform4fv(glGetUniformLocation(program, "colour"), 1, (GLfloat *)&pref.gridColour);
	glUniform1i(glGetUniformLocation(program, "pointSize"), pref.gridPointSize * 3);
	//glPointSize(pref.gridPointSize * 3);
	glDrawArrays(GL_POINTS, 0, grid.largePoints);
	glUniform1i(glGetUniformLocation(program, "pointSize"), pref.gridPointSize);
	//glPointSize(pref.gridPointSize);
	glDrawArrays(GL_POINTS, grid.largePoints, grid.vertices.count() - grid.largePoints);
	glDetachShader(program, grid.vsh);
	glDetachShader(program, fsh);
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
	qWarning(log);
	glDeleteShader(shader);
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
