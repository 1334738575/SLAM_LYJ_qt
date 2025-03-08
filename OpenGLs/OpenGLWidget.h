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
#include <array>
#include <vector>


class OpenGLWidgetLyj : public QOpenGLWidget, protected QOpenGLFunctions
{
	Q_OBJECT

public:
	OpenGLWidgetLyj(QWidget* parent = nullptr);
	~OpenGLWidgetLyj();

	void addPoint(const float x, const float y, const float z);
	void addPoint(const QVector3D& p);
	void addLine(const int p1, const int p2);
	void addTriangle(const int p1, const int p2, const int p3);
	void addQuad(const int p1, const int p2, const int p3, const int p4);
	void addPolygon(const std::vector<int>& points);

	void setPoints(const float* points, const int size);
	void setPoints(const std::vector<QVector3D>& ps);
	void setLines(const int* lines, const int size);
	void setTriangles(const int* triangles, const int size);
	void setQuads(const int* quads, const int size);
	void setPolygons(const std::vector<std::vector<int>>& polygons);

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
	struct LineInds
	{
		LineInds(){}
		LineInds(int i1, int i2) : ind1(i1), ind2(i2) {}
		int operator[](int i) const { return i == 0 ? ind1 : ind2; }
		int& operator[](int i) { return i == 0 ? ind1 : ind2; }
		int ind1 = -1;
		int ind2 = -1;
	};
	struct TriangleInds
	{
		TriangleInds(){}
		TriangleInds(int i1, int i2, int i3) : ind1(i1), ind2(i2), ind3(i3) {}
		int operator[](int i) const { return i == 0 ? ind1 : i == 1 ? ind2 : ind3; }
		int& operator[](int i) { return i == 0 ? ind1 : i == 1 ? ind2 : ind3; }
		int ind1 = -1;
		int ind2 = -1;
		int ind3 = -1;
	};
	struct QuadInds
	{
		QuadInds(){}
		QuadInds(int i1, int i2, int i3, int i4) : ind1(i1), ind2(i2), ind3(i3), ind4(i4) {}
		int operator[](int i) const { return i == 0 ? ind1 : i == 1 ? ind2 : i == 2 ? ind3 : ind4; }
		int& operator[](int i) { return i == 0 ? ind1 : i == 1 ? ind2 : i == 2 ? ind3 : ind4; }
		int ind1 = -1;
		int ind2 = -1;
		int ind3 = -1;
		int ind4 = -1;
	};
	bool m_enablePoint = true;
	bool m_enableLine = true;
	bool m_enableTriangle = true;
	bool m_enableQuad = true;
	bool m_enablePolygon = true;
	std::vector<QVector3D> m_points; // 存储三维点的数组
	std::vector<LineInds> m_lines;
	std::vector<TriangleInds> m_triangles;
	std::vector<QuadInds> m_quads;
	std::vector<std::vector<int>> m_polygons;
	float m_xRot = 0;              // 旋转角度
	float m_yRot = 0;              // 旋转角度
	float m_scale = 0;             // 缩放比例
	float m_detX = 0;             // 缩放比例
	float m_detY = 0;             // 缩放比例
	float m_detZ = 0;             // 缩放比例
	bool m_isPressLeft;          // 鼠标是否按下
	bool m_isPressRight;          // 鼠标是否按下
	QPoint m_lastPos;          // 上一个鼠标位置
};



/**********************************test****************************************/

class OpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    OpenGLWidget(QWidget *parent = nullptr);
    ~OpenGLWidget();

	void addMove(float x=0.1, float y=0.1, float z=0.1);

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