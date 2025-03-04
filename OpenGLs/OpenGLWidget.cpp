#include "OpenGLWidget.h"
#include <GL/gl.h>       // OpenGL
#include <GL/glu.h>      // OpenGL Utility Library
#include <QMatrix4x4>

OpenGLWidget::OpenGLWidget(QWidget *parent)
	: QOpenGLWidget(parent),
	m_xRot(0), m_yRot(0), m_isPressed(false), m_lastPos(0, 0), m_scale(-0.1f),
	m_texture(nullptr)
{
    // 初始化一些三维点
    points 
        << QVector3D(0.0f, 0.0f, 0.0f)
           << QVector3D(0.5f, 0.0f, 0.1f)
           << QVector3D(0.0f, 1.0f, 0.0f)
           << QVector3D(0.0f, 0.0f, 1.0f)
           << QVector3D(1.0f, 1.0f, 1.0f);
}

OpenGLWidget::~OpenGLWidget()
{
	delete m_texture;
}

void OpenGLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST); // 启用深度测试
	glEnable(GL_TEXTURE_2D);  // 启用纹理
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // 设置清除颜色为黑色

	// 加载图像并创建纹理
	QImage image("D:/testLyj/build/Release/down.png"); // 替换为你的图像路径
	if (!image.isNull()) {
		image = image.convertToFormat(QImage::Format_RGBA8888); // 确保图像格式
		m_texture = new QOpenGLTexture(image.mirrored()); // 创建纹理
		if (!m_texture->isCreated())
			qDebug() << "Failed to create texture!";
	}
	else 
		qDebug() << "Failed to load image!";
}

void OpenGLWidget::resizeGL(int w, int h)
{
    glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION); // 设置当前矩阵为投影矩阵
	glLoadIdentity();            // 重置当前矩阵
	gluPerspective(0.0f, 0.0f, 1.f, 0.0f); // 设置透视投影
	glMatrixMode(GL_MODELVIEW);  // 设置当前矩阵为模型视图矩阵
}

void OpenGLWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 清除颜色和深度缓冲

	//view
	QMatrix4x4 view;
	view.rotate(m_xRot, 1.0f, 0.0f, 0.0f);
	view.rotate(m_yRot, 0.0f, 1.0f, 0.0f);
	glLoadMatrixf(view.constData());
	glTranslatef(m_scale, 0.0f, -1.f);
	//glRotatef(m_xRot, 1.0f, 0.0f, 0.0f); // 绕X轴旋转
	//glRotatef(m_yRot, 0.0f, 1.0f, 0.0f); // 绕Y轴旋转

	//draw points
    glBegin(GL_POINTS);                                 // 开始绘制点
	glPointSize(10.0f);                                   // 设置点的大小
	glColor3f(1.0f, 0.0f, 0.0f);                        // 设置颜色为红色
    for (const QVector3D &point : points)
    {
        glVertex3f(point.x(), point.y(), point.z()); // 绘制每个点
    }
    glEnd();

	//draw lines
	glBegin(GL_LINES);                                   // 开始绘制线
	glColor3f(0.0f, 1.0f, 0.0f);                        // 设置颜色为绿色
	for (const QVector3D& point : points)
	{
		glVertex3f(0.0f, 0.0f, 0.0f);                  // 绘制从原点到每个点的线
		glVertex3f(point.x(), point.y(), point.z());
	}
	glEnd();

	//draw triangles
	glBegin(GL_TRIANGLES);                               // 开始绘制三角形
	glColor3f(0.0f, 0.0f, 1.0f);                        // 设置颜色为蓝色
	for (const QVector3D& point : points)
	{
		glVertex3f(0.0f, 0.0f, 0.0f);                  // 绘制以原点和每个点为顶点的三角形
		glVertex3f(point.x(), point.y(), point.z());
		glVertex3f(point.y(), point.z(), point.x());
	}
	glEnd();

	//draw quads
	glBegin(GL_QUADS);                                   // 开始绘制四边形
	glColor3f(1.0f, 1.0f, 0.0f);                        // 设置颜色为黄色
	for (const QVector3D& point : points)
	{
		glVertex3f(0.0f, 0.0f, 0.0f);                  // 绘制以原点和每个点为顶点的四边形
		glVertex3f(point.x(), point.y(), point.z());
		glVertex3f(point.y(), point.z(), point.x());
		glVertex3f(point.z(), point.x(), point.y());
	}
	glEnd();

	//draw polygon
	glBegin(GL_POLYGON);                                 // 开始绘制多边形
	glColor3f(1.0f, 0.0f, 1.0f);                        // 设置颜色为紫色
	for (const QVector3D& point : points)
	{
		glVertex3f(point.x(), point.y(), point.z());    // 绘制多边形
	}
	glEnd();

	//draw texture
	if (m_texture) {
		m_texture->bind();
		glBegin(GL_QUADS);
		glColor3f(1.0f, 1.0f, 1.0f);                        // 清除背景色
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, -0.5f, 0.0f);
		glTexCoord2f(1.0f, 0.0f); glVertex3f(0.5f, -0.5f, 0.0f);
		glTexCoord2f(1.0f, 1.0f); glVertex3f(0.5f, 0.5f, 0.0f);
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f, 0.5f, 0.0f);
		glEnd();
	}

}

void OpenGLWidget::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		m_isPressed = true;
		m_lastPos = event->pos();
	}
	//else
	//{
	//	m_isPressed = false;
	//}
}

void OpenGLWidget::mouseMoveEvent(QMouseEvent* event)
{
	if (m_isPressed)
	{
		int dx = event->x() - m_lastPos.x();
		int dy = event->y() - m_lastPos.y();
		m_xRot += dy;
		m_yRot += dx;
		m_lastPos = event->pos();
		update();
	}
}

void OpenGLWidget::mouseReleaseEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		m_isPressed = false;
	}
}

void OpenGLWidget::keyPressEvent(QKeyEvent* event)
{
	switch (event->key())
	{
	case Qt::Key_Up:
		m_scale += 0.1f;
		break;
	case Qt::Key_Down:
		m_scale -= 0.1f;
		break;
	default:
		break;
	}
	update();
}

void OpenGLWidget::keyReleaseEvent(QKeyEvent* event)
{
	QOpenGLWidget::keyReleaseEvent(event);
}

void OpenGLWidget::wheelEvent(QWheelEvent* event)
{
	m_scale += event->delta() / 1200.0f;
	update();
}
