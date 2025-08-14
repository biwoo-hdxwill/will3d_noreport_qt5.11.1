#ifndef ENDOSCOPY_GLOBAL_H
#define ENDOSCOPY_GLOBAL_H

#include <QtCore/qglobal.h>

#ifdef ENDOSCOPY_LIB
# define ENDOSCOPY_EXPORT Q_DECL_EXPORT
#else
# define ENDOSCOPY_EXPORT Q_DECL_IMPORT
#endif

#endif // ENDOSCOPY_GLOBAL_H
