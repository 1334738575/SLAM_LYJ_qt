#include "WindowsLyj.h"

namespace QT_LYJ {

	WindowsLyj::WindowsLyj(QWidget* parent)
		: QMainWindow(parent)
	{
		//main window
		resize(w_, h_);
		setWindowTitle("SLAM_LYJ_VIEWER");
		//QWidget* centralWidget = new QWidget(this);
		centralWidget = new QWidget(this);
		centralWidget->resize(w_, h_);
		layout_ = new QGridLayout();

		//add quit
		QPushButton* qButton = new QPushButton("quit", centralWidget);
		layout_->addWidget(qButton, 5, 2, 1, 1);
		connect(qButton, SIGNAL(clicked()), qApp, SLOT(quit()));
		buttons_.push_back(qButton);

		//opengl show
		openGLWidget_ = new OpenGLWidgetLyj(this);
		layout_->addWidget(openGLWidget_, 0, 0, 4, 5);

		//labels
		//addLabel("Hello, Qt!", "D:/testLyj/build/Release/down.png");
		addLabel("log", 4, 0, 1, 6);

		//layout
		centralWidget->setLayout(layout_);
		setCentralWidget(centralWidget);

	}

	void WindowsLyj::addBotton(const std::string _name, int _si, int _sj, int _r, int _c, std::function<void()> _func)
	{
		QPushButton* button = new QPushButton(_name.c_str(), centralWidget);
		layout_->addWidget(button, _si, _sj, _r, _c);
		if (_func)
			QObject::connect(button, &QPushButton::clicked, _func);
		buttons_.push_back(button);
	}
	void WindowsLyj::addLabel(const std::string _name, int _si, int _sj, int _r, int _c)
	{
		//	QLabel* label = new QLabel(_name.c_str());
		//	if (_path != "") {
		//		QPixmap pixmap(_path.c_str());
		//		label->setPixmap(pixmap);
		//		label->resize(pixmap.size());
		//	}
		//	layout_->addWidget(label, _si, _sj, _r, _c);
		//	labels_.push_back(label);
		QLabel* label = new QLabel(_name.c_str());
		label->setText(_name.c_str());
		layout_->addWidget(label, _si, _sj, _r, _c);
		labels_.push_back(label);
	}

	void WindowsLyj::changeImage(const std::string _path)
	{
		//if (_path != "" || labels_[0]) {
		//	QPixmap pixmap(_path.c_str());
		//	labels_[0]->setPixmap(pixmap);
		//	labels_[0]->resize(pixmap.size());
		//}
		QString path = QString::fromStdString(_path);
		QImage img(path);
		if (img.isNull()) {
			printLog("Failed to load image: " + _path);
			return;
		}
		// 转换为 RGBA8888 格式
		if (img.format() != QImage::Format_RGBA8888) {
			img = img.convertToFormat(QImage::Format_RGBA8888);
		}
		openGLWidget_->setTexture(img);
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
		if (labels_[0]) {
			labels_[0]->setText(_log.c_str());
			labels_[0]->update();
		}
	}

	WindowsLyj::~WindowsLyj()
	{
	}

}
