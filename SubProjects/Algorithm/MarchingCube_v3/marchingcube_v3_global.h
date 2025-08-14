#ifndef MARCHINGCUBE_V3_GLOBAL_H
#define MARCHINGCUBE_V3_GLOBAL_H

#include <QtCore/qglobal.h>

#ifdef MARCHINGCUBE_V3_LIB
# define MARCHINGCUBE_V3_EXPORT Q_DECL_EXPORT
#else
#pragma comment(lib, "MarchingCube_v3.lib")
# define MARCHINGCUBE_V3_EXPORT Q_DECL_IMPORT
#endif

#endif // MARCHINGCUBE_V3_GLOBAL_H
