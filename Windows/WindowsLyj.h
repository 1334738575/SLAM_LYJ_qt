#ifndef WINDOWS_LYJ_H
#define WINDOWS_LYJ_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QPixmap>

class WindowsLyj : public QMainWindow
{
    Q_OBJECT

public:
    WindowsLyj(QWidget *parent = nullptr);
	~WindowsLyj();

private slots:
	void openImage();

private:
	QLabel* imageLabel = nullptr;
	QPushButton* openButton = nullptr;
	QVBoxLayout* layout = nullptr;
	QFileDialog* fileDialog = nullptr;
	QPixmap pixmap;
};

#endif // WINDOWS_LYJ_H