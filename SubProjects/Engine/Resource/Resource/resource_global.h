#ifndef RESOURCE_GLOBAL_H
#define RESOURCE_GLOBAL_H

#include <QtCore/qglobal.h>

#ifdef RESOURCE_LIB
# define RESOURCE_EXPORT Q_DECL_EXPORT
#else
# define RESOURCE_EXPORT Q_DECL_IMPORT
#endif

#endif // RESOURCE_GLOBAL_H
