#pragma once

/**=================================================================================================

Project:		UIViewController
File:			transform_implant_3d.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2019-01-03
Last modify: 	2019-01-03

	Copyright (c) 2019 HDXWILL. All rights reserved.

 *===============================================================================================**/

#include "transform_roi_vr.h"

class TransformImplant3D : public TransformROIVR {
public:
	TransformImplant3D() = default;

	TransformImplant3D(const TransformImplant3D&) = delete;
	TransformImplant3D& operator=(const TransformImplant3D&) = delete;

public:
  void SyncBoneDensityCameraMatrix(const glm::mat4& rotate_mat,
								   const glm::mat4& reorien_mat,
								   const glm::mat4& view_mat);

private:
};
