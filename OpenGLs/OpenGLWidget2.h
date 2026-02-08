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
class MyOpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions_4_3_Core
{
    Q_OBJECT
public:
    explicit MyOpenGLWidget(int _w, int _h, QWidget *parent = nullptr);
    ~MyOpenGLWidget();

    /// <summary>
    /// 设置顶点
    /// </summary>
    /// <param name="_vtcs"指针></param>
    /// <param name="_sz"顶点数></param>
    virtual void setVertices(const float* const _vtcs, unsigned long long _sz);
    /// <summary>
    /// 设置顶点及纹理
    /// </summary>
    /// <param name="_vtcs"></param>
    /// <param name="_uvs"></param>
    /// <param name="_img"></param>
    /// <param name="_sz"></param>
    virtual void setVerticesTexture(const float* const _vtcs, const float* const _uvs, const QImage& _img, unsigned long long _sz);
    /// <summary>
    /// 设置面片
    /// </summary>
    /// <param name="_inds"指针></param>
    /// <param name="_sz"面片数></param>
    virtual void setIndices(unsigned int* _inds, unsigned long long _sz);

protected:
    void initProgram(const std::string& _vertPath, const std::string& _fragPath);
    virtual void initFBO();
    virtual void initVAO();
    virtual void initTexture();
    void genTexture2D(QImage& _qImg, GLuint& _texId);
    void initJustRenderProgram();

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
    int m_vtxStep = 5;
    int m_pointStep = 3;
    int m_uvStep = 2;
    QImage m_texture;
    GLuint m_textureID = 0;
    QImage m_textureDefault;
    GLuint m_textureIDDefault = 0;

    bool m_bDrawVertices = true;
    bool m_bDrawFaces = false;
    bool m_bDrawTexture = false;

    float* m_vertices = nullptr;
    int m_vSize = 0;
    unsigned int* m_indices = nullptr;
    int m_iSize = 0;
    // 立方体顶点数据 (位置)
    std::vector<float> m_verticesDefault{
        //-0.5f, -0.5f, -0.5f,
        //0.5f, -0.5f, -0.5f,
        //0.5f, 0.5f, -0.5f,
        //-0.5f, 0.5f, -0.5f,
        //-0.5f, -0.5f, 0.5f,
        //0.5f, -0.5f, 0.5f,
        //0.5f, 0.5f, 0.5f,
        //-0.5f, 0.5f, 0.5f 
        -0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
        -0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  0.0f, 1.0f
    };
    std::vector<unsigned int> m_indicesDefault{
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


    float m_detXRot = 0;              // 旋转角度x，绑定鼠标左键的横向
    float m_detYRot = 0;              // 旋转角度y，绑定鼠标左键的纵向
    float m_detX = 0;             // X移动，绑定鼠标右键的横向
    float m_detY = 0;             // Y移动，绑定鼠标右键的纵向
    float m_detZ = 0;             // Z移动，绑定鼠标滚轴
    bool m_isPressLeft = false;          // 鼠标是否按下
    bool m_isPressRight = false;          // 鼠标是否按下
    QPoint m_lastPos;          // 上一个鼠标位置
};


class MyOpenGLWidgetTs :public MyOpenGLWidget
{
public:
    explicit MyOpenGLWidgetTs(int _w, int _h, QWidget* parent = nullptr);
    ~MyOpenGLWidgetTs();

    void setData(const std::vector<SLAM_LYJ::Pose3D>& _Tcws,
        const std::vector<SLAM_LYJ::PinholeCamera>& _cams,
        const std::vector<COMMON_LYJ::CompressedImage>& _comImgs,
        const std::vector<SLAM_LYJ::SLAM_LYJ_MATH::BitFlagVec>& _pValids);

    static void cvMat3CToQImageRGB32(const cv::Mat& mat, QImage& qimg) {
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

protected:
    void initVAO() override;
    void initTexture() override;


    void paintGL() override;

    void keyPressEvent(QKeyEvent* event) override;

    std::vector<SLAM_LYJ::Pose3D> Tcws_;
    std::vector<SLAM_LYJ::PinholeCamera> cams_;
    std::vector<COMMON_LYJ::CompressedImage*> comImgs_;
    std::vector<SLAM_LYJ::SLAM_LYJ_MATH::BitFlagVec*> pValids_;
    int curId_ = 0;
    std::vector<uint> pValidTmp_;
    std::vector<GLuint> textures_;
    GLuint uboId; // UBO缓冲区ID
};

