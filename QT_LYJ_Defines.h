#ifndef QT_LYJ_DEFINES_H
#define QT_LYJ_DEFINES_H


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




NSP_QT_LYJ_END

#endif // !QT_LYJ_DEFINES_H
