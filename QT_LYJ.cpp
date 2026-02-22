#include <QApplication>
#include <QPushButton>
#include <QVBoxLayout>
#include <OpenGLs/OpenGLWidget.h>

#include <IO/MeshIO.h>
#include <base/CameraModule.h>
#include <base/Pose.h>

#include <CUDAInclude.h>

#include "QT_LYJ.h"
#include "OpenGLs/OpenGLTest.h"
#include "OpenGLs/OpenGLWidgetMesh.h"
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
	OpenGLWindow(bool bPly, int _w = 800, int _h = 600, std::string _title = "OpenGL Window", QWidget* parent = nullptr) : QDialog(parent)
	{
		setWindowTitle(QString::fromStdString(_title));
		setFixedSize(_w, _h);

		if(bPly)
			openGLWidget_ = new OpenGLWidgetPly(_w, _h, this);
		else
			openGLWidget_ = new OpenGLWidgetObj(_w, _h, this);
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
	OpenGLWidgetMeshAbr* openGLWidget_ = nullptr;
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
				COMMON_LYJ::BaseTriMesh btm;
				COMMON_LYJ::readPLYMesh(btmPath, btm);
				OpenGLWindow *w = new OpenGLWindow(true, 1600, 1200, "Show mesh or obj");
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
				COMMON_LYJ::BaseTriMesh obj;
				if (!COMMON_LYJ::readOBJMesh(btmPath, obj))
				{
					qDebug() << "read failed! ";
					return;
				}
				const auto& comImg = obj.getTexture();
				cv::Mat cvM;
				comImg.decompressCVMat(cvM);
				QImage image;
				cvMat3CToQImageRGB32(cvM, image);

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
				OpenGLWindow* w = new OpenGLWindow(false, 1600, 1200, "Show mesh or obj");
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



class OpenGLWindowTs : public QDialog
{
public:
	OpenGLWindowTs(int _w = 800, int _h = 600, std::string _title = "OpenGL Window Ts", QWidget* parent = nullptr) : QDialog(parent)
	{
		setWindowTitle(QString::fromStdString(_title));
		setFixedSize(_w, _h);

		openGLWidgetTs_ = new MyOpenGLWidgetTs(_w, _h, this);
		layout_ = new QVBoxLayout(this);
		layout_->addWidget(openGLWidgetTs_);
	}

	void changeMesh(float* _vtcs, unsigned long long _vSz, unsigned int* _inds, unsigned long long _iSz,
		const std::vector<COMMON_LYJ::Pose3D>& _Tcws,
		const std::vector<COMMON_LYJ::PinholeCamera>& _cams,
		const std::vector<COMMON_LYJ::CompressedImage>& _comImgs,
		const std::vector<COMMON_LYJ::BitFlagVec>& _pValids)
	{
		openGLWidgetTs_->setVertices(_vtcs, _vSz);
		openGLWidgetTs_->setIndices(_inds, _iSz);
		openGLWidgetTs_->setData(_Tcws, _cams, _comImgs, _pValids);
	}

private:
	MyOpenGLWidgetTs* openGLWidgetTs_ = nullptr;
	QVBoxLayout* layout_ = nullptr;
};
QT_LYJ_API int testTcws(int argc, char* argv[],
	const COMMON_LYJ::BaseTriMesh& _btm,
	const std::vector<COMMON_LYJ::Pose3D>& _Tcws, const std::vector<COMMON_LYJ::PinholeCamera>& _cams, const std::vector<COMMON_LYJ::CompressedImage>& _comImgs)
{
	COMMON_LYJ::BaseTriMesh btm = _btm;
	std::vector<Eigen::Vector3f> vertexs = btm.getVertexs();
	std::vector<COMMON_LYJ::BaseTriFace> fs = btm.getFaces();
	btm.enableFCenters();
	btm.calculateFCenters();
	btm.enableFNormals();
	btm.calculateFNormals();
	int sz = _Tcws.size();
	const auto& cam = _cams[0];
	int w = cam.wide();
	int h = cam.height();
	std::vector<float> ccc{ static_cast<float>(cam.fx()), static_cast<float>(cam.fy()), static_cast<float>(cam.cx()), static_cast<float>(cam.cy()) };

	float* Pws = vertexs[0].data();
	unsigned int PSize = btm.getVn();
	float* centers = btm.getFCenters()[0].data();
	float* fNormals = btm.getFNormals()[0].data();
	unsigned int* faces = fs[0].vId_;
	unsigned int fSize = btm.getFn();
	float* camParams = ccc.data();
	CUDA_LYJ::ProHandle proHandle = CUDA_LYJ::initProjector(Pws, PSize, centers, fNormals, faces, fSize, camParams, w, h);
	CUDA_LYJ::ProjectorCache cache;
	cache.init(PSize, fSize, w, h);

	std::vector<COMMON_LYJ::BitFlagVec> pValids(sz);
	for (int i = 0; i < sz; ++i)
	{
		pValids[i].assign(PSize, false);
	}
	for(int i=0;i<sz;++i)
	{
		const auto& pinCam = _cams[0];
		const COMMON_LYJ::Pose3D& TcwP = _Tcws[i];
		Eigen::Matrix<float, 3, 4> Tcw34;
	    Tcw34.block(0, 0, 3, 3) = TcwP.getR().cast<float>();
	    Tcw34.block(0, 3, 3, 1) = TcwP.gett().cast<float>();
	    float* Tcw = Tcw34.data();

	    cv::Mat depthsM(w, h, CV_32FC1);
	    float* depths = (float*)depthsM.data;
	    std::vector<char> allvisiblePIds(PSize, 0);
	    std::vector<char> allvisibleFIds(fSize, 0);
	    std::vector<unsigned int> fIdss(w * h, 0);
	    unsigned int* fIds = fIdss.data();
	    char* allVisiblePIds = allvisiblePIds.data();
	    char* allVisibleFIds = allvisibleFIds.data();

	    CUDA_LYJ::project(proHandle, cache, Tcw, depths, fIds, allVisiblePIds, allVisibleFIds, 0, FLT_MAX, 0.5, 0.01);

		for (int j = 0; j < PSize; ++j)
		{
			if (allvisiblePIds[j] == 1)
				pValids[i].setFlag(j, true);
		}
		//std::vector<Eigen::Vector3f> Pws;
		//for (int j = 0; j < PSize; ++j)
		//{
		//	if (pValids[i][j])
		//		Pws.push_back(vertexs[j]);
		//}
		//COMMON_LYJ::BaseTriMesh btmTmp;
		//btmTmp.setVertexs(Pws);
		//COMMON_LYJ::writePLYMesh("D:/tmp/pValid" + std::to_string(i) + ".ply", btmTmp);
	 //   cv::Mat depthsMShow(h, w, CV_8UC1);
	 //   depthsMShow.setTo(cv::Scalar(0));
	 //   std::vector<Eigen::Vector3f> Pcs;
	 //   Pcs.reserve(w * h);
	 //   for (int ii = 0; ii < h; ++ii)
	 //   {
	 //       for (int j = 0; j < w; ++j)
	 //       {
	 //           float d = depths[ii * w + j];
	 //           if (d == FLT_MAX)
	 //           {
	 //               depthsMShow.at<uchar>(ii, j) = 0;
	 //               continue;
	 //           }
	 //           Eigen::Vector3d Pc;
	 //           pinCam.image2World(j, ii, d, Pc);
	 //           Pcs.push_back(Pc.cast<float>());
	 //           depthsMShow.at<uchar>(ii, j) = d * 20 < 255 ? (char)(d * 20) : 255;
	 //       }
	 //   }
		//cv::imwrite("D:/tmp/" + std::to_string(i) + ".png", depthsMShow);
	 //   //cv::imshow("depth", depthsMShow);
	 //   //cv::waitKey();
		continue;
	}

	CUDA_LYJ::release(proHandle);


	QApplication app(argc, argv);
	QWidget window;
	window.setWindowTitle("QT_LYJ");
	QVBoxLayout* layout = new QVBoxLayout(&window);

	QPushButton* button = new QPushButton("open ply");
	layout->addWidget(button);
	QObject::connect(button, &QPushButton::clicked, [&]()
		{
			OpenGLWindowTs* w = new OpenGLWindowTs(1600, 1200, "Show mesh or obj");
			w->changeMesh(btm.getVertexs()[0].data(), btm.getVn(), btm.getFaces()[0].vId_, btm.getFn(), _Tcws, _cams, _comImgs, pValids);
			w->show();
		});

	window.setLayout(layout);
	window.resize(800, 600);
	window.show();
	app.exec();

	return 0;
}




QT_LYJ_API void debugWindows(int argc, char* argv[])
{
	QApplication app(argc, argv);
	WindowsMatch window;
	window.show();
	app.exec();
}

QT_LYJ_API int testOpenGLOnly()
{
	return testGL();
}
NSP_QT_LYJ_END