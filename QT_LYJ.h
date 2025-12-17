#ifndef QT_LYJ_H
#define QT_LYJ_H

#include <iostream>
#include "QT_LYJ_Defines.h"
#include "DataIO/DataWin2D.h"


NSP_QT_LYJ_BEGIN

QT_LYJ_API int testQT(int argc, char* argv[]);
QT_LYJ_API int testOpenGLOnly();


QT_LYJ_API void debugWindows(int argc, char* argv[]);
QT_LYJ_API void recordBin2D(const std::string& _dataHome2D, const Data2DPoint& _data2DPoint, const Data2DLine& _data2DLine, const Data2DEdge& _data2DEdge);

NSP_QT_LYJ_END

#endif // QT_LYJ_H
