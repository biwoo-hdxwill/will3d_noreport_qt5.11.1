#ifndef TABMGR_GLOBAL_H
#define TABMGR_GLOBAL_H

#include <QtCore/qglobal.h>

#ifdef TABMGR_LIB
# define TABMGR_EXPORT Q_DECL_EXPORT
#else
# define TABMGR_EXPORT Q_DECL_IMPORT
#endif

#endif // TABMGR_GLOBAL_H
