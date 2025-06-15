#ifndef WINDOWS_MATCH_H
#define WINDOWS_MATCH_H

#include "WindowsMatch2D.h"
#include "WindowsMatch3D.h"

namespace QT_LYJ
{
	class WindowsMatch : public WindowsMatch3D
	{
	public:
		WindowsMatch(QWidget* parent = nullptr);
		~WindowsMatch();

	private:
		std::shared_ptr<WindowsMatch2D> m_windowsMatch2D = nullptr;
	};

}

#endif // WINDOWS_MATCH_H