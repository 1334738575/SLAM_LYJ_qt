#include <QApplication>
#include <QPushButton>
#include <QVBoxLayout>
#include <OpenGLs/OpenGLWidget.h>

#include <IO/MeshIO.h>
#include <base/CameraModule.h>
#include <base/Pose.h>

#ifdef QT_LYJ_WITH_CUDA
#include <CUDAInclude.h>
#endif
#ifdef QT_LYJ_WITH_VULKAN
#include <VulkanInclude.h>
#endif

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

#include "QT_LYJ.h"
#include "OpenGLs/OpenGLTest.h"
#include "OpenGLs/OpenGLWidgetMesh.h"
#include "Windows/WindowsLyj.h"
#include "Windows/WindowsMatch3D.h"
#include "Windows/WindowsMatch.h"

NSP_QT_LYJ_BEGIN

ProjectorCamera::ProjectorCamera(const COMMON_LYJ::PinholeCamera& camera)
	: model(ProjectorCameraModel::Pinhole), width(camera.wide()), height(camera.height())
{
	parameters[0] = static_cast<float>(camera.fx());
	parameters[1] = static_cast<float>(camera.fy());
	parameters[2] = static_cast<float>(camera.cx());
	parameters[3] = static_cast<float>(camera.cy());
}

ProjectorCamera::ProjectorCamera(ProjectorCameraModel cameraModel, int imageWidth, int imageHeight,
	const std::vector<double>& cameraParameters)
	: model(cameraModel), width(imageWidth), height(imageHeight)
{
	const size_t parameterCount = model == ProjectorCameraModel::Fisheye ? 8u : 4u;
	if (cameraParameters.size() != parameterCount)
		throw std::invalid_argument(model == ProjectorCameraModel::Fisheye
			? "fisheye camera requires fx, fy, cx, cy, k1, k2, k3, k4"
			: "pinhole camera requires fx, fy, cx, cy");
	std::transform(cameraParameters.begin(), cameraParameters.end(), parameters.begin(),
		[](double value) { return static_cast<float>(value); });
}

static int testButton()
{
	int argc = 0;
	char** argv = nullptr;
	QApplication app(argc, argv);
	QPushButton button("Hello, Qt!");
	button.resize(200, 100);
	button.show();
	return app.exec();
}
static int testLabel()
{
	int argc = 0;
	char** argv = nullptr;
	QApplication app(argc, argv);
	QLabel label("Hello, Qt!");
	label.resize(200, 100);
	label.show();
	return app.exec();
}
static int testImage()
{
	int argc = 0;
	char** argv = nullptr;
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
		resize(_w, _h);

		if (bPly)
			openGLWidget_ = new OpenGLWidgetPly(_w, _h, this);
		else
			openGLWidget_ = new OpenGLWidgetObj(_w, _h, this);
		layout_ = new QVBoxLayout(this);
		layout_->setContentsMargins(0, 0, 0, 0);
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
	char** argv = nullptr;
	QApplication app(argc, argv);

	if (justOpenGL)
	{
		OpenGLWidget* w = new OpenGLWidget();
		w->resize(800, 600);
		w->show();
		return app.exec();
	}

	if (openGLAndBottons)
	{
		QWidget window;
		window.setWindowTitle("QT_LYJ");
		// qgridlayout qformlayout
		QVBoxLayout* layout = new QVBoxLayout(&window);

		OpenGLWidget* w = new OpenGLWidget();
		layout->addWidget(w);

		QPushButton* button = new QPushButton("move");
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
		QVBoxLayout* layout = new QVBoxLayout(&window);

		QPushButton* button = new QPushButton("open ply");
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
				OpenGLWindow* w = new OpenGLWindow(true, 1600, 1200, "Show mesh or obj");
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
				std::vector<Eigen::Vector3f> newPs(uvs.size(), Eigen::Vector3f(0, 0, 0));
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
		resize(_w, _h);

		openGLWidgetTs_ = new MyOpenGLWidgetTs(_w, _h, this);
		layout_ = new QVBoxLayout(this);
		layout_->setContentsMargins(0, 0, 0, 0);
		layout_->addWidget(openGLWidgetTs_);
	}

	void changeMesh(float* _vtcs, unsigned long long _vSz, unsigned int* _inds, unsigned long long _iSz,
		const std::vector<COMMON_LYJ::Pose3D>& _Tcws,
		const std::vector<ProjectorCamera>& _cams,
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

namespace
{
	bool failProjection(const std::string& message, std::string* errorMessage)
	{
		if (errorMessage)
			*errorMessage = message;
		return false;
	}

	float finiteVulkanMaxDepth(const COMMON_LYJ::BaseTriMesh& mesh,
		const Eigen::Matrix<float, 3, 4>& Tcw, float requestedMaxDepth, float minDepth)
	{
		if (std::isfinite(requestedMaxDepth) &&
			requestedMaxDepth < std::numeric_limits<float>::max() * 0.5f)
			return requestedMaxDepth;
		float maxDepth = minDepth;
		for (const Eigen::Vector3f& point : mesh.getVertexs())
		{
			const float depth = Tcw.row(2).head<3>().dot(point) + Tcw(2, 3);
			maxDepth = std::max(maxDepth, depth);
		}
		return std::max(minDepth * 2.0f, maxDepth * 1.01f + 1.0f);
	}
}

QT_LYJ_API bool projectMeshVisibility(
	const COMMON_LYJ::BaseTriMesh& mesh,
	const std::vector<COMMON_LYJ::Pose3D>& Tcws,
	const std::vector<ProjectorCamera>& cameras,
	std::vector<COMMON_LYJ::BitFlagVec>& pointVisibility,
	const ProjectionOptions& options,
	std::string* errorMessage)
{
	if (mesh.getVn() == 0 || mesh.getFn() == 0)
		return failProjection("mesh must contain vertices and faces", errorMessage);
	if (Tcws.empty())
		return failProjection("at least one pose is required", errorMessage);
	if (cameras.size() != 1 && cameras.size() != Tcws.size())
		return failProjection("camera count must be one or match the pose count", errorMessage);

	COMMON_LYJ::BaseTriMesh preparedMesh = mesh;
	preparedMesh.enableFCenters();
	preparedMesh.calculateFCenters();
	preparedMesh.enableFNormals();
	preparedMesh.calculateFNormals();
	const unsigned int pointCount = preparedMesh.getVn();
	const unsigned int faceCount = preparedMesh.getFn();
	pointVisibility.assign(Tcws.size(), COMMON_LYJ::BitFlagVec(static_cast<int>(pointCount)));

	for (size_t index = 0; index < Tcws.size(); ++index)
	{
		const ProjectorCamera& camera = cameras[cameras.size() == 1 ? 0 : index];
		if (camera.width <= 0 || camera.height <= 0 || camera.parameters[0] <= 0.0f || camera.parameters[1] <= 0.0f)
			return failProjection("camera dimensions and focal lengths must be positive", errorMessage);

		Eigen::Matrix<float, 3, 4> Tcw;
		Tcw.block(0, 0, 3, 3) = Tcws[index].getR().cast<float>();
		Tcw.block(0, 3, 3, 1) = Tcws[index].gett().cast<float>();
		std::vector<float> depths(static_cast<size_t>(camera.width) * camera.height);
		std::vector<unsigned int> faceIds(depths.size(), UINT32_MAX);
		std::vector<char> visiblePoints(pointCount, 0);
		std::vector<char> visibleFaces(faceCount, 0);

		if (options.backend == ProjectorBackend::CUDA)
		{
#ifdef QT_LYJ_WITH_CUDA
			const CUDA_LYJ::CameraModel model = camera.model == ProjectorCameraModel::Fisheye
				? CUDA_LYJ::CameraModel::Fisheye : CUDA_LYJ::CameraModel::Pinhole;
			CUDA_LYJ::ProHandle handle = CUDA_LYJ::initProjector(
				preparedMesh.getVertexs()[0].data(), pointCount, preparedMesh.getFCenters()[0].data(),
				preparedMesh.getFNormals()[0].data(), preparedMesh.getFaces()[0].vId_, faceCount,
				const_cast<float*>(camera.parameters.data()), camera.width, camera.height, model);
			if (!handle)
				return failProjection("failed to initialize CUDA projector", errorMessage);
			CUDA_LYJ::ProjectorCache cache(pointCount, faceCount, camera.width, camera.height);
			CUDA_LYJ::project(handle, cache, Tcw.data(), depths.data(), faceIds.data(),
				visiblePoints.data(), visibleFaces.data(), options.minDepth, options.maxDepth,
				options.normalCosineThreshold, options.visibilityDepthThreshold);
			CUDA_LYJ::release(handle);
#else
			return failProjection("QT_LYJ was built without CUDA projector support", errorMessage);
#endif
		}
		else
		{
#ifdef QT_LYJ_WITH_VULKAN
			const LYJ_VK::CameraModel model = camera.model == ProjectorCameraModel::Fisheye
				? LYJ_VK::CameraModel::Fisheye : LYJ_VK::CameraModel::Pinhole;
			LYJ_VK::ProVKHandle handle = LYJ_VK::initProjectorVK(
				preparedMesh.getVertexs()[0].data(), pointCount, preparedMesh.getFCenters()[0].data(),
				preparedMesh.getFNormals()[0].data(), preparedMesh.getFaces()[0].vId_, faceCount,
				const_cast<float*>(camera.parameters.data()), camera.width, camera.height, model);
			if (!handle)
				return failProjection("failed to initialize Vulkan projector", errorMessage);
			LYJ_VK::ProVKCacheHandle cache = LYJ_VK::initProjectorVKCache(handle);
			if (!cache)
			{
				LYJ_VK::releaseVK(handle);
				return failProjection("failed to initialize Vulkan projector cache", errorMessage);
			}
			const float maxDepth = finiteVulkanMaxDepth(preparedMesh, Tcw, options.maxDepth, options.minDepth);
			LYJ_VK::projectVK(handle, cache, Tcw.data(), depths.data(), faceIds.data(),
				visiblePoints.data(), visibleFaces.data(), options.minDepth, maxDepth,
				options.normalCosineThreshold, options.visibilityDepthThreshold);
			LYJ_VK::releaseProjectorVKCache(cache);
			LYJ_VK::releaseVK(handle);
#else
			return failProjection("QT_LYJ was built without Vulkan projector support", errorMessage);
#endif
		}

		for (unsigned int pointIndex = 0; pointIndex < pointCount; ++pointIndex)
			if (visiblePoints[pointIndex])
				pointVisibility[index].setFlag(pointIndex, true);
	}
	if (errorMessage)
		errorMessage->clear();
	return true;
}

QT_LYJ_API int testTcws(int argc, char* argv[],
	const COMMON_LYJ::BaseTriMesh& mesh,
	const std::vector<COMMON_LYJ::Pose3D>& Tcws,
	const std::vector<ProjectorCamera>& cameras,
	const std::vector<COMMON_LYJ::CompressedImage>& compressedImages,
	ProjectorBackend backend)
{
	if (compressedImages.size() != Tcws.size())
	{
		std::cerr << "Projection failed: compressed image count must match the pose count" << std::endl;
		return 1;
	}
	std::vector<COMMON_LYJ::BitFlagVec> pointVisibility;
	ProjectionOptions options;
	options.backend = backend;
	std::string error;
	if (!projectMeshVisibility(mesh, Tcws, cameras, pointVisibility, options, &error))
	{
		std::cerr << "Projection failed: " << error << std::endl;
		return 1;
	}


	QApplication app(argc, argv);
	QWidget window;
	window.setWindowTitle("QT_LYJ");
	QVBoxLayout* layout = new QVBoxLayout(&window);

	QPushButton* button = new QPushButton("open ply");
	layout->addWidget(button);
	QObject::connect(button, &QPushButton::clicked, [&]()
		{
			OpenGLWindowTs* w = new OpenGLWindowTs(1600, 1200, "Show mesh or obj");
			w->changeMesh(mesh.getVertexs()[0].data(), mesh.getVn(), mesh.getFaces()[0].vId_, mesh.getFn(), Tcws, cameras, compressedImages, pointVisibility);
			w->show();
		});

	window.setLayout(layout);
	window.resize(800, 600);
	window.show();
	app.exec();

	return 0;
}

QT_LYJ_API int testTcws(int argc, char* argv[],
	const COMMON_LYJ::BaseTriMesh& mesh,
	const std::vector<COMMON_LYJ::Pose3D>& Tcws,
	const std::vector<COMMON_LYJ::PinholeCamera>& cameras,
	const std::vector<COMMON_LYJ::CompressedImage>& compressedImages)
{
	std::vector<ProjectorCamera> projectorCameras;
	projectorCameras.reserve(cameras.size());
	for (const COMMON_LYJ::PinholeCamera& camera : cameras)
		projectorCameras.emplace_back(camera);
#ifdef QT_LYJ_WITH_CUDA
	const ProjectorBackend backend = ProjectorBackend::CUDA;
#else
	const ProjectorBackend backend = ProjectorBackend::Vulkan;
#endif
	return testTcws(argc, argv, mesh, Tcws, projectorCameras, compressedImages, backend);
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
