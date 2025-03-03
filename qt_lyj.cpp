#include "qt_lyj.h"
#include <QApplication>
#include <QPushButton>
#include "Windows/WindowSLyj.h"
#include "OpenGLs/OpenGLWidget.h"

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
    QPushButton button("Hello, Qt!");
	WindowsLyj window;
	window.show();
    return app.exec();
}

static int testOpenGL()
{
    int argc = 0;
    char** argv = nullptr;
    QApplication app(argc, argv);
    QPushButton button("Hello, Qt!");
    OpenGLWidget w;
	w.resize(800, 600);
	w.show();
    return app.exec();
}

int testQT()
{
	//testButton();
	//testWindow();
	testOpenGL();

    return 1;
}