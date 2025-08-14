#ifndef RENDERINGCODE_V2_GLOBAL_H
#define RENDERINGCODE_V2_GLOBAL_H

#include <QtCore/qglobal.h>

#ifdef RENDERINGCODE_V2_LIB
# define RENDERINGCODE_V2_EXPORT Q_DECL_EXPORT
#else
#pragma comment(lib, "RenderingCode_v2.lib")
# define RENDERINGCODE_V2_EXPORT Q_DECL_IMPORT
#endif

#endif // RENDERINGCODE_V2_GLOBAL_H
