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
#include <QTimer>
#include <array>
#include <vector>
#include <Eigen/Core>
#include <Eigen/Eigen>

namespace QT_LYJ 
{

	int64_t imagePair2Int64(int _i1, int _i2);
	std::pair<int, int> int642TwoImagePair(int64_t _pair);
	void transform(const float* _Rwc, const float* _twc,
		const float& _xc, const float& _yc, const float& _zc,
		float& _xw, float& _yw, float& _zw);
	void transformNormal(const float* _Rwc,
		const float& _nxc, const float& _nyc, const float& _nzc,
		float& _nxw, float& _nyw, float& _nzw);

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
		void setTexture(QImage& _img);

		void setMatchData(int _iter,
			int _frameSize,
			int _enableNormal,
			int _enableColor,
			std::vector<int>* _allPsSize,
			std::vector<std::vector<Eigen::Vector3f>>* _allPoints,
			std::vector<std::vector<Eigen::Vector3f>>* _allNormals,
			std::vector<std::vector<Eigen::Vector3f>>* _allColors,
			std::vector<std::map<int64_t, std::vector<Eigen::Vector2i>>>* _allCorrs,
			std::vector<std::vector<Eigen::Matrix3f>>* _allFrameRwcs,
			std::vector<std::vector<Eigen::Vector3f>>* _allFrametwcs);
		void changeIter(int _iter);
		void setPrintFunc(std::function<void(const std::string&)> _printFunc);

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
			LineInds() {}
			LineInds(int i1, int i2) : ind1(i1), ind2(i2) {}
			int operator[](int i) const { return i == 0 ? ind1 : ind2; }
			int& operator[](int i) { return i == 0 ? ind1 : ind2; }
			int ind1 = -1;
			int ind2 = -1;
		};
		struct TriangleInds
		{
			TriangleInds() {}
			TriangleInds(int i1, int i2, int i3) : ind1(i1), ind2(i2), ind3(i3) {}
			int operator[](int i) const { return i == 0 ? ind1 : i == 1 ? ind2 : ind3; }
			int& operator[](int i) { return i == 0 ? ind1 : i == 1 ? ind2 : ind3; }
			int ind1 = -1;
			int ind2 = -1;
			int ind3 = -1;
		};
		struct QuadInds
		{
			QuadInds() {}
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
		QOpenGLTexture* m_texture = nullptr; //纹理
		float m_xRot = 0;              // 旋转角度
		float m_yRot = 0;              // 旋转角度
		float m_scale = 0;             // 缩放比例
		float m_detX = 0;             // 缩放比例
		float m_detY = 0;             // 缩放比例
		float m_detZ = 0;             // 缩放比例
		bool m_isPressLeft;          // 鼠标是否按下
		bool m_isPressRight;          // 鼠标是否按下
		QPoint m_lastPos;          // 上一个鼠标位置

	public:
		int m_curIter = 0;
		int m_iter = 10;
		int m_frameSize = 0;
		int m_enableNormal = 0;
		int m_enableColor = 0;

		std::vector<int> m_allPsSize;
		std::vector<std::vector<Eigen::Vector3f>> m_allPoints;
		std::vector<std::vector<Eigen::Vector3f>> m_allNormals;
		std::vector<std::vector<Eigen::Vector3f>> m_allColors;
		//std::vector<std::vector<int64_t>> m_allPairs;
		//std::vector<std::vector<std::vector<Eigen::Vector2i>>> m_allCorrs;
		std::vector<std::map<int64_t, std::vector<Eigen::Vector2i>>> m_allCorrs;
		std::vector<std::vector<Eigen::Matrix3f>> m_allFrameRwcs;
		std::vector<std::vector<Eigen::Vector3f>> m_allFrametwcs;

		std::function<void(const std::string&)> m_printFunc = nullptr; //输出debug信息
	};



	/**********************************test****************************************/

	class OpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
	{
		Q_OBJECT

	public:
		OpenGLWidget(QWidget* parent = nullptr);
		~OpenGLWidget();

		void addMove(float x = 0.1, float y = 0.1, float z = 0.1);

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
		float m_xRot = 0;              // 旋转角度
		float m_yRot = 0;              // 旋转角度
		float m_scale = 0;             // 缩放比例
		float m_detX = 0;             // 缩放比例
		float m_detY = 0;             // 缩放比例
		float m_detZ = 0;             // 缩放比例
		bool m_isPressLeft;          // 鼠标是否按下
		bool m_isPressRight;          // 鼠标是否按下
		QPoint m_lastPos;          // 上一个鼠标位置
		QOpenGLTexture* m_texture; // 纹理
	};

}

#endif // OPENGLWIDGET_H