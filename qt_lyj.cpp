#include "qt_lyj.h"
#include <QApplication>
#include <QPushButton>
#include "Windows/WindowSLyj.h"
#include "OpenGLs/OpenGLWidget.h"
#include <QVBoxLayout>

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

static int testWindow()
{
    int argc = 0;
    char** argv = nullptr;
    QApplication app(argc, argv);
	WindowsLyj window;
	window.show();
    return app.exec();
}

class OpenGLWindow : public QDialog
{
public:
	OpenGLWindow(QWidget* parent = nullptr) : QDialog(parent)
	{
		setWindowTitle("OpenGL Window");
		setFixedSize(800, 600);

		OpenGLWidget* openGLWidget = new OpenGLWidget(this);
		QVBoxLayout* layout = new QVBoxLayout(this);
		layout->addWidget(openGLWidget);
	}
};

static int testOpenGL()
{
	bool justOpenGL = false;
	bool openGLAndBottons = false;
	bool openGLByBotton = true;

    int argc = 0;
    char** argv = nullptr;
    QApplication app(argc, argv);

	if (justOpenGL) {
		OpenGLWidget* w = new OpenGLWidget();
		w->resize(800, 600);
		w->show();
		return app.exec();
	}

	if (openGLAndBottons) {
		QWidget window;
		window.setWindowTitle("QT_LYJ");
		//qgridlayout qformlayout
		QVBoxLayout* layout = new QVBoxLayout(&window);

		OpenGLWidget* w = new OpenGLWidget();
		layout->addWidget(w);

		QPushButton* button = new QPushButton("move");
		layout->addWidget(button);

		//QObject::connect(button, &QPushButton::clicked, w, &OpenGLWidget::print);
		QObject::connect(button, &QPushButton::clicked, [&]() {
			w->addMove();
			});
		window.setLayout(layout);
		window.resize(800, 600);
		window.show();
		return app.exec();
	}

	if (openGLByBotton) {
		QWidget window;
		window.setWindowTitle("QT_LYJ");
		QVBoxLayout* layout = new QVBoxLayout(&window);

		QPushButton* button = new QPushButton("open!");
		layout->addWidget(button);

		QObject::connect(button, &QPushButton::clicked, [&]() {
			OpenGLWindow* w = new OpenGLWindow();
			w->exec();
			});
		window.setLayout(layout);
		window.resize(800, 600);
		window.show();
		return app.exec();
	}

    return app.exec();
}

int testQT()
{
	//testButton();
	//testWindow();
	testOpenGL();

    return 1;
}