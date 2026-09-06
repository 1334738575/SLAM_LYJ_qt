// myopenglwidget.cpp
#include "OpenGLWidgetMesh.h"
#include <QOpenGLFunctions_4_3_Core>
#include <common/BaseTriMesh.h>
#include <IO/MeshIO.h>
#include <QImage>
#include <QPainter>
#include <QRect>
#include <algorithm>
#include <cmath>
#include <utility>

/********************************************OpenGLWidgetMeshAbr***************************************************/
OpenGLWidgetMeshAbr::OpenGLWidgetMeshAbr(int _w, int _h, QWidget* parent)
    : m_w(_w), m_h(_h), QOpenGLWidget(parent)
{
    // 设置焦点策略：可通过鼠标/键盘获取焦点
    setFocusPolicy(Qt::StrongFocus);
    // 可选：强制获取焦点（窗口显示时自动聚焦）
    setFocus();
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
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
    attch1.resize(m_w * m_h * 4, 0);
    fids.resize(m_w * m_h, UINT_MAX);
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
    initMatrix();
}
void OpenGLWidgetMeshAbr::paintGL()
{
    renderFBO();
    renderWindows();
    glBindTexture(GL_TEXTURE_2D, 0);
}
void OpenGLWidgetMeshAbr::resizeGL(int w, int h)
{
    m_w = w > 0 ? w : 1;
    m_h = h > 0 ? h : 1;
    m_projection.setToIdentity();
    const float radius = std::max(m_boundsRadius, 1.0e-3f);
    m_projection.perspective(60.0f, m_w / static_cast<float>(m_h),
        std::max(1.0e-4f, radius * 1.0e-3f), std::max(100.0f, radius * 8.0f));
    initFBO();
    fitViewToBounds();
    update();
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
    const float dx = static_cast<float>(event->x() - m_lastPos.x());
    const float dy = static_cast<float>(event->y() - m_lastPos.y());
    if (m_isPressLeft)
    {
        const int maxSide = m_w > m_h ? m_w : m_h;
        const float rotateSpeed = maxSide > 0 ? 180.0f / static_cast<float>(maxSide) : 0.2f;
        QMatrix4x4 delta;
        delta.rotate(dx * rotateSpeed, QVector3D(0.f, 1.0f, 0.0f));
        delta.rotate(-dy * rotateSpeed, QVector3D(1.f, 0.0f, 0.0f));
        m_viewRotation = delta * m_viewRotation;
        m_lastPos = event->pos();
        update();
    }
    else if (m_isPressRight)
    {
        const int minSide = m_w < m_h ? m_w : m_h;
        const float panSpeed = minSide > 0 ? 2.0f / static_cast<float>(minSide) : 0.002f;
        m_detY -= dy * panSpeed;
        m_detX += dx * panSpeed;
        m_lastPos = event->pos();
        update();
    }
}
void OpenGLWidgetMeshAbr::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        updateRotationCenter(event->pos());
        update();
    }
}
void OpenGLWidgetMeshAbr::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_isPressLeft = false;
    }
    else if (event->button() == Qt::RightButton)
    {
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
    case Qt::Key_R:
        initMatrix();
        break;
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
    const float steps = event->angleDelta().y() / 120.0f;
    m_zoom = std::clamp(m_zoom * std::pow(1.15f, steps), 0.03f, 30.0f);
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
    int width = m_w > 0 ? m_w : 1;
    int height = m_h > 0 ? m_h : 1;
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
void OpenGLWidgetMeshAbr::initVAO()
{
    m_vao.create();
    m_vao.bind();

    // 初始化顶点缓冲
    m_vbo.create();
    m_vbo.bind();
    m_vbo.allocate(m_vertices, m_vSize * sizeof(float));

    setAttribute();

    //indexs
    m_ebo.create();
    m_ebo.bind();
    m_ebo.allocate(m_indices, m_iSize * sizeof(unsigned int));

    m_ebo.release();
    m_vbo.release();
    m_vao.release();
}
void OpenGLWidgetMeshAbr::updateViewInit()
{
    int curX = m_lastPos.x();
    int curY = m_lastPos.y();
    uint fId = fids[(m_h - curY - 1) * m_w + curX];
    if (fId == UINT_MAX)
        return;
    float fx = 0, fy = 0, fz = 0;
    for (int i = 0; i < 3; ++i)
    {
        fx += m_vertices[m_indices[fId * 3 + i] + 0];
        fy += m_vertices[m_indices[fId * 3 + i] + 1];
        fz += m_vertices[m_indices[fId * 3 + i] + 2];
    }
    fx /= 3;
    fy /= 3;
    fz /= 3;
    m_viewInit.lookAt(QVector3D(fx, fy, fz - 3), QVector3D(fx, fy, fz), QVector3D(0, 1, 0));

    return;
}
void OpenGLWidgetMeshAbr::updateRotationCenter(const QPoint& pos)
{
    if (pos.x() < 0 || pos.x() >= m_w || pos.y() < 0 || pos.y() >= m_h || !m_vertices)
        return;

    if (fids.size() == static_cast<size_t>(m_w * m_h) && m_indices)
    {
        const uint fId = fids[(m_h - pos.y() - 1) * m_w + pos.x()];
        if (fId != UINT_MAX && fId * 3 + 2 < static_cast<uint>(m_iSize))
        {
            QVector3D center(0.f, 0.f, 0.f);
            for (int i = 0; i < 3; ++i)
            {
                const uint pId = m_indices[fId * 3 + i];
                if (pId * m_vtxStep + 2 >= static_cast<uint>(m_vSize))
                    return;
                center += QVector3D(m_vertices[pId * m_vtxStep], m_vertices[pId * m_vtxStep + 1], m_vertices[pId * m_vtxStep + 2]);
            }
            m_rotationCenter = center / 3.0f;
            return;
        }
    }

    const float selectRadius2 = 30.0f * 30.0f;
    float bestDist2 = selectRadius2;
    int bestIndex = -1;
    const QMatrix4x4 mvp = m_projection * m_view * m_model;
    for (int i = 0; i + 2 < m_vSize; i += m_vtxStep)
    {
        const QVector4D clip = mvp * QVector4D(m_vertices[i], m_vertices[i + 1], m_vertices[i + 2], 1.0f);
        if (clip.w() == 0.0f)
            continue;
        const QVector3D ndc = clip.toVector3DAffine();
        const float sx = (ndc.x() + 1.0f) * 0.5f * m_w;
        const float sy = (1.0f - ndc.y()) * 0.5f * m_h;
        const float dx = sx - static_cast<float>(pos.x());
        const float dy = sy - static_cast<float>(pos.y());
        const float dist2 = dx * dx + dy * dy;
        if (dist2 < bestDist2)
        {
            bestDist2 = dist2;
            bestIndex = i;
        }
    }

    if (bestIndex >= 0)
        m_rotationCenter = QVector3D(m_vertices[bestIndex], m_vertices[bestIndex + 1], m_vertices[bestIndex + 2]);
}
void OpenGLWidgetMeshAbr::initMatrix()
{
    m_model.setToIdentity();
    m_zoom = 1.0f;
    m_viewInit.lookAt(QVector3D(0, 0, -3), QVector3D(0, 0, 0), QVector3D(0, 1, 0));
    m_viewRotation.setToIdentity();
    m_rotationCenter = QVector3D(0.f, 0.f, 0.f);
    if (m_hasBounds)
        fitViewToBounds();
    //QMatrix4x4 QMatrix4x4::lookAt(
    //    const QVector3D & eye,    // 摄像机位置
    //    const QVector3D & center, // 观察目标点
    //    const QVector3D & up     // 定义"上"方向的向量（通常为世界坐标系Y轴）
    //);
}
void OpenGLWidgetMeshAbr::renderFBO()
{
    if (!m_fbo)
        return;
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

    updateMatrixAndUBO();

    drawFBO();

    //读取颜色附件: 使用 glReadBuffer 指定颜色附件，然后通过 glReadPixels 读取。
    //读取深度附件 : 直接调用 glReadPixels 并指定 GL_DEPTH_COMPONENT 作为格式，从而读取深度数据。
    //保存颜色
    int SCR_WIDTH = m_w;
    int SCR_HEIGHT = m_h;

    if (false) {
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
    glReadBuffer(GL_COLOR_ATTACHMENT1);
    attch1.resize(SCR_WIDTH * SCR_HEIGHT * 4);
    glReadPixels(0, 0, SCR_WIDTH, SCR_HEIGHT, GL_RGBA_INTEGER, GL_UNSIGNED_BYTE, attch1.data());
    std::vector<uchar> colorPixels(static_cast<size_t>(SCR_WIDTH) * SCR_HEIGHT * 4);
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glReadPixels(0, 0, SCR_WIDTH, SCR_HEIGHT, GL_RGBA, GL_UNSIGNED_BYTE, colorPixels.data());
    fids.resize(SCR_WIDTH * SCR_HEIGHT, UINT_MAX);
    for (uint32_t i = 0; i < SCR_WIDTH * SCR_HEIGHT; ++i) {
        uint32_t id = i * 4;
        if (colorPixels[id + 3] == 0)
            continue;
        if (((int)attch1[id + 0] + (int)attch1[id + 1] + (int)attch1[id + 2] + (int)attch1[id + 3]) == 0)
            continue;
        uchar r = attch1[id + 0];
        uchar g = attch1[id + 1];
        uchar b = attch1[id + 2];
        uchar a = attch1[id + 3];
        uint32_t ret = ((attch1[id + 0] - 1) << 24) | (attch1[id + 1] << 16) | (attch1[id + 2] << 8) | attch1[id + 3];
        fids[i] = ret;
    }
    if (false) {
        cv::Mat mm = cv::Mat::zeros(SCR_HEIGHT, SCR_WIDTH, CV_8UC1);
        for (uint32_t i = 0; i < SCR_WIDTH * SCR_HEIGHT; ++i) {
            uint32_t id = i * 4;
            if (((int)attch1[id + 0] + (int)attch1[id + 1] + (int)attch1[id + 2] + (int)attch1[id + 3]) == 0)
                continue;
            uchar r = attch1[id + 0];
            uchar g = attch1[id + 1];
            uchar b = attch1[id + 2];
            uchar a = attch1[id + 3];
            int x = i % SCR_WIDTH;
            int y = i / SCR_WIDTH;
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
        COMMON_LYJ::BaseTriMesh btm;
        btm.setVertexs(ps);
        COMMON_LYJ::writePLYMesh("D:/tmp/fid.ply", btm);
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
void OpenGLWidgetMeshAbr::renderWindows()
{
    //glBindTexture(GL_TEXTURE_2D, 0);
    // ========== 第二步：将FBO的颜色纹理绘制到窗口（新增核心逻辑） ==========
    glViewport(0, 0, m_w, m_h);
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
    if (_texId == 0 || _qImg.isNull() || _qImg.width() <= 0 || _qImg.height() <= 0)
        return;
    const QImage uploadImage = _qImg.convertToFormat(QImage::Format_RGBA8888);
    glBindTexture(GL_TEXTURE_2D, _texId);
    // 加载纹理图片（Qt可使用QImage，OpenGL用stb_image）
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, uploadImage.width(), uploadImage.height(),
        0, GL_RGBA, GL_UNSIGNED_BYTE, uploadImage.constBits());
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
    if (_vtcs == nullptr || _sz == 0)
    {
        m_vSize = 0;
        m_verticesDefault.clear();
        m_vertices = nullptr;
        m_hasBounds = false;
        return;
    }
    m_vSize = _sz * m_vtxStep;
    m_verticesDefault.assign(m_vSize, -1);
    m_vertices = m_verticesDefault.data();
    for (int i = 0; i < _sz; ++i)
    {
        m_verticesDefault[m_vtxStep * i] = _vtcs[3 * i];
        m_verticesDefault[m_vtxStep * i + 1] = _vtcs[3 * i + 1];
        m_verticesDefault[m_vtxStep * i + 2] = _vtcs[3 * i + 2];
    }
    QVector3D minPoint(m_verticesDefault[0], m_verticesDefault[1], m_verticesDefault[2]);
    QVector3D maxPoint = minPoint;
    for (unsigned long long i = 1; i < _sz; ++i)
    {
        const QVector3D point(m_verticesDefault[m_vtxStep * i], m_verticesDefault[m_vtxStep * i + 1],
            m_verticesDefault[m_vtxStep * i + 2]);
        minPoint.setX(std::min(minPoint.x(), point.x()));
        minPoint.setY(std::min(minPoint.y(), point.y()));
        minPoint.setZ(std::min(minPoint.z(), point.z()));
        maxPoint.setX(std::max(maxPoint.x(), point.x()));
        maxPoint.setY(std::max(maxPoint.y(), point.y()));
        maxPoint.setZ(std::max(maxPoint.z(), point.z()));
    }
    m_boundsCenter = (minPoint + maxPoint) * 0.5f;
    m_boundsRadius = std::max((maxPoint - minPoint).length() * 0.5f, 1.0e-3f);
    m_hasBounds = true;
    fitViewToBounds();
}
void OpenGLWidgetPly::setVerticesTexture(const float* const _vtcs, const float* const _uvs, const QImage& _img, unsigned long long _sz)
{
    return;
}
void OpenGLWidgetPly::setIndices(const unsigned int* _inds, unsigned long long _sz)
{
	if (_inds == nullptr || _sz == 0)
	{
		m_iSize = 0;
		m_indicesDefault.clear();
		m_indices = nullptr;
		return;
	}
	m_iSize = _sz * 3;
	m_indicesDefault.assign(_inds, _inds + m_iSize);
	m_indices = m_indicesDefault.data();
}

void OpenGLWidgetMeshAbr::fitViewToBounds()
{
    if (!m_hasBounds)
        return;

    const float radius = std::max(m_boundsRadius, 1.0e-3f);
    const float distance = radius * 2.5f;
    m_viewInit.setToIdentity();
    m_viewInit.lookAt(m_boundsCenter + QVector3D(0.0f, 0.0f, -distance),
        m_boundsCenter, QVector3D(0.0f, 1.0f, 0.0f));
    m_rotationCenter = m_boundsCenter;
}


void OpenGLWidgetPly::setAttribute()
{
    // 配置顶点属性
    m_program.enableAttributeArray(0);
    m_program.setAttributeBuffer(0, GL_FLOAT, 0, m_pointStep, m_vtxStep * sizeof(GLfloat));
}
void OpenGLWidgetPly::initTexture()
{
    return;
}
void OpenGLWidgetPly::updateMatrixAndUBO()
{
    //updateViewInit();
    QMatrix4x4 mTmp;
    mTmp.setToIdentity();
    mTmp.translate(m_rotationCenter);
    mTmp *= m_viewRotation;
    mTmp.translate(-m_rotationCenter);
    mTmp.scale(m_zoom);
    QMatrix4x4 pan;
    pan.translate(-m_detX, m_detY, 0.0f);
    mTmp = pan * mTmp;
    m_view = m_viewInit * mTmp;
    // 传递矩阵到Shader
    m_program.setUniformValue("model", m_model);
    m_program.setUniformValue("view", m_view);
    m_program.setUniformValue("projection", m_projection);
}
void OpenGLWidgetPly::drawFBO()
{
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
}




/********************************************OpenGLWidgetObj***************************************************/
OpenGLWidgetObj::OpenGLWidgetObj(int _w, int _h, QWidget* parent)
    :OpenGLWidgetPly(_w, _h, parent)
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
    if (_vtcs == nullptr || _uvs == nullptr || _sz == 0)
    {
        m_vSize = 0;
        m_verticesDefault.clear();
        m_vertices = nullptr;
        m_texture = QImage();
        m_hasBounds = false;
        return;
    }
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
    QVector3D minPoint(m_verticesDefault[0], m_verticesDefault[1], m_verticesDefault[2]);
    QVector3D maxPoint = minPoint;
    for (unsigned long long i = 1; i < _sz; ++i)
    {
        const QVector3D point(m_verticesDefault[m_vtxStep * i], m_verticesDefault[m_vtxStep * i + 1],
            m_verticesDefault[m_vtxStep * i + 2]);
        minPoint.setX(std::min(minPoint.x(), point.x()));
        minPoint.setY(std::min(minPoint.y(), point.y()));
        minPoint.setZ(std::min(minPoint.z(), point.z()));
        maxPoint.setX(std::max(maxPoint.x(), point.x()));
        maxPoint.setY(std::max(maxPoint.y(), point.y()));
        maxPoint.setZ(std::max(maxPoint.z(), point.z()));
    }
    m_boundsCenter = (minPoint + maxPoint) * 0.5f;
    m_boundsRadius = std::max((maxPoint - minPoint).length() * 0.5f, 1.0e-3f);
    m_hasBounds = true;
    fitViewToBounds();
}

void OpenGLWidgetObj::setHoverTexturePreviewCallback(HoverTexturePreviewCallback callback)
{
    hoverTexturePreviewCallback_ = std::move(callback);
    setMouseTracking(true);
    connect(&hoverPreviewTimer_, &QTimer::timeout, this, &OpenGLWidgetObj::updateHoverTexturePreview,
        Qt::UniqueConnection);
    hoverPreviewTimer_.setSingleShot(true);
}

void OpenGLWidgetObj::mouseMoveEvent(QMouseEvent* event)
{
    OpenGLWidgetPly::mouseMoveEvent(event);
    hoverPreviewPos_ = event->pos();
    if (hoverTexturePreviewCallback_)
        hoverPreviewTimer_.start(180);
}

void OpenGLWidgetObj::leaveEvent(QEvent* event)
{
    hoverPreviewTimer_.stop();
    if (hoverTexturePreviewCallback_)
        hoverTexturePreviewCallback_(QImage(), -1, QPointF(-1.0, -1.0));
    OpenGLWidgetPly::leaveEvent(event);
}

void OpenGLWidgetObj::updateHoverTexturePreview()
{
    if (!hoverTexturePreviewCallback_)
        return;
    const auto clearPreview = [this]() { hoverTexturePreviewCallback_(QImage(), -1, QPointF(-1.0, -1.0)); };
    if (m_texture.isNull() || m_indices == nullptr || m_iSize < 3 || m_w <= 0 || m_h <= 0)
    {
        clearPreview();
        return;
    }

    const int x = std::clamp(hoverPreviewPos_.x(), 0, m_w - 1);
    const int y = std::clamp(hoverPreviewPos_.y(), 0, m_h - 1);
    const size_t fboIndex = static_cast<size_t>(m_h - y - 1) * static_cast<size_t>(m_w) + x;
    if (fboIndex >= fids.size())
    {
        clearPreview();
        return;
    }
    const uint faceId = fids[fboIndex];
    if (faceId == UINT_MAX || faceId * 3 + 2 >= static_cast<uint>(m_iSize))
    {
        clearPreview();
        return;
    }

    float minU = 1.0f, maxU = 0.0f, minV = 1.0f, maxV = 0.0f;
    QPointF screenPoints[3];
    const QMatrix4x4 mvp = m_projection * m_view * m_model;
    for (int corner = 0; corner < 3; ++corner)
    {
        const uint vertexId = m_indices[faceId * 3 + corner];
        if (vertexId * m_vtxStep + 4 >= static_cast<uint>(m_vSize))
        {
            clearPreview();
            return;
        }
        const float u = m_vertices[vertexId * m_vtxStep + 3];
        const float v = m_vertices[vertexId * m_vtxStep + 4];
        if (!std::isfinite(u) || !std::isfinite(v))
        {
            clearPreview();
            return;
        }
        minU = std::min(minU, u);
        maxU = std::max(maxU, u);
        minV = std::min(minV, v);
        maxV = std::max(maxV, v);

        const QVector4D clip = mvp * QVector4D(
            m_vertices[vertexId * m_vtxStep],
            m_vertices[vertexId * m_vtxStep + 1],
            m_vertices[vertexId * m_vtxStep + 2], 1.0f);
        if (std::abs(clip.w()) < 1e-6f)
        {
            clearPreview();
            return;
        }
        const QVector3D ndc = clip.toVector3DAffine();
        screenPoints[corner] = QPointF((ndc.x() + 1.0f) * 0.5f * m_w,
            (1.0f - ndc.y()) * 0.5f * m_h);
    }

    const int imageWidth = m_texture.width();
    const int imageHeight = m_texture.height();
    const double denominator = (screenPoints[1].y() - screenPoints[2].y()) *
        (screenPoints[0].x() - screenPoints[2].x()) +
        (screenPoints[2].x() - screenPoints[1].x()) *
        (screenPoints[0].y() - screenPoints[2].y());
    float centerU = (minU + maxU) * 0.5f;
    float centerV = (minV + maxV) * 0.5f;
    if (std::abs(denominator) <= 1e-6)
    {
        clearPreview();
        return;
    }
    const double w0 = ((screenPoints[1].y() - screenPoints[2].y()) *
            (hoverPreviewPos_.x() - screenPoints[2].x()) +
            (screenPoints[2].x() - screenPoints[1].x()) *
            (hoverPreviewPos_.y() - screenPoints[2].y())) / denominator;
    const double w1 = ((screenPoints[2].y() - screenPoints[0].y()) *
            (hoverPreviewPos_.x() - screenPoints[2].x()) +
            (screenPoints[0].x() - screenPoints[2].x()) *
            (hoverPreviewPos_.y() - screenPoints[2].y())) / denominator;
    const double w2 = 1.0 - w0 - w1;
    if (w0 < -0.05 || w1 < -0.05 || w2 < -0.05)
    {
        clearPreview();
        return;
    }
    {
        centerU = static_cast<float>(w0 * m_vertices[m_indices[faceId * 3] * m_vtxStep + 3] +
                w1 * m_vertices[m_indices[faceId * 3 + 1] * m_vtxStep + 3] +
                w2 * m_vertices[m_indices[faceId * 3 + 2] * m_vtxStep + 3]);
        centerV = static_cast<float>(w0 * m_vertices[m_indices[faceId * 3] * m_vtxStep + 4] +
                w1 * m_vertices[m_indices[faceId * 3 + 1] * m_vtxStep + 4] +
                w2 * m_vertices[m_indices[faceId * 3 + 2] * m_vtxStep + 4]);
    }
    centerU = std::clamp(centerU, 0.0f, 1.0f);
    centerV = std::clamp(centerV, 0.0f, 1.0f);
    const int cropWidth = std::max(320, static_cast<int>(std::ceil((maxU - minU) * imageWidth * 2.5f)));
    const int cropHeight = std::max(240, static_cast<int>(std::ceil((maxV - minV) * imageHeight * 2.5f)));
    const int centerX = static_cast<int>(std::lround(centerU * imageWidth));
    const int centerY = static_cast<int>(std::lround((1.0f - centerV) * imageHeight));
    const int left = centerX - cropWidth / 2;
    const int top = centerY - cropHeight / 2;
    const int right = left + cropWidth;
    const int bottom = top + cropHeight;
    const QRect crop = QRect(left, top, right - left, bottom - top).intersected(m_texture.rect());
    if (crop.width() < 2 || crop.height() < 2)
    {
        clearPreview();
        return;
    }
    QImage preview = m_texture.copy(crop).convertToFormat(QImage::Format_RGB32);
    QPainter painter(&preview);
    painter.setPen(QPen(Qt::red, std::max(2, std::min(preview.width(), preview.height()) / 120)));
    const QRectF faceRect(
        minU * imageWidth - crop.left(),
        (1.0f - maxV) * imageHeight - crop.top(),
        std::max(1.0f, (maxU - minU) * imageWidth),
        std::max(1.0f, (maxV - minV) * imageHeight));
    painter.drawRect(faceRect);
    hoverTexturePreviewCallback_(preview, static_cast<int>(faceId), QPointF(centerU, centerV));
}


void OpenGLWidgetObj::setAttribute()
{
    // 配置顶点属性
    m_program.enableAttributeArray(0);
    m_program.setAttributeBuffer(0, GL_FLOAT, 0, m_pointStep, m_vtxStep * sizeof(GLfloat));
    // 3. 配置VAO/VBO，添加纹理坐标属性
    m_program.enableAttributeArray(1);
    m_program.setAttributeBuffer(1, GL_FLOAT, m_pointStep * sizeof(GLfloat), m_uvStep, m_vtxStep * sizeof(GLfloat));
    m_program.setUniformValue("ourTexture", 0); // 告诉着色器采样器用单元0
}
void OpenGLWidgetObj::initTexture()
{
    // 2. 加载纹理图片
    if (m_texture.isNull())
        return;
    glGenTextures(1, &m_textureID);
    QImage imgOpengl = m_texture.convertToFormat(QImage::Format_RGBA8888).mirrored(false, true);
    genTexture2D(imgOpengl, m_textureID);
}
void OpenGLWidgetObj::drawFBO()
{
    glActiveTexture(GL_TEXTURE0); // 激活纹理单元0（默认）
    if (m_bDrawTexture && m_textureID != 0)
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
}





/********************************************MyOpenGLWidgetTs***************************************************/
MyOpenGLWidgetTs::MyOpenGLWidgetTs(int _w, int _h, QWidget* parent)
    :OpenGLWidgetObj(_w, _h, parent)
{
    setMouseTracking(true);
    connect(&previewTimer_, &QTimer::timeout, this, &MyOpenGLWidgetTs::updateTexturePreview);
    previewTimer_.setSingleShot(true);
    std::string sPath = SHADERPATH;
    m_vPath = sPath + "/vShaderTs.vert";
    m_fPath = sPath + "/fShaderFBO.frag";
}

void MyOpenGLWidgetTs::setTexturePreviewCallback(TexturePreviewCallback callback)
{
    texturePreviewCallback_ = std::move(callback);
}
MyOpenGLWidgetTs::~MyOpenGLWidgetTs()
{
}


void MyOpenGLWidgetTs::setData(const std::vector<COMMON_LYJ::Pose3D>& _Tcws, const std::vector<QT_LYJ::ProjectorCamera>& _cams, const std::vector<COMMON_LYJ::CompressedImage>& _comImgs, const std::vector<COMMON_LYJ::BitFlagVec>& _pValids)
{
    Tcws_ = _Tcws;
    cams_ = _cams;
    curId_ = 0;
    const size_t sz = Tcws_.size();
    comImgs_.clear();
    pValids_.clear();
    if (sz == 0 || _comImgs.empty() || _pValids.empty())
        return;
    comImgs_.resize(sz);
    pValids_.resize(sz);
    previewImages_.clear();
    for (size_t i = 0; i < sz; ++i)
    {
        comImgs_[i] = _comImgs[std::min(i, _comImgs.size() - 1)];
        pValids_[i] = _pValids[std::min(i, _pValids.size() - 1)];
    }
}

void MyOpenGLWidgetTs::mouseMoveEvent(QMouseEvent* event)
{
    OpenGLWidgetObj::mouseMoveEvent(event);
    previewPos_ = event->pos();
    previewTimer_.start(previewDelayMs_);
}

void MyOpenGLWidgetTs::leaveEvent(QEvent* event)
{
    previewTimer_.stop();
    if (texturePreviewCallback_)
        texturePreviewCallback_(QImage(), -1, QPointF(-1.0, -1.0));
    OpenGLWidgetObj::leaveEvent(event);
}

namespace
{
    bool projectTexturePoint(const COMMON_LYJ::Pose3D& pose,
        const QT_LYJ::ProjectorCamera& camera, const float* point, QPointF& pixel)
    {
        Eigen::Vector3d world(point[0], point[1], point[2]);
        Eigen::Vector3d cameraPoint = pose.getR() * world + pose.gett();
        if (cameraPoint.z() <= 1e-6 || camera.width <= 0 || camera.height <= 0)
            return false;

        double nx = cameraPoint.x() / cameraPoint.z();
        double ny = cameraPoint.y() / cameraPoint.z();
        if (camera.model == QT_LYJ::ProjectorCameraModel::Fisheye)
        {
            const double radius = std::hypot(nx, ny);
            if (radius > 1e-12)
            {
                const double theta = std::atan(radius);
                const double theta2 = theta * theta;
                const double theta4 = theta2 * theta2;
                const double theta6 = theta4 * theta2;
                const double theta8 = theta4 * theta4;
                const double distorted = theta * (1.0 + camera.parameters[4] * theta2
                    + camera.parameters[5] * theta4 + camera.parameters[6] * theta6
                    + camera.parameters[7] * theta8);
                const double scale = distorted / radius;
                nx *= scale;
                ny *= scale;
            }
        }

        pixel = QPointF(camera.parameters[0] * nx + camera.parameters[2],
            camera.parameters[1] * ny + camera.parameters[3]);
        return true;
    }
}

void MyOpenGLWidgetTs::updateTexturePreview()
{
    if (!texturePreviewCallback_)
        return;
    const auto clearPreview = [this]() { texturePreviewCallback_(QImage(), -1, QPointF(-1.0, -1.0)); };
    if (Tcws_.empty() || cams_.empty() || comImgs_.empty() || m_indices == nullptr ||
        m_iSize < 3 || m_w <= 0 || m_h <= 0)
    {
        clearPreview();
        return;
    }

    const int x = std::clamp(previewPos_.x(), 0, m_w - 1);
    const int y = std::clamp(previewPos_.y(), 0, m_h - 1);
    const size_t fboIndex = static_cast<size_t>(m_h - y - 1) * static_cast<size_t>(m_w) + x;
    if (fboIndex >= fids.size())
    {
        clearPreview();
        return;
    }
    const uint faceId = fids[fboIndex];
    if (faceId == UINT_MAX || faceId * 3 + 2 >= static_cast<uint>(m_iSize))
    {
        clearPreview();
        return;
    }

    const size_t frameId = std::min<size_t>(curId_, comImgs_.size() - 1);
    const size_t cameraId = cams_.size() == 1 ? 0 : std::min(frameId, cams_.size() - 1);
    const QT_LYJ::ProjectorCamera& camera = cams_[cameraId];
    if (previewImages_.size() <= frameId)
        previewImages_.resize(frameId + 1);
    if (previewImages_[frameId].isNull())
    {
        cv::Mat cvImage;
        bool decoded = false;
        try { decoded = comImgs_[frameId].decompressCVMat(cvImage); }
        catch (const std::exception&) { decoded = false; }
        if (!decoded)
        {
            clearPreview();
            return;
        }
        cvMat3CToQImageRGB32(cvImage, previewImages_[frameId]);
    }
    const QImage& image = previewImages_[frameId];
    if (image.isNull())
    {
        clearPreview();
        return;
    }

    QPointF projected[3];
    QRectF bounds;
    bool valid = true;
    for (int corner = 0; corner < 3; ++corner)
    {
        const uint vertexId = m_indices[faceId * 3 + corner];
        if (vertexId * m_vtxStep + 2 >= static_cast<uint>(m_vSize) ||
            !projectTexturePoint(Tcws_[curId_], camera,
                &m_vertices[vertexId * m_vtxStep], projected[corner]))
        {
            valid = false;
            break;
        }
        if (corner == 0)
            bounds = QRectF(projected[corner], QSizeF(0, 0));
        else
            bounds = bounds.united(QRectF(projected[corner], QSizeF(0, 0)));
    }
    if (!valid)
    {
        clearPreview();
        return;
    }

    const double margin = std::max(24.0, std::max(bounds.width(), bounds.height()) * 0.35);
    QRect crop(static_cast<int>(std::floor(bounds.left() - margin)),
        static_cast<int>(std::floor(bounds.top() - margin)),
        static_cast<int>(std::ceil(bounds.width() + margin * 2.0)),
        static_cast<int>(std::ceil(bounds.height() + margin * 2.0)));
    crop = crop.intersected(image.rect());
    if (crop.width() < 2 || crop.height() < 2)
    {
        clearPreview();
        return;
    }

    QImage preview = image.copy(crop).convertToFormat(QImage::Format_RGB32);
    QPainter painter(&preview);
    painter.setPen(QPen(Qt::red, std::max(2, std::min(preview.width(), preview.height()) / 120)));
    const QRectF faceRect(
        bounds.left() - crop.left(), bounds.top() - crop.top(),
        std::max(1.0, bounds.width()), std::max(1.0, bounds.height()));
    painter.drawRect(faceRect);
    const QPointF uvCenter(
        std::clamp(bounds.center().x() / static_cast<qreal>(camera.width), 0.0, 1.0),
        std::clamp(bounds.center().y() / static_cast<qreal>(camera.height), 0.0, 1.0));
    texturePreviewCallback_(preview, static_cast<int>(faceId), uvCenter);
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
        std::cout << "id: " << curId_ << std::endl;
        break;
    case Qt::Key_Left:
        curId_ = (curId_ - 1) < 0 ? (Tcws_.size() - 1) : (curId_ - 1);
        std::cout << "id: " << curId_ << std::endl;
        break;
    case Qt::Key_R:
        initMatrix();
        break;
    default:
        break;
    }
    QOpenGLWidget::keyPressEvent(event);
    if (event->key() == Qt::Key_Left || event->key() == Qt::Key_Right)
        previewTimer_.start(0);
    update();
}


void MyOpenGLWidgetTs::setAttribute()
{
    // 配置顶点属性
    m_program.enableAttributeArray(0);
    m_program.setAttributeBuffer(0, GL_FLOAT, 0, m_pointStep, m_vtxStep * sizeof(GLfloat));
    // 3. 配置VAO/VBO，添加纹理坐标属性
    m_program.enableAttributeArray(1);
    m_program.setAttributeBuffer(1, GL_FLOAT, m_pointStep * sizeof(GLfloat), m_uvStep, m_vtxStep * sizeof(GLfloat));
    m_program.setUniformValue("ourTexture", 0); // 告诉着色器采样器用单元0, GL_TEXTURE0
}
void MyOpenGLWidgetTs::initTexture()
{
    // 2. 加载纹理图片
    const int sz = static_cast<int>(std::min(Tcws_.size(), comImgs_.size()));
    if (sz <= 0)
        return;
    previewImages_.resize(sz);
    textures_.resize(sz, 0);
    glGenTextures(sz, textures_.data());
    for (int i = 0; i < sz; ++i)
    {
        cv::Mat cvM;
        bool decoded = false;
        try { decoded = comImgs_[i].decompressCVMat(cvM); }
        catch (const std::exception&) { decoded = false; }
        if (!decoded)
            continue;
        QImage image;
        cvMat3CToQImageRGB32(cvM, image);
        QImage imgOpengl = image.convertToFormat(QImage::Format_RGBA8888).mirrored(false, true);
        genTexture2D(imgOpengl, textures_[i]);
    }
}
void MyOpenGLWidgetTs::updateMatrixAndUBO()
{
    //updateViewInit();
    QMatrix4x4 mTmp;
    mTmp.setToIdentity();
    mTmp.translate(m_rotationCenter);
    mTmp *= m_viewRotation;
    mTmp.translate(-m_rotationCenter);
    mTmp.scale(m_zoom);
    QMatrix4x4 pan;
    pan.translate(-m_detX, m_detY, 0.0f);
    mTmp = pan * mTmp;
    m_view = m_viewInit * mTmp;
    COMMON_LYJ::Pose3D T = Tcws_[curId_];
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            m_model(i, j) = T.getR()(i, j);
        }
        m_model(i, 3) = T.gett()(i);
    }
    if (Tcws_.empty() || cams_.empty() || comImgs_.empty() || pValids_.empty() ||
        curId_ >= Tcws_.size() || curId_ >= pValids_.size())
        return;
    const size_t cameraId = cams_.size() == 1 ? 0 : std::min<size_t>(curId_, cams_.size() - 1);
    const QT_LYJ::ProjectorCamera& camera = cams_[cameraId];
    const int pVSz = static_cast<int>(std::min<size_t>(pValids_[curId_].size(),
        static_cast<size_t>(m_vSize / m_vtxStep)));
    for (int i = 0; i < pVSz; ++i)
    {
        if (pValids_[curId_][i])
            m_vertices[i * m_vtxStep + 3] = 1;
        else
            m_vertices[i * m_vtxStep + 3] = -1;
    }
    // 传递矩阵到Shader
    m_program.setUniformValue("model", m_model);
    m_program.setUniformValue("view", m_view);
    m_program.setUniformValue("projection", m_projection);
    m_program.setUniformValue("cameraSize", QVector2D(camera.width, camera.height));
    m_program.setUniformValueArray("cameraParameters", camera.parameters.data(), 8, 1);
    m_program.setUniformValue("cameraModel", camera.model == QT_LYJ::ProjectorCameraModel::Fisheye ? 1 : 0);
    m_vbo.write(0, m_vertices, m_vSize * sizeof(float));
}
void MyOpenGLWidgetTs::drawFBO()
{
    glActiveTexture(GL_TEXTURE0); // 激活纹理单元0（默认）
    if (m_bDrawTexture && curId_ < textures_.size() && textures_[curId_] != 0)
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
}
