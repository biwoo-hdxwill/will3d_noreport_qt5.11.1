#ifndef TEXTUREMAPPER_V2_GLOBAL_H
#define TEXTUREMAPPER_V2_GLOBAL_H

#include <QtCore/qglobal.h>

#ifdef TEXTUREMAPPER_V2_LIB
# define TEXTUREMAPPER_V2_EXPORT Q_DECL_EXPORT
#else
# pragma comment(lib, "TextureMapper_v2.lib")
# define TEXTUREMAPPER_V2_EXPORT Q_DECL_IMPORT
#endif

#endif // TEXTUREMAPPER_V2_GLOBAL_H
