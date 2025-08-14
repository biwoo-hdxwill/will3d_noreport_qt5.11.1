#ifndef SURFACING_GLOBAL_H
#define SURFACING_GLOBAL_H

#include <QtCore/qglobal.h>

#ifdef SURFACING_LIB
# define SURFACING_EXPORT Q_DECL_EXPORT
#else
# define SURFACING_EXPORT Q_DECL_IMPORT
#endif

#endif // SURFACING_GLOBAL_H
