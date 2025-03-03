#include "WindowsLyj.h"
#include <QLabel>

WindowsLyj::WindowsLyj(QWidget *parent) 
    : QMainWindow(parent), imageLabel(new QLabel("open image", this)), openButton(new QPushButton("Open", this))
{
	QVBoxLayout* layout = new QVBoxLayout(this);
	layout->addWidget(imageLabel);
	layout->addWidget(openButton);

	QWidget* centralWidget = new QWidget(this);
	centralWidget->setLayout(layout);
	setCentralWidget(centralWidget);
	connect(openButton, &QPushButton::clicked, this, &WindowsLyj::openImage);

    setWindowTitle("My Qt Application");

    //setFixedSize(400, 300);
    //QLabel *label = new QLabel("Hello, Qt!", this);
    //label->setAlignment(Qt::AlignCenter);
    //setCentralWidget(label);
}

WindowsLyj::~WindowsLyj()
{
}

void WindowsLyj::openImage()
{
	QString imagePath = QFileDialog::getOpenFileName(this, "Open Image", "", "Image Files (*.png *.jpg *.bmp)");
	if (imagePath.isEmpty())
	{
		return;
	}
	pixmap.load(imagePath);
	imageLabel->setPixmap(pixmap);
	imageLabel->setScaledContents(true);
}
