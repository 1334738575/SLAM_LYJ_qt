// myopenglwidget.cpp
#include "OpenGLWidgetMesh.h"
#include <QOpenGLFunctions_4_3_Core>
#include <common/BaseTriMesh.h>
#include <IO/MeshIO.h>

/********************************************OpenGLWidgetMeshAbr***************************************************/
OpenGLWidgetMeshAbr::OpenGLWidgetMeshAbr(int _w, int _h, QWidget *parent)
    : m_w(_w), m_h(_h), QOpenGLWidget(parent)
{
    // 设置焦点策略：可通过鼠标/键盘获取焦点
    setFocusPolicy(Qt::StrongFocus);
    // 可选：强制获取焦点（窗口显示时自动聚焦）
    setFocus();
    //// 设置自动更新
    //connect(&m_timer, &QTimer::timeout, this, [this]()
    //        { update(); });
    //m_timer.start(16); // ~60FPS
    // 创建 100×100 的 RGBA 格式灰色图像（灰度值可自定义，0=黑，255=白）
    int grayValue = 128; // 中等灰度（0-255 可调）
    m_textureDefault = QImage(100, 100, QImage::Format_RGBA8888); // OpenGL 兼容格式
    // 填充灰色（R=G=B=grayValue，A=255 不透明）
    m_textureDefault.fill(QColor(grayValue, grayValue, grayValue, 255));
    m_texture = m_textureDefault;
}
OpenGLWidgetMeshAbr::~OpenGLWidgetMeshAbr()
{
    m_vao.destroy();
    m_vbo.destroy();
    m_ebo.destroy(); 
    if (m_fbo) delete m_fbo;
    m_screenVAO.destroy();
    m_screenVBO.destroy();
}


void OpenGLWidgetMeshAbr::initializeGL()
{
    initializeOpenGLFunctions();
    glEnable(GL_DEPTH_TEST); // 开启深度测试

    initProgram(m_vPath, m_fPath);
    initTexture();
    glGenTextures(1, &m_textureIDDefault);
    genTexture2D(m_textureDefault, m_textureIDDefault);

    // 初始化视图矩阵
    updateMatrix();
}
void OpenGLWidgetMeshAbr::paintGL()
{
    renderFBO();
    renderWindows();
    glBindTexture(GL_TEXTURE_2D, 0);
}
void OpenGLWidgetMeshAbr::resizeGL(int w, int h)
{
    m_w = w;
    m_h = h;
    m_projection.setToIdentity();
    m_projection.perspective(60.0f, w / float(h), 0.1f, 100.0f);
}


void OpenGLWidgetMeshAbr::mousePressEvent(QMouseEvent* event)
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
void OpenGLWidgetMeshAbr::mouseMoveEvent(QMouseEvent* event)
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
void OpenGLWidgetMeshAbr::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_isPressLeft = false;
        m_isPressRight = false;
    }
}
void OpenGLWidgetMeshAbr::keyPressEvent(QKeyEvent* event)
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
void OpenGLWidgetMeshAbr::keyReleaseEvent(QKeyEvent* event)
{
    QOpenGLWidget::keyReleaseEvent(event);
}
void OpenGLWidgetMeshAbr::wheelEvent(QWheelEvent* event)
{
    m_detZ += event->delta() / 100.0f;
    update();
}


void OpenGLWidgetMeshAbr::initProgram(const std::string& _vertPath, const std::string& _fragPath)
{
    QString qvpath = QString::fromStdString(_vertPath);
    QString qfpath = QString::fromStdString(_fragPath);
    // 初始化Shader
    m_program.addShaderFromSourceFile(QOpenGLShader::Vertex, qvpath);
    m_program.addShaderFromSourceFile(QOpenGLShader::Fragment, qfpath);
    m_program.link();
    m_program.bind();

    initVAO();
    initFBO();

    m_program.release();

    initJustRenderProgram();
}
void OpenGLWidgetMeshAbr::initFBO()
{
    //std::cout << devicePixelRatio() << std::endl;
    int width = m_w;
    int height = m_h;
    // 销毁旧FBO和纹理
    if (m_fbo) {
        delete m_fbo;
        m_fbo = nullptr;
    }
    if (m_outColorId) glDeleteTextures(1, &m_outColorId);
    if (m_outFaceId) glDeleteTextures(1, &m_outFaceId);
    if (m_outDepthId) glDeleteTextures(1, &m_outDepthId);

    // 1. 创建FBO
    m_fbo = new QOpenGLFramebufferObject(width, height, QOpenGLFramebufferObject::NoAttachment);
    if (!m_fbo->isValid()) {
        std::cout << "frame buffer invalid" << std::endl;
        return;
    }
    GLuint fboId = m_fbo->handle();
    glBindFramebuffer(GL_FRAMEBUFFER, fboId);

    // 2. 创建颜色附着0纹理：存储最终颜色（RGBA8，默认纹理格式）
    glGenTextures(1, &m_outColorId);
    glBindTexture(GL_TEXTURE_2D, m_outColorId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_outColorId, 0);

    // 3. 创建颜色附着1纹理：存储面片ID（GL_RGBA8UI）
    glGenTextures(1, &m_outFaceId);
    glBindTexture(GL_TEXTURE_2D, m_outFaceId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8UI, width, height, 0, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, m_outFaceId, 0);

    // 4. 创建深度附着纹理：存储深度值（DEPTH_COMPONENT32F，高精度深度），必须有不然无法进行深度测试
    glGenTextures(1, &m_outDepthId);
    glBindTexture(GL_TEXTURE_2D, m_outDepthId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_outDepthId, 0);

    // 5. 关键：指定FBO的颜色输出附着点（与片段着色器out变量对应）
    //GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0 };
    //glDrawBuffers(1, drawBuffers);
    GLenum drawBuffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, drawBuffers);

    // 6. 检查FBO完整性（必须！确保所有附着有效）
    GLenum fboStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (fboStatus != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "frame buffer fail" << std::endl;
        return;
    }

    // 解绑纹理和FBO
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void OpenGLWidgetMeshAbr::renderWindows()
{
    //glBindTexture(GL_TEXTURE_2D, 0);
    // ========== 第二步：将FBO的颜色纹理绘制到窗口（新增核心逻辑） ==========
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f); // 窗口背景色（可选）
    glClear(GL_COLOR_BUFFER_BIT); // 清除默认帧缓冲
    glDisable(GL_DEPTH_TEST); // 绘制2D纹理无需深度测试
    // 绑定全屏着色器，绘制FBO纹理
    m_screenShader.bind();
    m_screenVAO.bind();
    m_screenVBO.bind();
    m_screenEBO.bind();
    // 绑定FBO的颜色纹理到纹理单元0，关联采样器
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_outColorId); // m_texColorId是FBO的颜色纹理ID
    m_screenShader.setUniformValue("screenTexture", 0);
    glDrawElements(GL_TRIANGLES, GLsizei(6), GL_UNSIGNED_INT, 0);
    // 解绑全屏绘制资源
    m_screenEBO.release();
    m_screenVBO.release();
    m_screenVAO.release();
    m_screenShader.release();
}


void OpenGLWidgetMeshAbr::genTexture2D(QImage& _qImg, GLuint& _texId)
{
    glBindTexture(GL_TEXTURE_2D, _texId);
    // 加载纹理图片（Qt可使用QImage，OpenGL用stb_image）
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, _qImg.width(), _qImg.height(),
        0, GL_RGBA, GL_UNSIGNED_BYTE, _qImg.bits());
    glGenerateMipmap(GL_TEXTURE_2D);
    // 7. 设置纹理采样参数（关键！否则纹理全黑）
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glBindTexture(GL_TEXTURE_2D, 0);
}
void OpenGLWidgetMeshAbr::cvMat3CToQImageRGB32(const cv::Mat& mat, QImage& qimg) {
    if (mat.empty() || mat.channels() != 3) {
        return;
    }

    cv::Mat mmm;
    cv::cvtColor(mat, mmm, cv::COLOR_BGR2BGRA);
    // BGR → BGR0（Format_RGB32的小端布局是BGR0），无需转RGB
    qimg = QImage(
        reinterpret_cast<const uchar*>(mmm.data),
        mmm.cols,
        mmm.rows,
        mmm.step,
        QImage::Format_RGB32
    );
    qimg = qimg.copy();
    return;
}


void OpenGLWidgetMeshAbr::initJustRenderProgram()
{
    // ========== 新增：初始化全屏绘制的着色器和VAO/VBO ==========
// 1. 编译全屏着色器（极简，仅采样纹理）
    std::string sPath = SHADERPATH;
    std::string vertPath = sPath + "/vShaderWindows.vert";
    std::string fragPath = sPath + "/fShaderWindows.frag";
    QString qvpath = QString::fromStdString(vertPath);
    QString qfpath = QString::fromStdString(fragPath);
    m_screenShader.addShaderFromSourceFile(QOpenGLShader::Vertex, qvpath);
    m_screenShader.addShaderFromSourceFile(QOpenGLShader::Fragment, qfpath);
    if (!m_screenShader.link()) {
        return;
    }
    m_screenShader.bind();

    // 2. 初始化全屏四边形VAO/VBO（覆盖整个NDC空间，无需投影矩阵）
        // 坐标          // 纹理坐标
    float quadVertices[] = {
        -1.f,  1.f,   0.0f, 1.0f,
        -1.f, -1.f,   0.0f, 0.0f,
         1.f, -1.f,   1.0f, 0.0f,
        -1.f,  1.f,   0.0f, 1.0f,
         1.f, -1.f,   1.0f, 0.0f,
         1.f,  1.f,   1.0f, 1.0f
    };
    m_screenVAO.create();
    m_screenVAO.bind();
    m_screenVBO.create();
    m_screenVBO.bind();
    m_screenVBO.allocate(quadVertices, sizeof(quadVertices));

    // 设置顶点属性：坐标（location=0）+ 纹理坐标（location=1）
    m_screenShader.enableAttributeArray(0);
    m_screenShader.setAttributeBuffer(0, GL_FLOAT, 0, 2, 4 * sizeof(float));
    m_screenShader.enableAttributeArray(1);
    m_screenShader.setAttributeBuffer(1, GL_FLOAT, 2 * sizeof(float), 2, 4 * sizeof(float));
    m_screenShader.setUniformValue("screenTexture", 0); // 告诉着色器采样器用单元0

    unsigned int indices[6] = {
        0, 1, 2,
        3, 4, 5
    };
    m_screenEBO.create();
    m_screenEBO.bind();
    m_screenEBO.allocate(indices, sizeof(indices));

    m_screenEBO.release();
    m_screenVBO.release();
    m_screenVAO.release();
    m_screenShader.release();
}






/********************************************OpenGLWidgetPly***************************************************/
OpenGLWidgetPly::OpenGLWidgetPly(int _w, int _h, QWidget* parent)
    :OpenGLWidgetMeshAbr(_w, _h, parent)
{
    m_vtxStep = 3;
    m_pointStep = 3;
    m_uvStep = 0;
    std::string sPath = SHADERPATH;
    m_vPath = sPath + "/vShaderMesh.vert";
    m_fPath = sPath + "/fShaderMesh.frag";

    m_verticesDefault = std::vector<float>{
    -0.5f, -0.5f, -0.5f,  
     0.5f, -0.5f, -0.5f,  
     0.5f,  0.5f, -0.5f,  
    -0.5f,  0.5f, -0.5f,  
    -0.5f, -0.5f,  0.5f,  
     0.5f, -0.5f,  0.5f,  
     0.5f,  0.5f,  0.5f,  
    -0.5f,  0.5f,  0.5f
    };
    m_indicesDefault = std::vector<unsigned int>{
        // 底面（后平面，-z）：拆分为 0→1→2 和 0→2→3
        0, 1, 2,
        0, 2, 3,
        // 顶面（前平面，+z）：拆分为 4→5→6 和 4→6→7
        4, 5, 6,
        4, 6, 7,
        // 左侧面（下平面，-y）：拆分为 0→1→5 和 0→5→4
        0, 1, 5,
        0, 5, 4,
        // 右侧面（上平面，+y）：拆分为 2→3→7 和 2→7→6
        2, 3, 7,
        2, 7, 6,
        // 前侧面（左平面，-x）：拆分为 0→3→7 和 0→7→4
        0, 3, 7,
        0, 7, 4,
        // 后侧面（右平面，+x）：拆分为 1→2→6 和 1→6→5
        1, 2, 6,
        1, 6, 5
    };
    m_vertices = m_verticesDefault.data();
    m_vSize = m_verticesDefault.size();
    m_indices = m_indicesDefault.data();
    m_iSize = m_indicesDefault.size();
}
OpenGLWidgetPly::~OpenGLWidgetPly()
{
}


void OpenGLWidgetPly::setVertices(const float* const _vtcs, unsigned long long _sz)
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
void OpenGLWidgetPly::setVerticesTexture(const float* const _vtcs, const float* const _uvs, const QImage& _img, unsigned long long _sz)
{
    return;
}
void OpenGLWidgetPly::setIndices(unsigned int* _inds, unsigned long long _sz)
{
    m_indices = _inds;
    m_iSize = _sz * 3;
}


void OpenGLWidgetPly::initVAO()
{
    m_vao.create();
    m_vao.bind();

    // 初始化顶点缓冲
    m_vbo.create();
    m_vbo.bind();
    m_vbo.allocate(m_vertices, m_vSize * sizeof(float));

    // 配置顶点属性
    m_program.enableAttributeArray(0);
    m_program.setAttributeBuffer(0, GL_FLOAT, 0, m_pointStep, m_vtxStep * sizeof(GLfloat));

    //indexs
    m_ebo.create();
    m_ebo.bind();
    m_ebo.allocate(m_indices, m_iSize * sizeof(unsigned int));

    m_ebo.release();
    m_vbo.release();
    m_vao.release();
}
void OpenGLWidgetPly::initTexture()
{
    return;
}
void OpenGLWidgetPly::updateMatrix()
{
    m_model.setToIdentity();
    m_view.lookAt(QVector3D(0, 0, -3), QVector3D(0, 0, 0), QVector3D(0, 1, 0));
    m_viewInit = m_view;
    //QMatrix4x4 QMatrix4x4::lookAt(
    //    const QVector3D & eye,    // 摄像机位置
    //    const QVector3D & center, // 观察目标点
    //    const QVector3D & up     // 定义"上"方向的向量（通常为世界坐标系Y轴）
    //);
}
void OpenGLWidgetPly::renderFBO()
{
    m_fbo->bind();
    glDisable(GL_BLEND); //输出8UI时，必须禁用
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_program.bind();
    m_vao.bind();
    m_vbo.bind();
    m_ebo.bind();

    QMatrix4x4 mTmp;
    mTmp.setToIdentity();
    mTmp.rotate(m_detYRot, QVector3D(0.f, 1.0f, 0.0f));
    mTmp.rotate(m_detXRot, QVector3D(1.f, 0.0f, 0.0f));
    mTmp(0, 3) = -m_detX;
    mTmp(1, 3) = m_detY;
    mTmp(2, 3) = m_detZ;
    m_view = m_viewInit * mTmp;

    // 传递矩阵到Shader
    m_program.setUniformValue("model", m_model);
    m_program.setUniformValue("view", m_view);
    m_program.setUniformValue("projection", m_projection);


    if (m_bDrawVertices)
    {
        glPointSize(10.0f);
        glDrawArrays(GL_POINTS, 0, m_vSize / m_vtxStep);
    }
    if (m_bDrawFaces && (m_iSize > 0))
        glDrawElements(GL_TRIANGLES, GLsizei(m_iSize), GL_UNSIGNED_INT, 0);
    //glDrawElements(GL_QUADS, 24, GL_UNSIGNED_INT, 0);
    //glDrawArrays(GL_LINES, 0, 8);
    //glDrawArrays(GL_LINE_LOOP, 0, 4);
    //glDrawArrays(GL_LINE_LOOP, 4, 4);

    m_vbo.release();
    m_ebo.release();
    m_vao.release();
    m_program.release();
    m_fbo->release();
}




/********************************************OpenGLWidgetObj***************************************************/
OpenGLWidgetObj::OpenGLWidgetObj(int _w, int _h, QWidget* parent)
    :OpenGLWidgetPly(_w, _h,  parent)
{
    m_vtxStep = 5;
    m_pointStep = 3;
    m_uvStep = 2;
    std::string sPath = SHADERPATH;
    m_vPath = sPath + "/vShaderObj.vert";
    m_fPath = sPath + "/fShaderFBO.frag";

    m_verticesDefault = std::vector<float>{
    -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
     0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f, 1.0f
    };
    m_indicesDefault = std::vector<unsigned int>{
        // 底面（后平面，-z）：拆分为 0→1→2 和 0→2→3
        0, 1, 2,
        0, 2, 3,
        // 顶面（前平面，+z）：拆分为 4→5→6 和 4→6→7
        4, 5, 6,
        4, 6, 7,
        // 左侧面（下平面，-y）：拆分为 0→1→5 和 0→5→4
        0, 1, 5,
        0, 5, 4,
        // 右侧面（上平面，+y）：拆分为 2→3→7 和 2→7→6
        2, 3, 7,
        2, 7, 6,
        // 前侧面（左平面，-x）：拆分为 0→3→7 和 0→7→4
        0, 3, 7,
        0, 7, 4,
        // 后侧面（右平面，+x）：拆分为 1→2→6 和 1→6→5
        1, 2, 6,
        1, 6, 5
    };
    m_vertices = m_verticesDefault.data();
    m_vSize = m_verticesDefault.size();
    m_indices = m_indicesDefault.data();
    m_iSize = m_indicesDefault.size();
}
OpenGLWidgetObj::~OpenGLWidgetObj()
{
}


void OpenGLWidgetObj::setVerticesTexture(const float* const _vtcs, const float* const _uvs, const QImage& _img, unsigned long long _sz)
{
    m_vSize = _sz * m_vtxStep;
    m_verticesDefault.assign(m_vSize, -1);
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


void OpenGLWidgetObj::initVAO()
{
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
    m_ebo.release();
    m_vbo.release();
    m_vao.release();
}
void OpenGLWidgetObj::initTexture()
{
    // 2. 加载纹理图片
    glGenTextures(1, &m_textureID);
    QImage imgOpengl = m_texture.convertToFormat(QImage::Format_RGBA8888).mirrored(false, true);
    genTexture2D(imgOpengl, m_textureID);
}
void OpenGLWidgetObj::renderFBO()
{
    m_fbo->bind();
    glDisable(GL_BLEND); //输出8UI时，必须禁用
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_program.bind();
    m_vao.bind();
    m_vbo.bind();
    m_ebo.bind();

    QMatrix4x4 mTmp;
    mTmp.setToIdentity();
    mTmp.rotate(m_detYRot, QVector3D(0.f, 1.0f, 0.0f));
    mTmp.rotate(m_detXRot, QVector3D(1.f, 0.0f, 0.0f));
    mTmp(0, 3) = -m_detX;
    mTmp(1, 3) = m_detY;
    mTmp(2, 3) = m_detZ;
    m_view = m_viewInit * mTmp;

    // 传递矩阵到Shader
    m_program.setUniformValue("model", m_model);
    m_program.setUniformValue("view", m_view);
    m_program.setUniformValue("projection", m_projection);


    glActiveTexture(GL_TEXTURE0); // 激活纹理单元0（默认）
    if (m_bDrawTexture)
        glBindTexture(GL_TEXTURE_2D, m_textureID);
    else
        glBindTexture(GL_TEXTURE_2D, m_textureIDDefault);
    if (m_bDrawVertices)
    {
        glPointSize(10.0f);
        glDrawArrays(GL_POINTS, 0, m_vSize / m_vtxStep);
    }
    if (m_bDrawFaces && (m_iSize > 0))
        glDrawElements(GL_TRIANGLES, GLsizei(m_iSize), GL_UNSIGNED_INT, 0);

    m_vbo.release();
    m_ebo.release();
    m_vao.release();
    m_program.release();
    m_fbo->release();
}





/********************************************MyOpenGLWidgetTs***************************************************/
MyOpenGLWidgetTs::MyOpenGLWidgetTs(int _w, int _h, QWidget* parent)
    :OpenGLWidgetObj(_w, _h, parent)
{
    std::string sPath = SHADERPATH;
    m_vPath = sPath + "/vShaderTs.vert";
    m_fPath = sPath + "/fShaderFBO.frag";
}
MyOpenGLWidgetTs::~MyOpenGLWidgetTs()
{}


void MyOpenGLWidgetTs::setData(const std::vector<SLAM_LYJ::Pose3D>& _Tcws, const std::vector<SLAM_LYJ::PinholeCamera>& _cams, const std::vector<COMMON_LYJ::CompressedImage>& _comImgs, const std::vector<SLAM_LYJ::SLAM_LYJ_MATH::BitFlagVec>& _pValids)
{
    Tcws_ = _Tcws;
    cams_ = _cams;
    int sz = _Tcws.size();
    comImgs_.resize(sz);
    pValids_.resize(sz);
    for (int i = 0; i < sz; ++i)
    {
        comImgs_[i] = const_cast<COMMON_LYJ::CompressedImage*>(&_comImgs[i]);
        pValids_[i] = const_cast<SLAM_LYJ::SLAM_LYJ_MATH::BitFlagVec*>(&_pValids[i]);
    }
}


void MyOpenGLWidgetTs::keyPressEvent(QKeyEvent* event)
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
    case Qt::Key_Right:
        curId_ = (curId_ + 1) >= Tcws_.size() ? 0 : (curId_ + 1);
        break;
    case Qt::Key_Left:
        curId_ = (curId_ - 1) < 0 ? (Tcws_.size() - 1) : (curId_ - 1);
        break;
    default:
        break;
    }
    QOpenGLWidget::keyPressEvent(event);
    update();
}


void MyOpenGLWidgetTs::initVAO()
{
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
    m_program.setUniformValue("ourTexture", 0); // 告诉着色器采样器用单元0, GL_TEXTURE0

    //indexs
    m_ebo.create();
    m_ebo.bind();
    m_ebo.allocate(m_indices, m_iSize * sizeof(unsigned int));

    m_ebo.release();
    m_vbo.release();
    m_vao.release();
}
void MyOpenGLWidgetTs::initTexture()
{
    // 2. 加载纹理图片
    int sz = Tcws_.size();
    textures_.resize(sz, 0);
    glGenTextures(sz, textures_.data());
    for (int i = 0; i < sz; ++i)
    {
        cv::Mat cvM;
        comImgs_[i]->decompressCVMat(cvM);
        QImage image;
        cvMat3CToQImageRGB32(cvM, image);
        QImage imgOpengl = image.convertToFormat(QImage::Format_RGBA8888).mirrored(false, true);
        genTexture2D(imgOpengl, textures_[i]);
    }
}
void MyOpenGLWidgetTs::renderFBO()
{
    m_fbo->bind();
    glViewport(0, 0, m_w, m_h);
    glDisable(GL_BLEND); //输出8UI时，必须禁用
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glDepthMask(GL_TRUE);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    m_program.bind();
    m_vao.bind();
    m_vbo.bind();
    m_ebo.bind();

    QMatrix4x4 mTmp;
    mTmp.setToIdentity();
    mTmp.rotate(m_detYRot, QVector3D(0.f, 1.0f, 0.0f));
    mTmp.rotate(m_detXRot, QVector3D(1.f, 0.0f, 0.0f));
    mTmp(0, 3) = -m_detX;
    mTmp(1, 3) = m_detY;
    mTmp(2, 3) = m_detZ;
    m_view = m_viewInit * mTmp;
    SLAM_LYJ::Pose3D T = Tcws_[curId_];
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            m_model(i, j) = T.getR()(i, j);
        }
        m_model(i, 3) = T.gett()(i);
    }

    // 传递矩阵到Shader
    m_program.setUniformValue("model", m_model);
    m_program.setUniformValue("view", m_view);
    m_program.setUniformValue("projection", m_projection);
    QMatrix4x4 camK;
    camK.setToIdentity();
    camK(0, 0) = cams_[0].fx() / cams_[0].wide();
    camK(1, 1) = cams_[0].fy() / cams_[0].height();
    camK(0, 2) = cams_[0].cx() / cams_[0].wide();
    camK(1, 2) = cams_[0].cy() / cams_[0].height();
    m_program.setUniformValue("cam", camK);
    // 2. 创建并初始化UBO
    int pVSz = pValids_[curId_]->size();
    for (int i = 0; i < pVSz; ++i)
    {
        if((*pValids_[curId_])[i])
            m_vertices[i * m_vtxStep + 3] = 1;
        else
            m_vertices[i * m_vtxStep + 3] = -1;
    }
    m_vbo.write(0, m_vertices, m_vSize * sizeof(float));


    glActiveTexture(GL_TEXTURE0); // 激活纹理单元0（默认）
    if (m_bDrawTexture)
        glBindTexture(GL_TEXTURE_2D, textures_[curId_]);
    else
        glBindTexture(GL_TEXTURE_2D, m_textureIDDefault);
    if (m_bDrawVertices)
    {
        glPointSize(10.0f);
        glDrawArrays(GL_POINTS, 0, m_vSize / m_vtxStep);
    }
    if (m_bDrawFaces)
        glDrawElements(GL_TRIANGLES, GLsizei(m_iSize), GL_UNSIGNED_INT, 0);

    //读取颜色附件: 使用 glReadBuffer 指定颜色附件，然后通过 glReadPixels 读取。
    //读取深度附件 : 直接调用 glReadPixels 并指定 GL_DEPTH_COMPONENT 作为格式，从而读取深度数据。
    //保存颜色
    int SCR_WIDTH = m_w;
    int SCR_HEIGHT = m_h;

    if(true){
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        std::vector<float> colors(SCR_WIDTH * SCR_HEIGHT * 4);
        glReadPixels(0, 0, SCR_WIDTH, SCR_HEIGHT, GL_RGBA, GL_FLOAT, colors.data());
        cv::Mat mm = cv::Mat::zeros(SCR_HEIGHT, SCR_WIDTH, CV_8UC3);
        for (uint32_t i = 0; i < SCR_WIDTH * SCR_HEIGHT; ++i) {
            uint32_t id = i * 4;
            int r = colors[id + 0] * 255;
            int g = colors[id + 1] * 255;
            int b = colors[id + 2] * 255;
            int a = colors[id + 3] * 255;
            int x = i % SCR_WIDTH;
            int y = i / SCR_WIDTH;
            cv::Vec3b& vvv = mm.at<cv::Vec3b>(m_h - y - 1, x);
            vvv[0] = uchar(b);
            vvv[1] = uchar(g);
            vvv[2] = uchar(r);
        }
        cv::imshow("clr", mm);
    }

    if(false){
        glReadBuffer(GL_COLOR_ATTACHMENT1);
        std::vector<uchar> colors(SCR_WIDTH * SCR_HEIGHT * 4);
        glReadPixels(0, 0, SCR_WIDTH, SCR_HEIGHT, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, colors.data());
        std::vector<uint32_t> fids(SCR_WIDTH * SCR_HEIGHT, UINT_MAX);
        cv::Mat mm = cv::Mat::zeros(SCR_HEIGHT, SCR_WIDTH, CV_8UC1);
        for (uint32_t i = 0; i < SCR_WIDTH * SCR_HEIGHT; ++i) {
            uint32_t id = i * 4;
            if (((int)colors[id + 0] + (int)colors[id + 1] + (int)colors[id + 2] + (int)colors[id + 3]) == 0)
                continue;
            uchar r = colors[id + 0];
            uchar g = colors[id + 1];
            uchar b = colors[id + 2];
            uchar a = colors[id + 3];
            int x = i % SCR_WIDTH;
            int y = i / SCR_WIDTH;
            uint32_t ret = ((colors[id + 0] - 1) << 24) | (colors[id + 1] << 16) | (colors[id + 2] << 8) | colors[id + 3];
            fids[i] = ret;
            mm.at<uchar>(y, x) = 255;
        }
        cv::imshow("fid", mm);
        std::vector<Eigen::Vector3f> ps;
        for (int i = 0; i < fids.size(); ++i)
        {
            if (fids[i] == UINT_MAX)
                continue;
            for (int j = 0; j < 3; ++j)
            {
                Eigen::Vector3f p;
                uint pId = m_indices[3 * fids[i] + j];
                p(0) = m_verticesDefault[5 * pId];
                p(1) = m_verticesDefault[5 * pId + 1];
                p(2) = m_verticesDefault[5 * pId + 2];
                ps.push_back(p);
            }
        }
        SLAM_LYJ::SLAM_LYJ_MATH::BaseTriMesh btm;
        btm.setVertexs(ps);
        SLAM_LYJ::writePLYMesh("D:/tmp/fid.ply", btm);
    }
    
    //{
    //    std::vector<float> depthData(SCR_WIDTH * SCR_HEIGHT);
    //    glBindTexture(GL_TEXTURE_2D, m_outDepthId);
    //    // 正常应为 GL_DEPTH_COMPONENT32F (0x8CAC) 或 GL_DEPTH_COMPONENT24 (0x81A6)
    //    glGetTexImage(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, GL_FLOAT, depthData.data());
    //    cv::Mat m(SCR_HEIGHT, SCR_WIDTH, CV_8UC1);
    //    for (int i = 0; i < SCR_WIDTH * SCR_HEIGHT; ++i) {
    //        //if (depthData[i] != 1.0)
    //            //std::cout << "111111" << std::endl;
    //        int x = i % SCR_WIDTH;
    //        int y = i / SCR_WIDTH;
    //        float d = (depthData[i] - 0.5f) * 2;
    //        m.at<uchar>(y, x) = static_cast<unsigned char>(d * 255);
    //    }
    //    cv::imshow("ddd", m);
    //}

    m_vbo.release();
    m_ebo.release();
    m_vao.release();
    m_program.release();
    m_fbo->release();
    
}