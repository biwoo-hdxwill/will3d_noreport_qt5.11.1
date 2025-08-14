#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(W3PROJECTIO_LIB)
#  define W3PROJECTIO_EXPORT Q_DECL_EXPORT
# else
#  define W3PROJECTIO_EXPORT Q_DECL_IMPORT
# endif
#else
# define W3PROJECTIO_EXPORT
#endif
