#ifndef TMJ_GLOBAL_H
#define TMJ_GLOBAL_H

#include <QtCore/qglobal.h>

#ifdef TMJ_LIB
# define TMJ_EXPORT Q_DECL_EXPORT
#else
# define TMJ_EXPORT Q_DECL_IMPORT
#endif

#endif // TMJ_GLOBAL_H
