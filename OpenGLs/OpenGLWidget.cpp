#include "OpenGLWidget.h"
#include <GL/gl.h>       // OpenGL
#include <GL/glu.h>      // OpenGL Utility Library
#include <QMatrix4x4>
#include <iostream>



OpenGLWidgetLyj::OpenGLWidgetLyj(QWidget* parent)
	: QOpenGLWidget(parent),
	m_xRot(0), m_yRot(0), m_detX(0), m_detY(0), m_detZ(0),
	m_isPressLeft(false), m_isPressRight(false), m_lastPos(0, 0), m_scale(1.f)
{
	setFocusPolicy(Qt::StrongFocus); // 设置焦点策略，可以接收键盘事件
}

OpenGLWidgetLyj::~OpenGLWidgetLyj()
{
}

void OpenGLWidgetLyj::addPoint(const float x, const float y, const float z)
{
	m_points.push_back(QVector3D(x, y, z));
	update();
}

void OpenGLWidgetLyj::addPoint(const QVector3D& p)
{
	m_points.push_back(p);
	update();
}

void OpenGLWidgetLyj::addLine(const int p1, const int p2)
{
	m_lines.emplace_back(p1, p2);
	update();
}

void OpenGLWidgetLyj::addTriangle(const int p1, const int p2, const int p3)
{
	m_triangles.emplace_back(p1, p2, p3);
	update();
}

void OpenGLWidgetLyj::addQuad(const int p1, const int p2, const int p3, const int p4)
{
	m_quads.emplace_back(p1, p2, p3, p4);
	update();
}

void OpenGLWidgetLyj::addPolygon(const std::vector<int>& points)
{
	m_polygons.push_back(points);
	update();
}

void OpenGLWidgetLyj::setPoints(const float* points, const int size)
{
	m_points.resize(size);
	for (int i = 0; i < size; ++i)
	{
		m_points[i] = QVector3D(points[i * 3], points[i * 3 + 1], points[i * 3 + 2]);
	}
	update();
}

void OpenGLWidgetLyj::setPoints(const std::vector<QVector3D>& ps)
{
	m_points = ps;
	update();
}

void OpenGLWidgetLyj::setLines(const int* lines, const int size)
{
	m_lines.resize(size);
	for (int i = 0; i < size; ++i)
	{
		m_lines[i] = LineInds(lines[i * 2], lines[i * 2 + 1]);
	}
	update();
}

void OpenGLWidgetLyj::setTriangles(const int* triangles, const int size)
{
	m_triangles.resize(size);
	for (int i = 0; i < size; ++i)
	{
		m_triangles[i] = TriangleInds(triangles[i * 3], triangles[i * 3 + 1], triangles[i * 3 + 2]);
	}
	update();
}

void OpenGLWidgetLyj::setQuads(const int* quads, const int size)
{
	m_quads.resize(size);
	for (int i = 0; i < size; ++i)
	{
		m_quads[i] = QuadInds(quads[i * 4], quads[i * 4 + 1], quads[i * 4 + 2], quads[i * 4 + 3]);
	}
	update();
}

void OpenGLWidgetLyj::setPolygons(const std::vector<std::vector<int>>& polygons)
{
	m_polygons = polygons;
	update();
}

void OpenGLWidgetLyj::initializeGL()
{
	initializeOpenGLFunctions();
	glEnable(GL_DEPTH_TEST); // 启用深度测试
	//glEnable(GL_TEXTURE_2D);  // 启用纹理
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // 设置清除颜色为黑色
}

void OpenGLWidgetLyj::resizeGL(int w, int h)
{
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION); // 设置当前矩阵为投影矩阵
	glLoadIdentity();            // 重置当前矩阵
	gluPerspective(0.0f, 0.0f, 1.f, 0.0f); // 设置透视投影
	glMatrixMode(GL_MODELVIEW);  // 设置当前矩阵为模型视图矩阵
}

void OpenGLWidgetLyj::paintGL()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // 清除颜色和深度缓冲

	glLoadIdentity();

	//view
	QMatrix4x4 view;
	view.scale(m_scale);
	view.translate(m_detX, m_detY, m_detZ);
	view.rotate(m_xRot, 1.0f, 0.0f, 0.0f);
	view.rotate(m_yRot, 0.0f, 1.0f, 0.0f);
	glLoadMatrixf(view.constData());
	//glTranslatef(m_detX * m_scale, m_detY * m_scale, m_detZ * m_scale);
	//glRotatef(m_xRot, 1.0f, 0.0f, 0.0f); // 绕X轴旋转
	//glRotatef(m_yRot, 0.0f, 1.0f, 0.0f); // 绕Y轴旋转

	//draw points
	if (m_enablePoint) {
		glBegin(GL_POINTS);                                 // 开始绘制点
		glPointSize(10.0f);                                   // 设置点的大小
		glColor3f(1.0f, 0.0f, 0.0f);                        // 设置颜色为红色
		for (const QVector3D& point : m_points)
		{
			glVertex3f(point.x(), point.y(), point.z()); // 绘制每个点
		}
		glEnd();
	}

	//draw lines
	if (m_enableLine) {
		glBegin(GL_LINES);                                   // 开始绘制线
		glColor3f(0.0f, 1.0f, 0.0f);                        // 设置颜色为绿色
		for (const auto& line : m_lines)
		{
			const auto& p1 = m_points[line[0]];
			const auto& p2 = m_points[line[1]];
			glVertex3f(p1.x(), p1.y(), p1.z());
			glVertex3f(p2.x(), p2.y(), p2.z());
		}
		glEnd();
	}

	//draw triangles
	if (m_enableTriangle) {
		glBegin(GL_TRIANGLES);                               // 开始绘制三角形
		glColor3f(0.0f, 0.0f, 1.0f);                        // 设置颜色为蓝色
		for (const auto& triangle: m_triangles)
		{
			const auto& p1 = m_points[triangle[0]];
			const auto& p2 = m_points[triangle[1]];
			const auto& p3 = m_points[triangle[2]];
			glVertex3f(p1.x(), p1.y(), p1.z());
			glVertex3f(p2.x(), p2.y(), p2.z());
			glVertex3f(p3.x(), p3.y(), p3.z());
		}
		glEnd();
	}

	//draw quads
	if (m_enableQuad) {
		glBegin(GL_QUADS);                                   // 开始绘制四边形
		glColor3f(1.0f, 1.0f, 0.0f);                        // 设置颜色为黄色
		for (const auto& quad: m_quads)
		{
			const auto& p1 = m_points[quad[0]];
			const auto& p2 = m_points[quad[1]];
			const auto& p3 = m_points[quad[2]];
			const auto& p4 = m_points[quad[3]];
			glVertex3f(p1.x(), p1.y(), p1.z());
			glVertex3f(p2.x(), p2.y(), p2.z());
			glVertex3f(p3.x(), p3.y(), p3.z());
			glVertex3f(p4.x(), p4.y(), p4.z());
		}
		glEnd();
	}

	//draw polygon
	if (m_enablePolygon) {
		for (const auto& polygon: m_polygons)
		{
			glBegin(GL_POLYGON);                                 // 开始绘制多边形
			glColor3f(1.0f, 0.0f, 1.0f);                        // 设置颜色为紫色
			for (const auto& index : polygon)
			{
				const auto& point = m_points[index];
				glVertex3f(point.x(), point.y(), point.z());    // 绘制多边形
			}
			glEnd();
		}
	}

}

void OpenGLWidgetLyj::mousePressEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		m_isPressLeft = true;
		m_lastPos = event->pos();
	}
	else if (event->button() == Qt::RightButton)
	{
		m_isPressRight = true;
		m_lastPos = event->pos();
	}
	else
	{
		m_isPressLeft = false;
		m_isPressRight = false;
	}
}

void OpenGLWidgetLyj::mouseMoveEvent(QMouseEvent* event)
{
	if (m_isPressLeft)
	{
		float dx = event->x() - m_lastPos.x();
		float dy = event->y() - m_lastPos.y();
		m_xRot += dy;
		m_yRot += dx;
		m_lastPos = event->pos();
		update();
	}
	else if (m_isPressRight)
	{
		float dx = (event->x() - m_lastPos.x()) / 200.f;
		float dy = (event->y() - m_lastPos.y()) / 200.f;
		m_detY -= dy;
		m_detX += dx;
		m_lastPos = event->pos();
		update();
	}
}

void OpenGLWidgetLyj::mouseReleaseEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		m_isPressLeft = false;
		m_isPressRight = false;
	}
}

void OpenGLWidgetLyj::keyPressEvent(QKeyEvent* event)
{
	switch (event->key())
	{
	case Qt::Key_Up:
		m_detY += 0.1f;
		break;
	case Qt::Key_Down:
		m_detY -= 0.1f;
		break;
	case Qt::Key_Left:
		m_detX -= 0.1f;
		break;
	case Qt::Key_Right:
		m_detX += 0.1f;
		break;
	case Qt::Key_L:
		m_enableLine = !m_enableLine;
		break;
	case Qt::Key_T:
		m_enableTriangle = !m_enableTriangle;
		break;
	case Qt::Key_Q:
		m_enableQuad = !m_enableQuad;
		break;
	case Qt::Key_P:
		m_enablePolygon = !m_enablePolygon;
		break;
	default:
		break;
	}
	QOpenGLWidget::keyPressEvent(event);
	update();
}

void OpenGLWidgetLyj::keyReleaseEvent(QKeyEvent* event)
{
	QOpenGLWidget::keyReleaseEvent(event);
}

void OpenGLWidgetLyj::wheelEvent(QWheelEvent* event)
{
	// 获取滚轮的移动量
	int delta = event->angleDelta().y();
	if (delta > 0) {
		m_scale *= 0.9f;  // 放大
	}
	else {
		m_scale *= 1.1f;  // 缩小
	}

	//// 触发重新绘制
	//update();
	//m_scale += event->delta() / 1000.0f;
	update();
}









/***********************************test***************************************/

OpenGLWidget::OpenGLWidget(QWidget *parent)
	: QOpenGLWidget(parent),
	m_xRot(0), m_yRot(0), m_detX(0), m_detY(0), m_detZ(0),
	m_isPressLeft(false), m_isPressRight(false), m_lastPos(0, 0), m_scale(1.f),
	m_texture(nullptr)
{
    // 初始化一些三维点
    points 
        << QVector3D(0.0f, 0.0f, 0.0f)
           << QVector3D(0.5f, 0.0f, 0.1f)
           << QVector3D(0.0f, 1.0f, 0.0f)
           << QVector3D(0.0f, 0.0f, 1.0f)
           << QVector3D(1.0f, 1.0f, 1.0f);
	setFocusPolicy(Qt::StrongFocus); // 设置焦点策略，可以接收键盘事件
}

OpenGLWidget::~OpenGLWidget()
{
	delete m_texture;
}

void OpenGLWidget::addMove(float x, float y, float z)
{
	m_detX += x;
	m_detY += y;
	m_detZ += z;
	update();
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
	glTranslatef(m_detX * m_scale, m_detY * m_scale, m_detZ * m_scale);
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
		m_isPressLeft = true;
		m_lastPos = event->pos();
	}
	else if (event->button() == Qt::RightButton)
	{
		m_isPressRight = true;
		m_lastPos = event->pos();
	}
	else
	{
		m_isPressLeft = false;
		m_isPressRight = false;
	}
}

void OpenGLWidget::mouseMoveEvent(QMouseEvent* event)
{
	if (m_isPressLeft)
	{
		float dx = event->x() - m_lastPos.x();
		float dy = event->y() - m_lastPos.y();
		m_xRot += dy;
		m_yRot += dx;
		m_lastPos = event->pos();
		update();
	}
	else if (m_isPressRight)
	{
		float dx = (event->x() - m_lastPos.x()) / 200.f;
		float dy = (event->y() - m_lastPos.y()) / 200.f;
		m_detY -= dy;
		m_detX += dx;
		m_lastPos = event->pos();
		update();
	}
}

void OpenGLWidget::mouseReleaseEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		m_isPressLeft = false;
		m_isPressRight = false;
	}
}

void OpenGLWidget::keyPressEvent(QKeyEvent* event)
{
	switch (event->key())
	{
	case Qt::Key_Up:
		m_detY += 0.1f;
		break;
	case Qt::Key_Down:
		m_detY -= 0.1f;
		break;
	case Qt::Key_Left:
		m_detX -= 0.1f;
		break;
	case Qt::Key_Right:
		m_detX += 0.1f;
		break;
	default:
		break;
	}
	QOpenGLWidget::keyPressEvent(event);
	update();
}

void OpenGLWidget::keyReleaseEvent(QKeyEvent* event)
{
	QOpenGLWidget::keyReleaseEvent(event);
}

void OpenGLWidget::wheelEvent(QWheelEvent* event)
{
	m_scale += event->delta() / 1000.0f;
	update();
}
