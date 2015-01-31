#ifndef ANALOGWAVEFORM_H
#define ANALOGWAVEFORM_H

#include <QtWidgets>
#include "structures.h"

class AnalogWaveform : public QOpenGLWidget, protected QOpenGLFunctions
{
	Q_OBJECT
public:
	AnalogWaveform(QWidget *parent);
	~AnalogWaveform();
	void setAnalog(analog_t *analog);

protected:
	void initializeGL();
	void resizeGL(int w, int h);
	void paintGL();

private:
	void updateIndices(void);

	GLuint loadShader(GLenum type, const QByteArray& context);
	GLuint loadShaderFile(GLenum type, const char *path);
	GLuint createProgram(GLuint vsh, GLuint fsh);
	GLuint fsh;
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
			GLuint data, index;		// attribute int
			GLuint maxValue, hCount;	// uniform int
			GLuint timebase, frequency;	// uniform float
			GLuint projection, modelView;	// uniform mat4
			GLuint colour;			// uniform vec4
		} locationYT;

		GLuint programYT, vshYT;
	} wave;

	QMatrix4x4 projection;
	QVector<GLuint> indices;

	void init(void);
	void generateGrid(void);

	analog_t *analog;
};

#endif // ANALOGWAVEFORM_H
