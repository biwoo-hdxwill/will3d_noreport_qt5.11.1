#pragma once

/**=================================================================================================

Project:		UIComponent
File:			view_pano_orientation.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-11-14
Last modify: 	2018-11-14

	Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/

#include "base_view_orientation.h"

class UICOMPONENT_EXPORT ViewPanoOrientation : public BaseViewOrientation {
public:
	ViewPanoOrientation(const ReorientViewID& type,
						QWidget* parent = 0);
	~ViewPanoOrientation();

	ViewPanoOrientation(const ViewPanoOrientation&) = delete;
	ViewPanoOrientation& operator=(const ViewPanoOrientation&) = delete;
};
