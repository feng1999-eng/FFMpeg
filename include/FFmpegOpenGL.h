#ifndef FFMPEGOPENGL
#define FFMPEGOPENGL

#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLBuffer>
#include <QOpenGLShader>
#include <QOpenGLWidget>
#include "FFmpegDecoder.h"


class FFmpegOpenGL : public QOpenGLWidget, public QOpenGLFunctions
{
    Q_OBJECT
public:
    FFmpegOpenGL(QWidget* parent = nullptr);
    ~FFmpegOpenGL();

    void setUrl(QString url);

    void startVideo();
    void stop();
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

private:
    //shader³ÌÐò
    QOpenGLShaderProgram m_program;
    QOpenGLBuffer vbo;

    int idY, idU, idV;

    int width, height;

    FFmpegDecoder* decoder;

    uchar* ptr;


};

#endif 