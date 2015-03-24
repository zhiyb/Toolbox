#ifndef ANALOGWAVEFORM_H
#define ANALOGWAVEFORM_H

#include <QtWidgets>
#include "structures.h"
#include "device.h"

class AnalogWaveform : public QOpenGLWidget, protected QOpenGLFunctions
{
	Q_OBJECT
public:
	AnalogWaveform(Device *dev, QWidget *parent);
	~AnalogWaveform();
	void setAnalog(analog_t *analog);

signals:
	void updateAt(quint32 sequence);

protected:
	void initializeGL();
	void resizeGL(int w, int h);
	void paintGL();

private:
	void updateIndices(void);

	GLuint loadShader(GLenum type, const QByteArray& context);
	GLuint loadShaderFile(GLenum type, const char *path);
	GLuint createProgram(GLuint vsh, GLuint fsh);
	struct general_t {
		struct lcoation_t {
			GLuint vertex;			// attribute vec2
			GLuint projection, modelView;	// uniform mat4
			GLuint colour;			// uniform vec4
		} location;
		GLuint program, vsh, fsh;
	} general;
	struct grid_t {
		struct location_t {
			GLuint vertex;			// attribute vec2
			GLuint pointSize;		// uniform int
			GLuint projection, modelView;	// uniform mat4
			GLuint colour;			// uniform vec4
		} location;

		GLuint program, vsh;
		int largePoints;
		QVector<QVector2D> vertices;
	} grid;
	struct wave_t {
		struct location_t {
			GLuint data, index;					// attribute int
			GLuint hCount, vCount, maxValue;			// uniform int
			GLuint timebase, frequency, reference, offset, scale;	// uniform float
			GLuint projection, modelView;				// uniform mat4
			GLuint colour;						// uniform vec4
		} locationYT;

		GLuint programYT, vshYT;
	} wave;
	struct stencil_t {
		QVector<QVector2D> vertices;
	} stencil;

	QMatrix4x4 projection;
	QVector<GLfloat> indices;

	bool init(void);
	void generateGrid(void);

	Device *dev;
	analog_t *analog;
};

#endif // ANALOGWAVEFORM_H
