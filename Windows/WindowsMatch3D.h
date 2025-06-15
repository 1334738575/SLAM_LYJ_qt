#ifndef WINDOWS_MATCH3D_H
#define WINDOWS_MATCH3D_H

#include "WindowsLyj.h"

namespace QT_LYJ
{
	class WindowsMatch3D : public WindowsLyj
	{
	public:
		WindowsMatch3D(QWidget* parent = nullptr);
		~WindowsMatch3D();

	protected:
		void loadModel(const QString _path);

	protected:
	};

}

#endif
