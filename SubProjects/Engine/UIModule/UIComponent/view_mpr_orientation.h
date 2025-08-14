#pragma once
/*=========================================================================

File:			view_mpr_orientation.h
Language:		C++11
Library:		Qt 5.8.0, Standard C++ Library
Author:			JUNG DAE GUN
First Date:		2020-02-20
Last Modify:	2020-02-20

Copyright (c) 2018~2020 All rights reserved by HDXWILL.

=========================================================================*/

#include "base_view_orientation.h"

class UICOMPONENT_EXPORT ViewMPROrientation : public BaseViewOrientation {
public:
	ViewMPROrientation(const ReorientViewID& type, QWidget* parent = 0);
	virtual ~ViewMPROrientation();
};
