#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(WILL3DENGINE_LIB)
#  define WILL3DENGINE_EXPORT Q_DECL_EXPORT
# else
#  define WILL3DENGINE_EXPORT Q_DECL_IMPORT
# endif
#else
# define WILL3DENGINE_EXPORT
#endif
