// myopenglwidget.cpp
#include "OpenGLWidget2.h"

MyOpenGLWidget::MyOpenGLWidget(QWidget *parent)
    : QOpenGLWidget(parent)
{
    // 设置自动更新
    connect(&m_timer, &QTimer::timeout, this, [this]()
            { update(); });
    m_timer.start(16); // ~60FPS
}

MyOpenGLWidget::~MyOpenGLWidget()
{
    m_vao.destroy();
    m_vbo.destroy();
    m_ebo.destroy();
}

void MyOpenGLWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST); // 开启深度测试

    // 初始化Shader
    m_program.addShaderFromSourceFile(QOpenGLShader::Vertex, "D:/testLyj/QT/shaders/vertex_shader.vert");
    m_program.addShaderFromSourceFile(QOpenGLShader::Fragment, "D:/testLyj/QT/shaders/fragment_shader.frag");
    m_program.link();
    m_program.bind();

    //有VAO的话，VBO和EBO可以不绑定
    m_vao.create();
    m_vao.bind();

    // 初始化顶点缓冲
    m_vbo.create();
    m_vbo.bind();
    m_vbo.allocate(m_vertices, sizeof(m_vertices));

    // 配置顶点属性
    m_program.enableAttributeArray(0);
    m_program.setAttributeBuffer(0, GL_FLOAT, 0, 3, 3 * sizeof(GLfloat));

    //indexs
    m_ebo.create();
    m_ebo.bind();
	GLuint indices[] = {
		0, 1, 2, 3, // 底面
		4, 5, 6, 7, // 顶面
		0, 1, 5, 4, // 左侧面
		2, 3, 7, 6, // 右侧面
		0, 3, 7, 4, // 前侧面
		1, 2, 6, 5 }; // 后侧面
    m_ebo.allocate(indices, sizeof(indices));

    // 初始化视图矩阵
    m_view.lookAt(QVector3D(0, 0, 3), QVector3D(0, 0, 0), QVector3D(0, 1, 0));
    //QMatrix4x4 QMatrix4x4::lookAt(
    //    const QVector3D & eye,    // 摄像机位置
    //    const QVector3D & center, // 观察目标点
    //    const QVector3D & up     // 定义"上"方向的向量（通常为世界坐标系Y轴）
    //);

    //m_vbo.release();
    //m_ebo.release();
    m_vao.release();
    m_program.release();
}

void MyOpenGLWidget::paintGL()
{
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    m_program.bind();
    m_vao.bind();

    // 更新模型矩阵（旋转）
    m_angle += 1.0f;
    m_model.setToIdentity();
    m_model.rotate(m_angle, QVector3D(0.5f, 1.0f, 0.0f));

    // 传递矩阵到Shader
    m_program.setUniformValue("model", m_model);
    m_program.setUniformValue("view", m_view);
    m_program.setUniformValue("projection", m_projection);

    // 绘制立方体线框
    glPointSize(10.0f);
    glDrawArrays(GL_POINTS, 0, 8);
    glDrawElements(GL_QUADS, 24, GL_UNSIGNED_INT, 0);
    //glDrawArrays(GL_LINES, 0, 8);
    //glDrawArrays(GL_LINE_LOOP, 0, 4);
    //glDrawArrays(GL_LINE_LOOP, 4, 4);

    m_vao.release();
    m_program.release();
}

void MyOpenGLWidget::resizeGL(int w, int h)
{
    m_projection.setToIdentity();
    m_projection.perspective(45.0f, w / float(h), 0.1f, 100.0f);
}