#ifndef UTIL_GLOBAL_H
#define UTIL_GLOBAL_H

#include <QtCore/qglobal.h>

#ifdef UTIL_LIB
# define UTIL_EXPORT Q_DECL_EXPORT
#else
# pragma comment(lib, "Util.lib")
# define UTIL_EXPORT Q_DECL_IMPORT
#endif

#endif // UTIL_GLOBAL_H
