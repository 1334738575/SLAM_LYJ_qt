#ifndef WINDOWS_LYJ_H
#define WINDOWS_LYJ_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QPixmap>
#include <QDir>
#include <QFileInfoList>
#include <QApplication>
#include <QWidget>
#include <fstream>
#include "OpenGLs/OpenGLWidget.h"

namespace QT_LYJ {

	class OpenGLWidgetLyj;
	class WindowsLyj : public QMainWindow
	{
		Q_OBJECT

	public:
		WindowsLyj(QWidget* parent = nullptr);
		~WindowsLyj();

	protected:
		void addBotton(const std::string _name, std::function<void()> _func = nullptr);
		void addLabel(const std::string _name);
		void printLog(const std::string _log);

		void changeImage(const std::string _path);
		void changeImage(QPixmap& _pixmap);

		QString getFileName(const std::string& _filePath) const
		{
			return QString::fromStdString(_filePath.substr(_filePath.find_last_of("/\\") + 1));
		}
		QStringList getAbsFilePathInDirByCode(const std::string& _dir = ".") const
		{
			QStringList filePaths;
			QDir dir(QString::fromStdString(_dir));
			if (dir.exists()) {
				//QFileInfoList fileList = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::AllEntries);//获取所有文件和子目录
				QFileInfoList fileList = dir.entryInfoList(QDir::Files);
				for (const QFileInfo& fileInfo : fileList) {
					filePaths.append(fileInfo.absoluteFilePath());
				}
			}
			return filePaths;
		}
		QStringList getAbsFileNameInDirByCode(const std::string& _dir = ".") const
		{
			QStringList filePaths;
			QDir dir(QString::fromStdString(_dir));
			if (dir.exists()) {
				QFileInfoList fileList = dir.entryInfoList(QDir::Files);
				for (const QFileInfo& fileInfo : fileList) {
					filePaths.append(fileInfo.fileName());
				}
			}
			return filePaths;
		}
		QStringList getAbsFilePathByUI(const std::string& _dir = ".")
		{
			QString dir = QString::fromStdString(_dir);
			QStringList filePaths = QFileDialog::getOpenFileNames(this, tr("Open file"), dir);
			return filePaths;
		}
		QStringList getAbsFileNameByUI(const std::string& _dir = ".")
		{
			QString dir = QString::fromStdString(_dir);
			QStringList filePaths = QFileDialog::getOpenFileNames(this, tr("Open file"), dir);
			QStringList fileNames;
			for (const auto& file : filePaths)
			{
				std::string pathStr = file.toStdString();
				QString fileName = QString::fromStdString(pathStr.substr(pathStr.find_last_of("/\\") + 1));
				fileNames.push_back(fileName);
			}
			return fileNames;
		}
		QStringList getAbsFilePathInDirByUI(const std::string& _dir = ".")
		{
			QString dir = QString::fromStdString(_dir);
			// 打开文件夹选择对话框
			QString directoryPath = QFileDialog::getExistingDirectory(nullptr, "Open directory", dir);
			// 检查用户是否选择了目录
			QStringList fileNames;
			if (!directoryPath.isEmpty()) {
				// 使用 QDir 获取该目录下的所有文件
				QDir dir(directoryPath);
				QFileInfoList fileList = dir.entryInfoList(QDir::Files);
				for (const QFileInfo& fileInfo : fileList) {
					fileNames.push_back(fileInfo.absoluteFilePath());
				}
			}
			return fileNames;
		}
		QStringList getAbsFileNameInDirByUI(const std::string& _dir = ".")
		{
			QString dir = QString::fromStdString(_dir);
			// 打开文件夹选择对话框
			QString directoryPath = QFileDialog::getExistingDirectory(nullptr, "Open directory", dir);
			// 检查用户是否选择了目录
			QStringList fileNames;
			if (!directoryPath.isEmpty()) {
				// 使用 QDir 获取该目录下的所有文件
				QDir dir(directoryPath);
				QFileInfoList fileList = dir.entryInfoList(QDir::Files);
				for (const QFileInfo& fileInfo : fileList) {
					fileNames.push_back(fileInfo.fileName());
				}
			}
			return fileNames;
		}


	protected:
		OpenGLWidgetLyj* openGLWidget_ = nullptr;
		std::vector<QLabel*> labels_;
		std::vector<QPushButton*> buttons_;
		QVBoxLayout* layout_ = nullptr;
		int w_ = 1200;
		int h_ = 1000;
		QWidget* centralWidget = nullptr;
	};

}
#endif // WINDOWS_LYJ_H