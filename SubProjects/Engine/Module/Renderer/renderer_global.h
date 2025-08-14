#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(RENDERER_LIB)
#  define RENDERER_EXPORT Q_DECL_EXPORT
# else
#  define RENDERER_EXPORT Q_DECL_IMPORT
# endif
#else
# define RENDERER_EXPORT
#endif
