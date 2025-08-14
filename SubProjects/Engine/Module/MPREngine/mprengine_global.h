#ifndef MPRENGINE_GLOBAL_H
#define MPRENGINE_GLOBAL_H

#include <QtCore/qglobal.h>

#ifdef MPRENGINE_LIB
# define MPRENGINE_EXPORT Q_DECL_EXPORT
#else
# define MPRENGINE_EXPORT Q_DECL_IMPORT
#endif

#endif // MPRENGINE_GLOBAL_H
