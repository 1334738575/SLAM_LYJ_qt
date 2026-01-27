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

class MyOpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    explicit MyOpenGLWidget(QWidget *parent = nullptr);
    ~MyOpenGLWidget();

    /// <summary>
    /// 设置顶点
    /// </summary>
    /// <param name="_vtcs"指针></param>
    /// <param name="_sz"顶点数></param>
    void setVertices(const float* const _vtcs, unsigned long long _sz);
    /// <summary>
    /// 设置顶点及纹理
    /// </summary>
    /// <param name="_vtcs"></param>
    /// <param name="_uvs"></param>
    /// <param name="_img"></param>
    /// <param name="_sz"></param>
    void setVerticesTexture(const float* const _vtcs, const float* const _uvs, const QImage& _img, unsigned long long _sz);
    /// <summary>
    /// 设置面片
    /// </summary>
    /// <param name="_inds"指针></param>
    /// <param name="_sz"面片数></param>
    void setIndices(unsigned int* _inds, unsigned long long _sz);

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

private:
    QOpenGLShaderProgram m_program;
    QOpenGLBuffer m_vbo{ QOpenGLBuffer::VertexBuffer };
    QOpenGLBuffer m_ebo{QOpenGLBuffer::IndexBuffer};
    QOpenGLVertexArrayObject m_vao;
    QMatrix4x4 m_projection;
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
    //std::vector<unsigned int> m_indicesDefault{
    //    0, 1, 2, 3, // 底面
    //    4, 5, 6, 7, // 顶面
    //    0, 1, 5, 4, // 左侧面
    //    2, 3, 7, 6, // 右侧面
    //    0, 3, 7, 4, // 前侧面
    //    1, 2, 6, 5 }; // 后侧面
    // 按三角形拆分的索引数组（6个面 × 2个三角形 × 3个索引 = 36个索引）
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