#ifndef PANORAMA_GLOBAL_H
#define PANORAMA_GLOBAL_H

#include <QtCore/qglobal.h>

#ifdef PANORAMA_LIB
# define PANORAMA_EXPORT Q_DECL_EXPORT
#else
# define PANORAMA_EXPORT Q_DECL_IMPORT
#endif

#endif // PANORAMA_GLOBAL_H
