#ifndef MESHMOVE3D_V2_GLOBAL_H
#define MESHMOVE3D_V2_GLOBAL_H

#include <QtCore/qglobal.h>

#ifdef MESHMOVE3D_V2_LIB
#pragma message("#export MeshMove_v2")
# define MESHMOVE3D_V2_EXPORT Q_DECL_EXPORT
#else
#pragma comment(lib, "MeshMove3d_v2.lib")
#pragma message("#import MeshMove_v2")
# define MESHMOVE3D_V2_EXPORT Q_DECL_IMPORT
#endif

#endif // MESHMOVE3D_V2_GLOBAL_H
