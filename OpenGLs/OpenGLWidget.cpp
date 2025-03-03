#include "OpenGLWidget.h"
#include <GL/gl.h>       // OpenGL
#include <GL/glu.h>      // OpenGL Utility Library

OpenGLWidget::OpenGLWidget(QWidget *parent)
    : QOpenGLWidget(parent)
{
    // 初始化一些三维点
    points << QVector3D(0.0f, 0.0f, 0.0f)
           << QVector3D(1.0f, 0.0f, 0.0f)
           << QVector3D(0.0f, 1.0f, 0.0f)
           << QVector3D(0.0f, 0.0f, 1.0f)
           << QVector3D(1.0f, 1.0f, 1.0f);
}

OpenGLWidget::~OpenGLWidget()
{
}

void OpenGLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST); // 启用深度测试
}

void OpenGLWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
}

void OpenGLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 清除颜色和深度缓冲
    glBegin(GL_POINTS);                                 // 开始绘制点
    for (const QVector3D &point : points)
    {
        glVertex3f(point.x(), point.y(), point.z()); // 绘制每个点
    }
    glEnd();
}