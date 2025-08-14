#pragma once
/*=========================================================================

File:			class CW3Cursor
Language:		C++11
Library:		Qt 5.4.0, Standard C++ Library
Author:			TaeHoon Yoo
First Date:		2015-07-13
Modify Date:	2016-07-13
Version:		1.0

Copyright (c) 2015 All rights reserved by HDXWILL.

=========================================================================*/
#include <QCursor>

namespace common{
enum class CommonToolTypeOnOff;
}
#include "common_global.h"

class COMMON_EXPORT CW3Cursor {
public:
	CW3Cursor() {};

	static void SetViewCursor(const common::CommonToolTypeOnOff& tool_type);
	static QCursor ArrowCursor();
	static QCursor CrossCursor();
	static QCursor FreedrawCursor();
	static QCursor LightCursor();
	static QCursor WaitCursor(unsigned int step);
	static QCursor IBeamCursor();
	static QCursor SizeVerCursor();
	static QCursor SizeHorCursor();
	static QCursor SizeBDiagCursor();
	static QCursor SizeFDiagCursor();
	static QCursor SizeAllCursor();
	static QCursor SplitVCursor();
	static QCursor SplitHCursor();
	static QCursor PointingHandCursor();
	static QCursor ForbiddenCursor();
	static QCursor OpenHandCursor();
	static QCursor ClosedHandCursor();
	static QCursor RotateCursor();
	static QCursor ZoomCursor();
	static QCursor ZoomInCursor();
	static QCursor ZoomOutCursor();
	static QCursor PointCursor();
};
