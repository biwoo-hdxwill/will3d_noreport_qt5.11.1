#pragma once

/**=================================================================================================

Project:		UIComponent
File:			view_tmj_orientation.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-11-14
Last modify: 	2018-11-14

	Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/

#include "base_view_orientation.h"

class UICOMPONENT_EXPORT ViewTMJorientation : public BaseViewOrientation {
public:
	ViewTMJorientation(const ReorientViewID& type,
						QWidget* parent = 0);
	~ViewTMJorientation();

	ViewTMJorientation(const ViewTMJorientation&) = delete;
	ViewTMJorientation& operator=(const ViewTMJorientation&) = delete;
private:

};
