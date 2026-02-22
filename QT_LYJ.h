#ifndef QT_LYJ_H
#define QT_LYJ_H

#include <iostream>
#include "QT_LYJ_Defines.h"
#include <IO/DataWin2D.h>
#include <common/BaseTriMesh.h>
#include <base/Pose.h>
#include <base/CameraModule.h>
#include <common/CompressedImage.h>


NSP_QT_LYJ_BEGIN

QT_LYJ_API int testQT(int argc, char* argv[]);
QT_LYJ_API int testOpenGLOnly();


QT_LYJ_API void debugWindows(int argc, char* argv[]); 
QT_LYJ_API int testTcws(int argc, char* argv[],
	const COMMON_LYJ::BaseTriMesh& _btm,
	const std::vector<COMMON_LYJ::Pose3D>& _Tcws, const std::vector<COMMON_LYJ::PinholeCamera>& _cams, const std::vector<COMMON_LYJ::CompressedImage>& _comImgs);

NSP_QT_LYJ_END

#endif // QT_LYJ_H
