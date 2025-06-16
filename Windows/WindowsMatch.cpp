#include "WindowsMatch.h"

namespace QT_LYJ
{
	WindowsMatch::WindowsMatch(QWidget* parent)
		:WindowsMatch3D(parent)
	{
		auto funcPrint = [&](const std::string& _info) {
			printLog(_info);
			};
		m_windowsMatch2D = std::make_shared<WindowsMatch2D>(funcPrint);
		//bottons
		addBotton("load image", [&]() {
			openGLWidget_->makeCurrent();
			// 释放关键资源（可选）
			openGLWidget_->doneCurrent();
			// 延迟触发对话框
			QTimer::singleShot(0, this, [&]() {
				QString directoryPath = QFileDialog::getExistingDirectory(nullptr, "Open directory", "../QT/data");
				if (!directoryPath.isEmpty()) {
					if (m_windowsMatch2D)
						m_windowsMatch2D->loadModel(directoryPath.toStdString());
					else
						printLog("windows match 2D is nullptr!");
				}
				//m_windowsMatch2D->loadModel("../QT/data/2D");
			});
		});
	}
	WindowsMatch::~WindowsMatch()
	{}

}