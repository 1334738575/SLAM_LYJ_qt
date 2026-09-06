#include <QApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFileSystemModel>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPixmap>
#include <QPushButton>
#include <QSpinBox>
#include <QTreeView>
#include <QVBoxLayout>
#include <OpenGLs/OpenGLWidget.h>

#include <IO/MeshIO.h>
#include <IO/SimpleIO.h>
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
#include <filesystem>
#include <fstream>
#include <stdexcept>

#include "QT_LYJ.h"
#include "OpenGLs/OpenGLProjector.h"
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

class TexturePreviewPanel : public QWidget
{
public:
	explicit TexturePreviewPanel(QWidget* parent = nullptr)
		: QWidget(parent)
	{
		imageLabel_ = new QLabel(this);
		imageLabel_->setAlignment(Qt::AlignCenter);
		imageLabel_->setMinimumSize(280, 240);
		imageLabel_->setStyleSheet("background-color: black;");
		infoLabel_ = new QLabel(this);
		infoLabel_->setAlignment(Qt::AlignCenter);
		infoLabel_->setText("No corresponding texture");
		QVBoxLayout* layout = new QVBoxLayout(this);
		layout->addWidget(imageLabel_, 1);
		layout->addWidget(infoLabel_);
		setMinimumWidth(280);
	}

	void setTexture(const QImage& image, int faceId, const QPointF& uv)
	{
		if (image.isNull())
		{
			imageLabel_->clear();
			infoLabel_->setText("No corresponding texture");
			return;
		}
		const QPixmap pixmap = QPixmap::fromImage(image).scaled(
			imageLabel_->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
		imageLabel_->setPixmap(pixmap);
		infoLabel_->setText(QString("Face %1   UV (%2, %3)")
			.arg(faceId).arg(uv.x(), 0, 'f', 5).arg(uv.y(), 0, 'f', 5));
	}

private:
	QLabel* imageLabel_ = nullptr;
	QLabel* infoLabel_ = nullptr;
};

class OpenGLWindow : public QDialog
{
public:
	OpenGLWindow(bool bPly, int _w = 800, int _h = 600, std::string _title = "OpenGL Window", QWidget* parent = nullptr) : QDialog(parent)
	{
		setAttribute(Qt::WA_DeleteOnClose);
		setWindowTitle(QString::fromStdString(_title));
		setWindowFlags(windowFlags() | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);
		resize(_w, _h);

		if (bPly)
			openGLWidget_ = new OpenGLWidgetPly(_w, _h, this);
		else
			openGLWidget_ = new OpenGLWidgetObj(_w, _h, this);
		texturePreviewPanel_ = new TexturePreviewPanel(this);
		if (!bPly)
		{
			static_cast<OpenGLWidgetObj*>(openGLWidget_)->setHoverTexturePreviewCallback(
				[this](const QImage& image, int faceId, const QPointF& uv)
				{
					texturePreviewPanel_->setTexture(image, faceId, uv);
				});
		}
		layout_ = new QHBoxLayout(this);
		layout_->setContentsMargins(0, 0, 0, 0);
		layout_->addWidget(openGLWidget_, 3);
		layout_->addWidget(texturePreviewPanel_, 1);
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
		openGLWidget_->setDrawFaces(true);
		openGLWidget_->setDrawTexture(true);
		openGLWidget_->setDrawVertices(false);
	}

private:
	OpenGLWidgetMeshAbr* openGLWidget_ = nullptr;
	QHBoxLayout* layout_ = nullptr;
	TexturePreviewPanel* texturePreviewPanel_ = nullptr;
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

static int testOpenGLUnified(int argc, char* argv[]);

int testQT(int argc, char* argv[])
{
	// testButton();
	// testLabel();
	// testImage();
	 //testWindow(argc, argv);
	return testOpenGLUnified(argc, argv);
}


class OpenGLWindowTs : public QDialog
{
public:
	OpenGLWindowTs(int _w = 800, int _h = 600, std::string _title = "OpenGL Window Ts", QWidget* parent = nullptr) : QDialog(parent)
	{
		setAttribute(Qt::WA_DeleteOnClose);
		setWindowTitle(QString::fromStdString(_title));
		setWindowFlags(windowFlags() | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);
		resize(_w, _h);

		openGLWidgetTs_ = new MyOpenGLWidgetTs(_w, _h, this);
		texturePreviewPanel_ = new TexturePreviewPanel(this);
		openGLWidgetTs_->setTexturePreviewCallback([this](const QImage& image, int faceId, const QPointF& uv)
			{
				texturePreviewPanel_->setTexture(image, faceId, uv);
			});
		layout_ = new QHBoxLayout(this);
		layout_->setContentsMargins(0, 0, 0, 0);
		layout_->addWidget(openGLWidgetTs_, 3);
		layout_->addWidget(texturePreviewPanel_, 1);
	}

	void changeMesh(const float* _vtcs, unsigned long long _vSz, const unsigned int* _inds, unsigned long long _iSz,
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
	QHBoxLayout* layout_ = nullptr;
	TexturePreviewPanel* texturePreviewPanel_ = nullptr;
};

class AssetManagerDialog : public QDialog
{
public:
	explicit AssetManagerDialog(QWidget* parent = nullptr)
		: QDialog(parent), model_(new QFileSystemModel(this)), tree_(new QTreeView(this))
	{
		setWindowTitle("Asset Manager");
		resize(760, 520);
		model_->setFilter(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot);
		model_->setRootPath(QDir::rootPath());
		tree_->setModel(model_);
		tree_->setRootIndex(model_->index(QDir::rootPath()));
		tree_->setSelectionMode(QAbstractItemView::SingleSelection);
		for (int column = 1; column < model_->columnCount(); ++column)
			tree_->hideColumn(column);
		QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
		connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
		connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
		QVBoxLayout* layout = new QVBoxLayout(this);
		layout->addWidget(tree_);
		layout->addWidget(buttons);
	}

	QString selectedPath() const
	{
		const QModelIndex index = tree_->currentIndex();
		return index.isValid() ? model_->filePath(index) : QString();
	}

private:
	QFileSystemModel* model_ = nullptr;
	QTreeView* tree_ = nullptr;
};

class TextureProjectionConfigDialog : public QDialog
{
public:
	explicit TextureProjectionConfigDialog(QWidget* parent = nullptr) : QDialog(parent)
	{
		setWindowTitle("Texture Projection");
		resize(720, 240);
		dataRoot_ = addPathRow("Image/camera directory", false);
		poseRoot_ = addPathRow("Pose directory", false);
		meshPath_ = addPathRow("PLY mesh file", true);
		frameLimit_ = new QSpinBox(this);
		frameLimit_->setRange(0, 1000000);
		frameLimit_->setSpecialValueText("All");
		QFormLayout* form = new QFormLayout;
		form->addRow("Image/camera directory", rowFor(dataRoot_));
		form->addRow("Pose directory", rowFor(poseRoot_));
		form->addRow("PLY mesh file", rowFor(meshPath_));
		form->addRow("Frame limit", frameLimit_);
		QDialogButtonBox* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
		connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
		connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
		QVBoxLayout* layout = new QVBoxLayout(this);
		layout->addLayout(form);
		layout->addWidget(buttons);
	}

	QString dataRoot() const { return dataRoot_->text().trimmed(); }
	QString poseRoot() const { return poseRoot_->text().trimmed(); }
	QString meshPath() const { return meshPath_->text().trimmed(); }
	int frameLimit() const { return frameLimit_->value(); }

private:
	QLineEdit* addPathRow(const QString&, bool file)
	{
		QLineEdit* edit = new QLineEdit(this);
		QWidget* row = new QWidget(this);
		QHBoxLayout* rowLayout = new QHBoxLayout(row);
		rowLayout->setContentsMargins(0, 0, 0, 0);
		rowLayout->addWidget(edit, 1);
		QPushButton* browse = new QPushButton("Browse", row);
		QPushButton* manager = new QPushButton("Asset Manager", row);
		rowLayout->addWidget(browse);
		rowLayout->addWidget(manager);
		connect(browse, &QPushButton::clicked, this, [this, edit, file]()
			{
				const QString path = file
					? QFileDialog::getOpenFileName(this, "Select mesh", edit->text(), "PLY files (*.ply);;All files (*.*)")
					: QFileDialog::getExistingDirectory(this, "Select directory", edit->text());
				if (!path.isEmpty())
					edit->setText(path);
			});
		connect(manager, &QPushButton::clicked, this, [this, edit]()
			{
				AssetManagerDialog dialog(this);
				if (dialog.exec() == QDialog::Accepted && !dialog.selectedPath().isEmpty())
					edit->setText(dialog.selectedPath());
			});
		edit->setProperty("pathRow", QVariant::fromValue(static_cast<void*>(row)));
		return edit;
	}

	QWidget* rowFor(QLineEdit* edit) const
	{
		return static_cast<QWidget*>(edit->property("pathRow").value<void*>());
	}

	QLineEdit* dataRoot_ = nullptr;
	QLineEdit* poseRoot_ = nullptr;
	QLineEdit* meshPath_ = nullptr;
	QSpinBox* frameLimit_ = nullptr;
};

static bool showPlyFile(const QString& path, QWidget* parent)
{
	COMMON_LYJ::BaseTriMesh mesh;
	COMMON_LYJ::readPLYMesh(path.toStdString(), mesh);
	if (mesh.getVn() == 0 || mesh.getFn() == 0)
	{
		QMessageBox::critical(parent, "PLY load failed", "The file has no valid vertices or faces.");
		return false;
	}
	// Mesh files used by the viewer are conventionally millimetres; use metres
	// for both PLY and OBJ so their displayed scale is comparable.
	std::vector<Eigen::Vector3f> displayPoints = mesh.getVertexs();
	for (Eigen::Vector3f& point : displayPoints)
		point /= 1000.0f;
	for (const auto& face : mesh.getFaces())
		for (int corner = 0; corner < 3; ++corner)
			if (face.vId_[corner] >= displayPoints.size())
			{
				QMessageBox::critical(parent, "PLY load failed", "The PLY contains an invalid face index.");
				return false;
			}
	OpenGLWindow* window = new OpenGLWindow(true, 1600, 1200, "PLY Viewer");
	window->changeMesh(displayPoints[0].data(), displayPoints.size(), mesh.getFaces()[0].vId_, mesh.getFn());
	window->show();
	return true;
}

static bool showObjFile(const QString& path, QWidget* parent)
{
	COMMON_LYJ::BaseTriMesh obj;
	bool loaded = false;
	try { loaded = COMMON_LYJ::readOBJMesh(path.toStdString(), obj); }
	catch (const std::exception& error)
	{
		QMessageBox::critical(parent, "OBJ load failed", QString::fromUtf8(error.what()));
		return false;
	}
	if (!loaded || obj.getVn() == 0 || obj.getFn() == 0 ||
		obj.getTextureCoords().empty() || obj.getTriUVs().size() != obj.getFaces().size())
	{
		QMessageBox::critical(parent, "OBJ load failed", "The OBJ or its texture coordinates are invalid.");
		return false;
	}
	cv::Mat cvImage;
	QImage image;
	try
	{
		if (obj.getTexture().decompressCVMat(cvImage))
			cvMat3CToQImageRGB32(cvImage, image);
	}
	catch (const std::exception&)
	{
		// MeshIO historically assumes <obj basename>.jpg.  If an MTL points to
		// another single texture, load that file as a fallback.
		std::ifstream mtl(std::filesystem::path(path.toStdString()).replace_extension(".mtl"));
		std::string key, texturePath;
		while (mtl >> key)
		{
			std::getline(mtl, texturePath);
			if (key == "map_Kd")
			{
				texturePath.erase(0, texturePath.find_first_not_of(" \t"));
				const auto resolved = std::filesystem::path(path.toStdString()).parent_path() / texturePath;
				image.load(QString::fromStdString(resolved.lexically_normal().string()));
				break;
			}
		}
	}
	if (!image.isNull())
		image = image.convertToFormat(QImage::Format_RGBA8888);
	if (image.isNull())
	{
		std::ifstream mtl(std::filesystem::path(path.toStdString()).replace_extension(".mtl"));
		std::string key, texturePath;
		while (mtl >> key)
		{
			std::getline(mtl, texturePath);
			if (key == "map_Kd")
			{
				texturePath.erase(0, texturePath.find_first_not_of(" \t"));
				const auto resolved = std::filesystem::path(path.toStdString()).parent_path() / texturePath;
				image.load(QString::fromStdString(resolved.lexically_normal().string()));
				break;
			}
		}
	}
	if (image.isNull())
	{
		QMessageBox::critical(parent, "OBJ load failed", "The OBJ texture image is missing or invalid.");
		return false;
	}
	const auto& points = obj.getVertexs();
	const auto& faces = obj.getFaces();
	const auto& uvs = obj.getTextureCoords();
	const auto& triUVs = obj.getTriUVs();
	std::vector<Eigen::Vector3f> expandedPoints;
	std::vector<Eigen::Vector2f> expandedUVs;
	std::vector<uint32_t> indices;
	expandedPoints.reserve(faces.size() * 3);
	expandedUVs.reserve(faces.size() * 3);
	indices.reserve(faces.size() * 3);
	for (size_t faceId = 0; faceId < faces.size(); ++faceId)
	{
		for (int corner = 0; corner < 3; ++corner)
		{
			const uint32_t pointId = faces[faceId].vId_[corner];
			const uint32_t uvId = triUVs[faceId].uvId_[corner];
			if (pointId >= points.size() || uvId >= uvs.size())
				return false;
			expandedPoints.push_back(points[pointId] / 1000.0f);
			expandedUVs.push_back(uvs[uvId]);
			indices.push_back(static_cast<uint32_t>(expandedPoints.size() - 1));
		}
	}
	OpenGLWindow* window = new OpenGLWindow(false, 1600, 1200, "OBJ Viewer");
	window->changeObj(expandedPoints[0].data(), expandedUVs[0].data(), image,
		expandedPoints.size(), indices.data(), indices.size() / 3);
	window->show();
	return true;
}

static bool readProjectionCamera(const std::filesystem::path& path, std::vector<double>& parameters)
{
	std::ifstream input(path);
	parameters.resize(4);
	for (double& value : parameters)
	{
		std::string name;
		std::string equals;
		if (!(input >> name >> equals >> value) || equals != "=")
			return false;
	}
	return true;
}

static size_t projectionFrameCount(const std::filesystem::path& dataRoot,
	const std::filesystem::path& poseRoot)
{
	size_t count = 0;
	while (std::filesystem::is_regular_file(dataRoot / (std::to_string(count) + ".jpg")) &&
		std::filesystem::is_regular_file(dataRoot / ("cam_" + std::to_string(count) + ".txt")) &&
		std::filesystem::is_regular_file(poseRoot / ("rt_" + std::to_string(count) + ".txt")))
		++count;
	return count;
}

static bool showTextureProjection(const TextureProjectionConfigDialog& dialog, QWidget* parent)
{
	const std::filesystem::path dataRoot(dialog.dataRoot().toStdString());
	const std::filesystem::path poseRoot(dialog.poseRoot().toStdString());
	const std::filesystem::path meshPath(dialog.meshPath().toStdString());
	if (!std::filesystem::is_directory(dataRoot) || !std::filesystem::is_directory(poseRoot) ||
		!std::filesystem::is_regular_file(meshPath))
	{
		QMessageBox::warning(parent, "Invalid paths", "Enter valid image, pose, and PLY paths.");
		return false;
	}
	size_t frameCount = projectionFrameCount(dataRoot, poseRoot);
	if (dialog.frameLimit() > 0)
		frameCount = std::min(frameCount, static_cast<size_t>(dialog.frameLimit()));
	if (frameCount == 0)
	{
		QMessageBox::warning(parent, "No frames", "No complete image/camera/pose sequence was found.");
		return false;
	}
	COMMON_LYJ::BaseTriMesh mesh;
	COMMON_LYJ::readPLYMesh(meshPath.string(), mesh);
	if (mesh.getVn() == 0 || mesh.getFn() == 0)
	{
		QMessageBox::critical(parent, "Mesh load failed", "The PLY mesh has no valid vertices or faces.");
		return false;
	}
	for (Eigen::Vector3f& point : mesh.getVertexs())
		point /= 1000.0f;
	std::vector<COMMON_LYJ::Pose3D> poses(frameCount);
	std::vector<ProjectorCamera> cameras;
	std::vector<COMMON_LYJ::CompressedImage> images(frameCount);
	cameras.reserve(frameCount);
	for (size_t index = 0; index < frameCount; ++index)
	{
		const std::string frame = std::to_string(index);
		cv::Mat image = cv::imread((dataRoot / (frame + ".jpg")).string(), cv::IMREAD_COLOR);
		if (image.empty())
		{
			QMessageBox::warning(parent, "Image load failed", QString("Could not load frame %1.").arg(index));
			return false;
		}
		cv::pyrDown(image, image);
		if (!images[index].compressCVMat(image))
			return false;
		std::vector<double> cameraParameters;
		if (!readProjectionCamera(dataRoot / ("cam_" + frame + ".txt"), cameraParameters))
			return false;
		for (double& parameter : cameraParameters)
			parameter /= 2.0;
		cameras.emplace_back(ProjectorCameraModel::Pinhole, image.cols, image.rows, cameraParameters);
		if (!COMMON_LYJ::readT34((poseRoot / ("rt_" + frame + ".txt")).string(), poses[index]))
			return false;
		poses[index].gett() /= 1000.0;
	}
	std::vector<COMMON_LYJ::BitFlagVec> visibility;
	ProjectionOptions options;
	options.backend = ProjectorBackend::OpenGL;
	std::string error;
	if (!projectMeshVisibility(mesh, poses, cameras, visibility, options, &error))
	{
		QMessageBox::critical(parent, "Projection failed", QString::fromStdString(error));
		return false;
	}
	OpenGLWindowTs* window = new OpenGLWindowTs(1600, 1200, "Texture Projection");
	window->changeMesh(mesh.getVertexs()[0].data(), mesh.getVn(), mesh.getFaces()[0].vId_, mesh.getFn(),
		poses, cameras, images, visibility);
	window->show();
	return true;
}

static int testOpenGLUnified(int argc, char* argv[])
{
	QApplication app(argc, argv);
	QWidget window;
	window.setWindowTitle("QT_LYJ Asset Viewer");
	window.resize(520, 260);
	QVBoxLayout* layout = new QVBoxLayout(&window);
	QLabel* title = new QLabel("Choose a viewer");
	title->setAlignment(Qt::AlignCenter);
	layout->addWidget(title);
	QPushButton* plyButton = new QPushButton("Show PLY");
	QPushButton* objButton = new QPushButton("Show OBJ");
	QPushButton* projectionButton = new QPushButton("Texture Projection");
	layout->addWidget(plyButton);
	layout->addWidget(objButton);
	layout->addWidget(projectionButton);
	QObject::connect(plyButton, &QPushButton::clicked, &window, [&window]()
		{
			const QString path = QFileDialog::getOpenFileName(&window, "Select PLY", QString(), "PLY files (*.ply)");
			if (!path.isEmpty())
				showPlyFile(path, &window);
		});
	QObject::connect(objButton, &QPushButton::clicked, &window, [&window]()
		{
			const QString path = QFileDialog::getOpenFileName(&window, "Select OBJ", QString(), "OBJ files (*.obj)");
			if (!path.isEmpty())
				showObjFile(path, &window);
		});
	QObject::connect(projectionButton, &QPushButton::clicked, &window, [&window]()
		{
			TextureProjectionConfigDialog dialog(&window);
			if (dialog.exec() == QDialog::Accepted)
				showTextureProjection(dialog, &window);
		});
	window.show();
	return app.exec();
}

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
	OpenGLProjector openGLProjector;
	if (options.backend == ProjectorBackend::OpenGL)
	{
		std::string openGLError;
		if (!openGLProjector.initialize(preparedMesh.getVertexs()[0].data(), pointCount,
			preparedMesh.getFNormals()[0].data(), preparedMesh.getFaces()[0].vId_, faceCount,
			&openGLError))
			return failProjection(openGLError, errorMessage);
	}

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

		if (options.backend == ProjectorBackend::OpenGL)
		{
			OpenGLProjectorCamera glCamera;
			glCamera.width = camera.width;
			glCamera.height = camera.height;
			glCamera.cameraModel = camera.model == ProjectorCameraModel::Fisheye ? 1 : 0;
			glCamera.parameters = camera.parameters;
			const float maxDepth = finiteVulkanMaxDepth(preparedMesh, Tcw, options.maxDepth, options.minDepth);
			if (!openGLProjector.project(Tcw.data(), glCamera, options.minDepth, maxDepth,
				options.normalCosineThreshold, options.visibilityDepthThreshold, depths, faceIds,
				visiblePoints, visibleFaces, errorMessage))
				return false;
		}
		else if (options.backend == ProjectorBackend::CUDA)
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
	QApplication app(argc, argv);
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


	QWidget window;
	window.setWindowTitle("QT_LYJ");
	window.setWindowFlags(window.windowFlags() | Qt::WindowMinimizeButtonHint | Qt::WindowMaximizeButtonHint);
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
	const ProjectorBackend backend = ProjectorBackend::OpenGL;
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

QT_LYJ_API int testOBJ(const std::string& path)
{
	COMMON_LYJ::BaseTriMesh obj;
	bool loaded = false;
	try { loaded = COMMON_LYJ::readOBJMesh(path, obj); }
	catch (const std::exception& error)
	{
		std::cerr << "Failed to read textured OBJ: " << error.what() << std::endl;
		return 1;
	}
	if (!loaded || obj.getVn() == 0 || obj.getFn() == 0 ||
		obj.getTextureCoords().empty() || obj.getTriUVs().size() != obj.getFaces().size())
	{
		std::cerr << "Failed to read textured OBJ: " << path << std::endl;
		return 1;
	}

	cv::Mat cvImage;
	QImage image;
	try
	{
		if (obj.getTexture().decompressCVMat(cvImage))
			cvMat3CToQImageRGB32(cvImage, image);
	}
	catch (const std::exception& error)
	{
		std::cerr << "Texture image is invalid for OBJ: " << error.what() << std::endl;
		return 1;
	}
	if (image.isNull())
	{
		std::cerr << "Texture image is missing or invalid for OBJ: " << path << std::endl;
		return 1;
	}

	const auto& points = obj.getVertexs();
	const auto& faces = obj.getFaces();
	const auto& uvs = obj.getTextureCoords();
	const auto& triUVs = obj.getTriUVs();
	std::vector<Eigen::Vector3f> expandedPoints;
	std::vector<Eigen::Vector2f> expandedUVs;
	expandedPoints.reserve(faces.size() * 3);
	expandedUVs.reserve(faces.size() * 3);
	std::vector<uint32_t> indices;
	indices.reserve(faces.size() * 3);
	for (size_t faceId = 0; faceId < faces.size(); ++faceId)
	{
		for (int corner = 0; corner < 3; ++corner)
		{
			const uint32_t pointId = faces[faceId].vId_[corner];
			const uint32_t uvId = triUVs[faceId].uvId_[corner];
			if (pointId >= points.size() || uvId >= uvs.size())
			{
				std::cerr << "Invalid OBJ vertex/texture index in face " << faceId << std::endl;
				return 1;
			}
			expandedPoints.push_back(points[pointId] / 1000.0f);
			expandedUVs.push_back(uvs[uvId]);
			indices.push_back(static_cast<uint32_t>(expandedPoints.size() - 1));
		}
	}

	int appArgc = 1;
	char appName[] = "QT_LYJ_OBJ";
	char* appArgv[] = { appName, nullptr };
	QApplication app(appArgc, appArgv);
	OpenGLWindow* window = new OpenGLWindow(false, 1600, 1200, "OBJ texture viewer");
	window->changeObj(expandedPoints[0].data(), expandedUVs[0].data(), image,
		expandedPoints.size(), indices.data(), indices.size() / 3);
	window->show();
	return app.exec();
}
NSP_QT_LYJ_END
