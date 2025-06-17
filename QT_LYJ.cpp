#include <QApplication>
#include <QPushButton>
#include <QVBoxLayout>
#include <OpenGLs/OpenGLWidget.h>

#include "QT_LYJ.h"
//#include "SLAM_LYJ.h"
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
	OpenGLWindow(QWidget *parent = nullptr) : QDialog(parent)
	{
		setWindowTitle("OpenGL Window");
		setFixedSize(800, 600);

		//OpenGLWidget* openGLWidget = new OpenGLWidget(this);
		MyOpenGLWidget* openGLWidget = new MyOpenGLWidget(this);
		QVBoxLayout *layout = new QVBoxLayout(this);
		layout->addWidget(openGLWidget);
	}
};

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

		QPushButton *button = new QPushButton("open!");
		layout->addWidget(button);

		QObject::connect(button, &QPushButton::clicked, [&]()
						 {
							 OpenGLWindow *w = new OpenGLWindow();
							 // w->exec();
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

NSP_QT_LYJ_END