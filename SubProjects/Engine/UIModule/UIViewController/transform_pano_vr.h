#pragma once

/**=================================================================================================

Project: 			UIViewController
File:				transform_pano_vr.h
Language:			C++11
Library:			Qt 5.8.0
author:				Tae Hoon Yoo
First date:			2017-08-31
Last modify:		2017-08-31

Copyright (c) 2017 All rights reserved by HDXWILL.

*===============================================================================================**/

#include "base_transform.h"

class TransformPanoVR : public BaseTransform
{
public:
	TransformPanoVR();

	TransformPanoVR(const TransformPanoVR&) = delete;
	TransformPanoVR& operator=(const TransformPanoVR&) = delete;

public:
	virtual void Initialize(const glm::vec3& world_scale) override {
		BaseTransform::Initialize(world_scale);
	}

private:
	virtual void SetViewMatrix() override;
};
