#ifndef WINDOWS_LYJ_H
#define WINDOWS_LYJ_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QPixmap>

class OpenGLWidgetLyj;
class WindowsLyj : public QMainWindow
{
    Q_OBJECT

public:
    WindowsLyj(QWidget *parent = nullptr);
	~WindowsLyj();

	void addBotton(const std::string _name, std::function<void()> _func = nullptr);
	void addLabel(const std::string _name, const std::string _path = "");
	void changeImage(const std::string _path);
	void changeImage(QPixmap& _pixmap);
	void printLog(const std::string _log);
	void loadModel(const std::string _path);

private:
	std::vector<QLabel*> labels_; //0 for image, 1 for log
	std::vector<QPushButton*> buttons_;
	QVBoxLayout* layout_ = nullptr;
	OpenGLWidgetLyj* openGLWidget_ = nullptr;
	int w_ = 600;
	int h_ = 1200;
};

#endif // WINDOWS_LYJ_H