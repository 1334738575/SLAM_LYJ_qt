// myopenglwidget.cpp
#include "OpenGLWidget2.h"

MyOpenGLWidget::MyOpenGLWidget(QWidget *parent)
    : QOpenGLWidget(parent)
{
    // 设置焦点策略：可通过鼠标/键盘获取焦点
    setFocusPolicy(Qt::StrongFocus);
    // 可选：强制获取焦点（窗口显示时自动聚焦）
    setFocus();
    //// 设置自动更新
    //connect(&m_timer, &QTimer::timeout, this, [this]()
    //        { update(); });
    //m_timer.start(16); // ~60FPS
    m_vertices = m_verticesDefault.data();
    m_vSize = m_verticesDefault.size();
    m_indices = m_indicesDefault.data();
    m_iSize = m_indicesDefault.size();
    std::string sPath = SHADERPATH;
    // 创建 100×100 的 RGBA 格式灰色图像（灰度值可自定义，0=黑，255=白）
    int grayValue = 128; // 中等灰度（0-255 可调）
    m_textureDefault = QImage(100, 100, QImage::Format_RGBA8888); // OpenGL 兼容格式
    // 填充灰色（R=G=B=grayValue，A=255 不透明）
    m_textureDefault.fill(QColor(grayValue, grayValue, grayValue, 255));
    m_texture = m_textureDefault;
}
MyOpenGLWidget::~MyOpenGLWidget()
{
    m_vao.destroy();
    m_vbo.destroy();
    m_ebo.destroy();
}

void MyOpenGLWidget::setVertices(const float* const _vtcs, unsigned long long _sz)
{
    m_vSize = _sz * m_vtxStep;
    m_verticesDefault.assign(m_vSize, -1);
    m_vertices = m_verticesDefault.data();
    for (int i = 0; i < _sz; ++i)
    {
        m_verticesDefault[m_vtxStep * i] = _vtcs[3 * i];
        m_verticesDefault[m_vtxStep * i + 1] = _vtcs[3 * i + 1];
        m_verticesDefault[m_vtxStep * i + 2] = _vtcs[3 * i + 2];
    }
}
void MyOpenGLWidget::setVerticesTexture(const float* const _vtcs, const float* const _uvs, const QImage& _img, unsigned long long _sz)
{
    m_vSize = _sz * m_vtxStep;
    m_verticesDefault.assign(m_vSize, 0);
    m_vertices = m_verticesDefault.data();
    for (int i = 0; i < _sz; ++i)
    {
        m_verticesDefault[m_vtxStep * i] = _vtcs[3 * i];
        m_verticesDefault[m_vtxStep * i + 1] = _vtcs[3 * i + 1];
        m_verticesDefault[m_vtxStep * i + 2] = _vtcs[3 * i + 2];
        m_verticesDefault[m_vtxStep * i + 3] = _uvs[2 * i];
        m_verticesDefault[m_vtxStep * i + 4] = _uvs[2 * i + 1];
    }
    m_texture = _img;
}
void MyOpenGLWidget::setIndices(unsigned int* _inds, unsigned long long _sz)
{
    m_indices = _inds;
    m_iSize = _sz * 3;
}

void MyOpenGLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST); // 开启深度测试

    std::string sPath = SHADERPATH;
    std::string vPath = sPath + "/vertex_shader.vert";
    std::string fPath = sPath +"/fragment_shader.frag";
    QString qvpath = QString::fromStdString(vPath);
    QString qfpath = QString::fromStdString(fPath);
    // 初始化Shader
    m_program.addShaderFromSourceFile(QOpenGLShader::Vertex, qvpath);
    m_program.addShaderFromSourceFile(QOpenGLShader::Fragment, qfpath);
    m_program.link();
    m_program.bind();

    //有VAO的话，VBO和EBO可以不绑定
    m_vao.create();
    m_vao.bind();

    // 初始化顶点缓冲
    m_vbo.create();
    m_vbo.bind();
    m_vbo.allocate(m_vertices, m_vSize * sizeof(float));

    // 配置顶点属性
    m_program.enableAttributeArray(0);
    m_program.setAttributeBuffer(0, GL_FLOAT, 0, m_pointStep, m_vtxStep * sizeof(GLfloat));
    // 3. 配置VAO/VBO，添加纹理坐标属性
    m_program.enableAttributeArray(1);
    m_program.setAttributeBuffer(1, GL_FLOAT, m_pointStep * sizeof(GLfloat), m_uvStep, m_vtxStep * sizeof(GLfloat));
    m_program.setUniformValue("ourTexture", 0); // 告诉着色器采样器用单元0

    //indexs
    m_ebo.create();
    m_ebo.bind();
    m_ebo.allocate(m_indices, m_iSize * sizeof(unsigned int));

    // 初始化视图矩阵
    m_model.setToIdentity();
    m_view.lookAt(QVector3D(0, 0, -3), QVector3D(0, 0, 0), QVector3D(0, 1, 0));
    //QMatrix4x4 QMatrix4x4::lookAt(
    //    const QVector3D & eye,    // 摄像机位置
    //    const QVector3D & center, // 观察目标点
    //    const QVector3D & up     // 定义"上"方向的向量（通常为世界坐标系Y轴）
    //);

    // 2. 加载纹理图片
    glGenTextures(1, &m_textureID);
    glBindTexture(GL_TEXTURE_2D, m_textureID);
    // 加载纹理图片（Qt可使用QImage，OpenGL用stb_image）
    QImage imgOpengl = m_texture.convertToFormat(QImage::Format_RGBA8888).mirrored(false, true);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, imgOpengl.width(), imgOpengl.height(),
        0, GL_RGBA, GL_UNSIGNED_BYTE, imgOpengl.bits());
    glGenerateMipmap(GL_TEXTURE_2D);
    // 7. 设置纹理采样参数（关键！否则纹理全黑）
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenTextures(1, &m_textureIDDefault);
    glBindTexture(GL_TEXTURE_2D, m_textureIDDefault);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_textureDefault.width(), m_textureDefault.height(),
        0, GL_RGBA, GL_UNSIGNED_BYTE, m_textureDefault.bits());
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D, 0);


    m_vao.release();
    m_program.release();

}
void MyOpenGLWidget::paintGL()
{
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_program.bind();
    m_vao.bind();

    //QMatrix4x4 invView = m_view.inverted();
    m_model.setToIdentity();
    m_model.rotate(m_detYRot, QVector3D(0.f, 1.0f, 0.0f));
    m_model.rotate(m_detXRot, QVector3D(1.f, 0.0f, 0.0f));
    m_model(0, 3) = -m_detX;
    m_model(1, 3) = m_detY;
    m_model(2, 3) = m_detZ;

    // 传递矩阵到Shader
    m_program.setUniformValue("model", m_model);
    m_program.setUniformValue("view", m_view);
    m_program.setUniformValue("projection", m_projection);


    glActiveTexture(GL_TEXTURE0); // 激活纹理单元0（默认）
    if(m_bDrawTexture)
        glBindTexture(GL_TEXTURE_2D, m_textureID);
    else
        glBindTexture(GL_TEXTURE_2D, m_textureIDDefault);
    // 绘制立方体线框
    if (m_bDrawVertices)
    {
        glPointSize(10.0f);
        glDrawArrays(GL_POINTS, 0, m_vSize / m_vtxStep);
    }
    if(m_bDrawFaces)
        glDrawElements(GL_TRIANGLES, GLsizei(m_iSize), GL_UNSIGNED_INT, 0);
    //glDrawElements(GL_QUADS, 24, GL_UNSIGNED_INT, 0);
    //glDrawArrays(GL_LINES, 0, 8);
    //glDrawArrays(GL_LINE_LOOP, 0, 4);
    //glDrawArrays(GL_LINE_LOOP, 4, 4);

    glBindTexture(GL_TEXTURE_2D, 0);
    m_vao.release();
    m_program.release();
}
void MyOpenGLWidget::resizeGL(int w, int h)
{
    m_projection.setToIdentity();
    m_projection.perspective(60.0f, w / float(h), 0.1f, 100.0f);
}


void MyOpenGLWidget::mousePressEvent(QMouseEvent* event)
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
void MyOpenGLWidget::mouseMoveEvent(QMouseEvent* event)
{
	if (m_isPressLeft)
	{
		float dx = event->x() - m_lastPos.x();
		float dy = event->y() - m_lastPos.y();
		m_detXRot -= dy;
		m_detYRot += dx;
		m_lastPos = event->pos();
		update();
	}
	else if (m_isPressRight)
	{
		float dx = (event->x() - m_lastPos.x()) / 20.f;
		float dy = (event->y() - m_lastPos.y()) / 20.f;
		m_detY -= dy;
		m_detX += dx;
		m_lastPos = event->pos();
		update();
	}
}
void MyOpenGLWidget::mouseReleaseEvent(QMouseEvent* event)
{
	if (event->button() == Qt::LeftButton)
	{
		m_isPressLeft = false;
		m_isPressRight = false;
	}
}
void MyOpenGLWidget::keyPressEvent(QKeyEvent* event)
{
	switch (event->key())
	{
	case Qt::Key_V:
		m_bDrawVertices = !m_bDrawVertices;
		break;
	case Qt::Key_F:
		m_bDrawFaces = !m_bDrawFaces;
		break;
	case Qt::Key_T:
		m_bDrawTexture = !m_bDrawTexture;
		break;
	//case Qt::Key_Right:
	//	m_detX += 0.1f;
	//	break;
	default:
		break;
	}
	QOpenGLWidget::keyPressEvent(event);
	update();
}
void MyOpenGLWidget::keyReleaseEvent(QKeyEvent* event)
{
	QOpenGLWidget::keyReleaseEvent(event);
}
void MyOpenGLWidget::wheelEvent(QWheelEvent* event)
{
	m_detZ += event->delta() / 100.0f;
	update();
}