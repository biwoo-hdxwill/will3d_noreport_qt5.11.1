#ifndef SOLVER_GLOBAL_H
#define SOLVER_GLOBAL_H

#include <QtCore/qglobal.h>

#ifdef SOLVER_LIB
# define SOLVER_EXPORT Q_DECL_EXPORT
#else
#pragma comment(lib, "Solver.lib")
#pragma message("# import Solver.lib")
# define SOLVER_EXPORT Q_DECL_IMPORT
#endif

#endif // SOLVER_GLOBAL_H
