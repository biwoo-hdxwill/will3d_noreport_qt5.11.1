#ifndef W3DICOMIO_GLOBAL_H
#define W3DICOMIO_GLOBAL_H

#include <QtCore/qglobal.h>

#ifdef W3DICOMIO_LIB
# define W3DICOMIO_EXPORT Q_DECL_EXPORT
#else
# define W3DICOMIO_EXPORT Q_DECL_IMPORT
#endif

#endif // W3DICOMIO_GLOBAL_H
