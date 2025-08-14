#pragma once

/**=================================================================================================

Project: 			UIViewController
File:				volume_transform.h
Language:			C++11
Library:			Qt 5.8.0
author:				Tae Hoon Yoo
First date:			2017-08-31
Last modify:		2017-08-31

Copyright (c) 2017 All rights reserved by HDXWILL.

 *===============================================================================================**/

#include "base_transform.h"

class TransformBasicVR : public BaseTransform
{
public:
	TransformBasicVR() = default;

	TransformBasicVR(const TransformBasicVR&) = delete;
	TransformBasicVR& operator=(const TransformBasicVR&) = delete;

public:
	virtual void Initialize(const glm::vec3& model_scale) override {
		BaseTransform::Initialize(model_scale);
	}

private:
	virtual void SetViewMatrix() override;
};
