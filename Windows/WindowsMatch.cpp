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
			QString directoryPath = QFileDialog::getExistingDirectory(this, "Open directory", "../QT/data");
			if (!directoryPath.isEmpty()) {
				if (m_windowsMatch2D)
					m_windowsMatch2D->loadModel(directoryPath.toStdString());
				else
					printLog("windows match 2D is nullptr!");
			}
			});
	}
	WindowsMatch::~WindowsMatch()
	{}

}