#ifndef OPENGLWIDGET_H
#define OPENGLWIDGET_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include <QVector3D>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QWheelEvent>
#include <QImage>
#include <QOpenGLTexture>

class OpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    OpenGLWidget(QWidget *parent = nullptr);
    ~OpenGLWidget();

protected:
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;

	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void keyPressEvent(QKeyEvent* event) override;
	void keyReleaseEvent(QKeyEvent* event) override;
	void wheelEvent(QWheelEvent* event) override;

private:
    QVector<QVector3D> points; // 存储三维点的数组
	float m_xRot;              // 旋转角度
	float m_yRot;              // 旋转角度
	bool m_isPressed;          // 鼠标是否按下
	QPoint m_lastPos;          // 上一个鼠标位置
	float m_scale;             // 缩放比例
	QOpenGLTexture* m_texture; // 纹理
};

#endif // OPENGLWIDGET_H