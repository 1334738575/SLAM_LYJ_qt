// myopenglwidget.h
#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLBuffer>
#include <QMatrix4x4>
#include <QTimer>
#include <QOpenGLVertexArrayObject>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QOpenGLFramebufferObject>

#include <base/CameraModule.h>
#include <base/Pose.h>
#include <common/CompressedImage.h>
#include <common/CommonAlgorithm.h>

#include <opencv2/opencv.hpp>
#include <QOpenGLFunctions_4_3_Core>
class OpenGLWidgetMeshAbr : public QOpenGLWidget, protected QOpenGLFunctions_4_3_Core
{
    Q_OBJECT
public:
    explicit OpenGLWidgetMeshAbr(int _w, int _h, QWidget *parent = nullptr);
    ~OpenGLWidgetMeshAbr();

    /// <summary>
    /// 设置顶点
    /// </summary>
    /// <param name="_vtcs"指针></param>
    /// <param name="_sz"顶点数></param>
    virtual void setVertices(const float* const _vtcs, unsigned long long _sz)=0;
    /// <summary>
    /// 设置顶点及纹理
    /// </summary>
    /// <param name="_vtcs"></param>
    /// <param name="_uvs"></param>
    /// <param name="_img"></param>
    /// <param name="_sz"></param>
    virtual void setVerticesTexture(const float* const _vtcs, const float* const _uvs, const QImage& _img, unsigned long long _sz)=0;
    /// <summary>
    /// 设置面片
    /// </summary>
    /// <param name="_inds"指针></param>
    /// <param name="_sz"面片数></param>
    virtual void setIndices(unsigned int* _inds, unsigned long long _sz)=0;

protected:
    void initializeGL() override;
    void paintGL() override;
    void resizeGL(int w, int h) override;

    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

protected:
    void initProgram(const std::string& _vertPath, const std::string& _fragPath);
    void initFBO();
    void initVAO();
    void initMatrix();
    virtual void setAttribute() = 0;
    virtual void initTexture() = 0;
    void updateViewInit();
    virtual void updateMatrixAndUBO() = 0;
    void renderFBO();
    virtual void drawFBO() = 0;
    void renderWindows();


    void genTexture2D(QImage& _qImg, GLuint& _texId);
    void cvMat3CToQImageRGB32(const cv::Mat& mat, QImage& qimg);
private:
    void initJustRenderProgram();


protected:
    int m_w = 0;
    int m_h = 0;    
    std::string m_vPath;
    std::string m_fPath;

    QOpenGLShaderProgram m_program;
    QOpenGLBuffer m_vbo{ QOpenGLBuffer::VertexBuffer };
    QOpenGLBuffer m_ebo{QOpenGLBuffer::IndexBuffer};
    QOpenGLVertexArrayObject m_vao;
    QOpenGLFramebufferObject* m_fbo = nullptr;       // 自定义帧缓冲
    GLuint m_outColorId = 0;                            // 颜色纹理ID（GL_COLOR_ATTACHMENT0）
    GLuint m_outFaceId = 0;                             // 面片ID纹理ID（GL_COLOR_ATTACHMENT1）
    GLuint m_outDepthId = 0;                            // 深度纹理ID

    //just render to show
    QOpenGLShaderProgram m_screenShader; // 全屏绘制着色器
    QOpenGLVertexArrayObject m_screenVAO;            // 全屏四边形VAO
    QOpenGLBuffer m_screenVBO{ QOpenGLBuffer::VertexBuffer };                       // 全屏四边形VBO
    QOpenGLBuffer m_screenEBO{ QOpenGLBuffer::IndexBuffer };

    QMatrix4x4 m_projection;
    QMatrix4x4 m_viewInit;
    QMatrix4x4 m_view;
    QMatrix4x4 m_model;
    //float m_angle = 0.0f;
    //QTimer m_timer;
    QImage m_texture;
    GLuint m_textureID = 0;
    QImage m_textureDefault;
    GLuint m_textureIDDefault = 0;
    // 立方体顶点数据 (位置)
    std::vector<float> m_verticesDefault;
    std::vector<unsigned int> m_indicesDefault;
    std::vector<uchar> attch1;//减少内存碎片
    std::vector<unsigned int> fids;

    bool m_bDrawVertices = true;
    bool m_bDrawFaces = false;
    bool m_bDrawTexture = false;

    float* m_vertices = nullptr;
    int m_vSize = 0;
    unsigned int* m_indices = nullptr;
    int m_iSize = 0;


    float m_detXRot = 0;              // 旋转角度x，绑定鼠标左键的横向
    float m_detYRot = 0;              // 旋转角度y，绑定鼠标左键的纵向
    float m_detX = 0;             // X移动，绑定鼠标右键的横向
    float m_detY = 0;             // Y移动，绑定鼠标右键的纵向
    float m_detZ = 0;             // Z移动，绑定鼠标滚轴
    bool m_isPressLeft = false;          // 鼠标是否按下
    bool m_isPressRight = false;          // 鼠标是否按下
    QPoint m_lastPos;          // 上一个鼠标位置
};


class OpenGLWidgetPly : public OpenGLWidgetMeshAbr
{
public:
    explicit OpenGLWidgetPly(int _w, int _h, QWidget* parent = nullptr);
    ~OpenGLWidgetPly();

    void setVertices(const float* const _vtcs, unsigned long long _sz) override;
    void setVerticesTexture(const float* const _vtcs, const float* const _uvs, const QImage& _img, unsigned long long _sz) override;
    void setIndices(unsigned int* _inds, unsigned long long _sz) override;

protected:

    virtual void setAttribute();
    virtual void initTexture();
    virtual void updateMatrixAndUBO();
    virtual void drawFBO();



protected:
    int m_vtxStep = 0;
    int m_pointStep = 0;
    int m_uvStep = 0;
};

class OpenGLWidgetObj : public OpenGLWidgetPly
{
public:
    explicit OpenGLWidgetObj(int _w, int _h, QWidget* parent = nullptr);
    ~OpenGLWidgetObj();

    void setVerticesTexture(const float* const _vtcs, const float* const _uvs, const QImage& _img, unsigned long long _sz) override;

protected:
    virtual void setAttribute();
    virtual void initTexture();
    virtual void drawFBO();

};

class MyOpenGLWidgetTs :public OpenGLWidgetObj
{
public:
    explicit MyOpenGLWidgetTs(int _w, int _h, QWidget* parent = nullptr);
    ~MyOpenGLWidgetTs();

    void setData(const std::vector<SLAM_LYJ::Pose3D>& _Tcws,
        const std::vector<SLAM_LYJ::PinholeCamera>& _cams,
        const std::vector<COMMON_LYJ::CompressedImage>& _comImgs,
        const std::vector<SLAM_LYJ::SLAM_LYJ_MATH::BitFlagVec>& _pValids);

protected:
    void keyPressEvent(QKeyEvent* event) override;
    
    
    void setAttribute() override;
    void initTexture() override;
    void updateMatrixAndUBO() override;
    void drawFBO()override;


protected:
    std::vector<SLAM_LYJ::Pose3D> Tcws_;
    std::vector<SLAM_LYJ::PinholeCamera> cams_;
    std::vector<COMMON_LYJ::CompressedImage*> comImgs_;
    std::vector<SLAM_LYJ::SLAM_LYJ_MATH::BitFlagVec*> pValids_;
    int curId_ = 0;
    std::vector<uint> pValidTmp_;
    std::vector<GLuint> textures_;
    GLuint uboId; // UBO缓冲区ID
};

