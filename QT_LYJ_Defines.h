#ifndef QT_LYJ_DEFINES_H
#define QT_LYJ_DEFINES_H

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

#define NSP_QT_LYJ_BEGIN namespace QT_LYJ {
#define NSP_QT_LYJ_END }


NSP_QT_LYJ_BEGIN

QT_LYJ_API int64_t imagePair2Int64(int _i1, int _i2);
QT_LYJ_API std::pair<int, int> int642TwoImagePair(int64_t _pair);


NSP_QT_LYJ_END

#endif // !QT_LYJ_DEFINES_H
