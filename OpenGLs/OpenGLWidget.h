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

	void addMove(float x=0.1, float y=0.1, float z=0.1);
	void print();

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
	float m_xRot=0;              // 旋转角度
	float m_yRot=0;              // 旋转角度
	float m_scale=0;             // 缩放比例
	float m_detX=0;             // 缩放比例
	float m_detY=0;             // 缩放比例
	float m_detZ=0;             // 缩放比例
	bool m_isPressLeft;          // 鼠标是否按下
	bool m_isPressRight;          // 鼠标是否按下
	QPoint m_lastPos;          // 上一个鼠标位置
	QOpenGLTexture* m_texture; // 纹理
};

#endif // OPENGLWIDGET_H