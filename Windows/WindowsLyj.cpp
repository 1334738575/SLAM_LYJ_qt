#include "WindowsLyj.h"
#include <QLabel>
#include "OpenGLs/OpenGLWidget.h"

WindowsLyj::WindowsLyj(QWidget *parent) 
    : QMainWindow(parent)
{
	//main window
	resize(w_, h_);
	setWindowTitle("SLAM_LYJ_VIEWER");
	QWidget* centralWidget = new QWidget(this);
	centralWidget->resize(w_, h_);
	layout_ = new QVBoxLayout();

	//opengl show
	openGLWidget_ = new OpenGLWidgetLyj(this);
	layout_->addWidget(openGLWidget_);

	//bottons
	addBotton("load module", [&]() {
		QString path = QFileDialog::getOpenFileName(this, tr("Load Module"), ".");
		if (!path.isEmpty()) {
			printLog("load module: " + path.toStdString());
			loadModel("");
		}
		});
	addBotton("open image", [&]() {
		QString path = QFileDialog::getOpenFileName(this, tr("Open Image"), ".", tr("Image Files(*.png *.jpg *.bmp)"));
		if (!path.isEmpty()) {
			changeImage(path.toStdString());
		}
		});

	//labels
	addLabel("Hello, Qt!", "D:/testLyj/build/Release/down.png");
	addLabel("log");

	//layout
	centralWidget->setLayout(layout_);
	setCentralWidget(centralWidget);

}

void WindowsLyj::addBotton(const std::string _name, std::function<void()> _func)
{
	QPushButton* button = new QPushButton(_name.c_str());
	layout_->addWidget(button);
	if (_func)
		QObject::connect(button, &QPushButton::clicked, _func);
	buttons_.push_back(button);
}

void WindowsLyj::addLabel(const std::string _name, const std::string _path)
{
	QLabel* label = new QLabel(_name.c_str());
	if (_path != "") {
		QPixmap pixmap(_path.c_str());
		label->setPixmap(pixmap);
		label->resize(pixmap.size());
	}
	layout_->addWidget(label);
	labels_.push_back(label);
}

void WindowsLyj::changeImage(const std::string _path)
{
	if (_path != "" || labels_[0]) {
		QPixmap pixmap(_path.c_str());
		labels_[0]->setPixmap(pixmap);
		labels_[0]->resize(pixmap.size());
	}
}

void WindowsLyj::changeImage(QPixmap& _pixmap)
{
	if (labels_[0]) {
		labels_[0]->setPixmap(_pixmap);
		labels_[0]->resize(_pixmap.size());
	}
}

void WindowsLyj::printLog(const std::string _log)
{
	if (labels_[1]) {
		labels_[1]->setText(_log.c_str());
	}
}

void WindowsLyj::loadModel(const std::string _path)
{
	// 初始化一些三维点
	std::vector<QVector3D> points;
	points.push_back(QVector3D(0.0f, 0.0f, 0.0f));
	points.push_back(QVector3D(0.5f, 0.0f, 0.1f));
	points.push_back(QVector3D(0.0f, 1.0f, 0.0f));
	points.push_back(QVector3D(0.0f, 0.0f, 1.0f));
	points.push_back(QVector3D(1.0f, 1.0f, 1.0f));
	openGLWidget_->setPoints(points);
	openGLWidget_->addLine(0, 1);
	openGLWidget_->addTriangle(0, 1, 2);
	openGLWidget_->addQuad(0, 1, 2, 3);
	std::vector<int> polygon = { 0, 1, 2, 3, 4 };
	openGLWidget_->addPolygon(polygon);
}

WindowsLyj::~WindowsLyj()
{
}

