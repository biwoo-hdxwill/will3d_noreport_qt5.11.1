#ifndef JOBMGR_GLOBAL_H
#define JOBMGR_GLOBAL_H

#include <QtCore/qglobal.h>

#ifdef JOBMGR_LIB
# define JOBMGR_EXPORT Q_DECL_EXPORT
#else
# define JOBMGR_EXPORT Q_DECL_IMPORT
#endif

#endif // JOBMGR_GLOBAL_H
