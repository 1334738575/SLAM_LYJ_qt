#include <QApplication>
#include <QPushButton>
#include <QVBoxLayout>
#include <OpenGLs/OpenGLWidget.h>

#include <IO/MeshIO.h>

#include "QT_LYJ.h"
#include "OpenGLs/OpenGLTest.h"
#include "OpenGLs/OpenGLWidget2.h"
#include "Windows/WindowsLyj.h"
#include "Windows/WindowsMatch3D.h"
#include "Windows/WindowsMatch.h"

NSP_QT_LYJ_BEGIN

static int testButton()
{
	int argc = 0;
	char **argv = nullptr;
	QApplication app(argc, argv);
	QPushButton button("Hello, Qt!");
	button.resize(200, 100);
	button.show();
	return app.exec();
}
static int testLabel()
{
	int argc = 0;
	char **argv = nullptr;
	QApplication app(argc, argv);
	QLabel label("Hello, Qt!");
	label.resize(200, 100);
	label.show();
	return app.exec();
}
static int testImage()
{
	int argc = 0;
	char **argv = nullptr;
	QApplication app(argc, argv);
	QLabel label;
	QPixmap pixmap("D:/testLyj/build/Release/down.png");
	label.setPixmap(pixmap);
	label.resize(pixmap.size());
	label.show();
	return app.exec();
}
static int testWindow(int argc, char* argv[])
{
	QApplication app(argc, argv);
	//WindowsLyj window;
	//WindowsMatch3D window;
	WindowsMatch window;
	window.show();
	return app.exec();
}

class OpenGLWindow : public QDialog
{
public:
	OpenGLWindow(int _w = 800, int _h = 600, std::string _title = "OpenGL Window", QWidget* parent = nullptr) : QDialog(parent)
	{
		setWindowTitle(QString::fromStdString(_title));
		setFixedSize(_w, _h);

		openGLWidget_ = new MyOpenGLWidget(this);
		layout_ = new QVBoxLayout(this);
		layout_->addWidget(openGLWidget_);
	}

	void changeMesh(float* _vtcs, unsigned long long _vSz, unsigned int* _inds, unsigned long long _iSz)
	{
		openGLWidget_->setVertices(_vtcs, _vSz);
		openGLWidget_->setIndices(_inds, _iSz);
	}
	void changeObj(const float* const _vtcs, const float* const _uvs, const QImage& _img, unsigned long long _vSz, unsigned int* _inds, unsigned long long _iSz)
	{
		openGLWidget_->setVerticesTexture(_vtcs, _uvs, _img, _vSz);
		openGLWidget_->setIndices(_inds, _iSz);
	}

private:
	MyOpenGLWidget* openGLWidget_ = nullptr;
	QVBoxLayout* layout_ = nullptr;
};
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
static int testOpenGL()
{
	bool justOpenGL = false;
	bool openGLAndBottons = false;
	bool openGLByBotton = true;

	int argc = 0;
	char **argv = nullptr;
	QApplication app(argc, argv);

	if (justOpenGL)
	{
		OpenGLWidget *w = new OpenGLWidget();
		w->resize(800, 600);
		w->show();
		return app.exec();
	}

	if (openGLAndBottons)
	{
		QWidget window;
		window.setWindowTitle("QT_LYJ");
		// qgridlayout qformlayout
		QVBoxLayout *layout = new QVBoxLayout(&window);

		OpenGLWidget *w = new OpenGLWidget();
		layout->addWidget(w);

		QPushButton *button = new QPushButton("move");
		layout->addWidget(button);

		// QObject::connect(button, &QPushButton::clicked, w, &OpenGLWidget::print);
		QObject::connect(button, &QPushButton::clicked, [&]()
						 { w->addMove(); });
		window.setLayout(layout);
		window.resize(800, 600);
		window.show();
		return app.exec();
	}

	if (openGLByBotton)
	{
		QWidget window;
		window.setWindowTitle("QT_LYJ");
		QVBoxLayout *layout = new QVBoxLayout(&window);

		QPushButton *button = new QPushButton("open ply");
		layout->addWidget(button);
		QObject::connect(button, &QPushButton::clicked, [&]()
						 {
				QString plyPath = QFileDialog::getOpenFileName(&window, "Open PLY", "./", "PLY文件(*.ply)");
				if (plyPath.isEmpty()) {
					qDebug() << "plyPath is nullptr!";
					return;
				}
				std::string btmPath = plyPath.toStdString();
				if (btmPath.find(".ply") != (btmPath.size() - 4))
				{
					qDebug() << "open failed! ";
					return;
				}
				SLAM_LYJ::BaseTriMesh btm;
				SLAM_LYJ::readPLYMesh(btmPath, btm);
				OpenGLWindow *w = new OpenGLWindow(1600, 1200, "Show mesh or obj");
				w->changeMesh(btm.getVertexs()[0].data(), btm.getVn(), btm.getFaces()[0].vId_, btm.getFn());

				w->show();
						 });

		QPushButton* button2 = new QPushButton("open obj");
		layout->addWidget(button2);
		QObject::connect(button2, &QPushButton::clicked, [&]()
			{
				QString objPath = QFileDialog::getOpenFileName(&window, "Open OBJ", "./", "OBJ文件(*.obj)");
				if (objPath.isEmpty()) {
					qDebug() << "objPath is nullptr!";
					return;
				}
				std::string btmPath = objPath.toStdString();
				if (btmPath.find(".obj") != (btmPath.size() - 4))
				{
					qDebug() << "open failed! ";
					return;
				}
				SLAM_LYJ::BaseTriMesh obj;
				if (!SLAM_LYJ::readOBJMesh(btmPath, obj))
				{
					qDebug() << "read failed! ";
					return;
				}
				const auto& comImg = obj.getTexture();
				cv::Mat cvM;
				comImg.decompressCVMat(cvM);
				QImage image;
				cvMat3CToQImageRGB32(cvM, image);
				//QImage image("D:/SLAM_LYJ_Packages/SLAM_LYJ_qt/tmp/111.png");

				//// 4. 使用QImage（显示/保存/处理）
				//qDebug() << "图像宽：" << image.width() << " 高：" << image.height();
				//qDebug() << "图像格式：" << image.format();

				const auto& ps = obj.getVertexs();
				const auto& fs = obj.getFaces();
				const auto& uvs = obj.getTextureCoords();
				const auto& triUVs = obj.getTriUVs();
				std::vector<Eigen::Vector3f> newPs(uvs.size(), Eigen::Vector3f(0,0,0));
				for (int i = 0; i < fs.size(); ++i)
				{
					const auto& uvIds = triUVs[i].uvId_;
					const auto& vIds = fs[i].vId_;
					for (int j = 0; j < 3; ++j)
					{
						newPs[uvIds[j]] = ps[vIds[j]];
					}
				}
				OpenGLWindow* w = new OpenGLWindow(1600, 1200, "Show mesh or obj");
				w->changeObj(newPs[0].data(), uvs[0].data(), image, uvs.size(), const_cast<uint32_t*>(triUVs[0].uvId_), fs.size());

				w->show();
			});


		window.setLayout(layout);
		window.resize(800, 600);
		window.show();
		return app.exec();
	}

	return app.exec();
}

int testQT(int argc, char* argv[])
{
	// testButton();
	// testLabel();
	// testImage();
	 //testWindow(argc, argv);
	 testOpenGL();

	return 1;
}
QT_LYJ_API int testOpenGLOnly()
{
	return testGL();
}



QT_LYJ_API void debugWindows(int argc, char* argv[])
{
	QApplication app(argc, argv);
	WindowsMatch window;
	window.show();
	app.exec();
}


NSP_QT_LYJ_END