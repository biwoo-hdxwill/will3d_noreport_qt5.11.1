#ifndef FACE_GLOBAL_H
#define FACE_GLOBAL_H

#include <QtCore/qglobal.h>

#ifdef FACE_LIB
# define FACE_EXPORT Q_DECL_EXPORT
#else
# define FACE_EXPORT Q_DECL_IMPORT
#endif

#endif // FACE_GLOBAL_H
