#ifndef WINDOWS_MATCH3D_H
#define WINDOWS_MATCH3D_H

#include "WindowsLyj.h"
#include <Eigen/Core>
#include <Eigen/Eigen>

namespace QT_LYJ
{
	class WindowsMatch3D : public WindowsLyj
	{
	public:
		WindowsMatch3D(QWidget* parent = nullptr);
		~WindowsMatch3D();

		void loadModel(const QString _path);

	protected:

	protected:
		int m_iter = 0;
		int m_frameSize = 0;
		int m_enableNormal = 0;
		int m_enableColor = 0;
		std::vector<int> m_allPsSize;
		std::vector<std::vector<Eigen::Vector3f>> m_allPoints;
		std::vector<std::vector<Eigen::Vector3f>> m_allNormals;
		std::vector<std::vector<Eigen::Vector3f>> m_allColors;
		std::vector<std::map<int64_t, std::vector<Eigen::Vector2i>>> m_allCorrs;
		std::vector<std::vector<Eigen::Matrix3f>> m_allFrameRwcs;
		std::vector<std::vector<Eigen::Vector3f>> m_allFrametwcs;
	};

}

#endif
