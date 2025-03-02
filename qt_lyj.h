#ifndef QT_PCL_H
#define QT_PCL_H

#include <iostream>

// export
#ifdef WIN32
#ifdef _MSC_VER
#define QT_LYJ_API __declspec(dllexport)
#else
#define QT_LYJ_API
#endif
#else
#define QT_LYJ_API
#endif

QT_LYJ_API int testQT();

#endif // QT_PCL_H
