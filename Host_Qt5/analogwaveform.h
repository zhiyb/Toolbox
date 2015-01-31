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
	void paintGL();
	void resizeGL(int w, int h);

private:
	GLuint loadShader(GLenum type, const QByteArray& context);
	GLuint loadShaderFile(GLenum type, const char *path);
	GLuint program, fsh;
	struct grid_t {
		GLuint vsh;
		int largePoints;
		QVector<QVector2D> vertices;
	} grid;

	QMatrix4x4 projection;

	void init(void);
	void generateGrid(void);

	analog_t *analog;
};

#endif // ANALOGWAVEFORM_H
