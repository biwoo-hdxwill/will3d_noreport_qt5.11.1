#pragma once

#include <QtCore/qglobal.h>

#ifndef BUILD_STATIC
# if defined(UITOOLS_LIB)
#  define UITOOLS_EXPORT Q_DECL_EXPORT
# else
#  define UITOOLS_EXPORT Q_DECL_IMPORT
# endif
#else
# define UITOOLS_EXPORT
#endif

#include <qmargins.h>

namespace ui_tools {
const QMargins kMarginZero(0, 0, 0, 0);
} // end of namespace ui_tools
