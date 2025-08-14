#pragma once

/**=================================================================================================

Project: 			UIViewController
File:				transform_image.h
Language:			C++11
Library:			Qt 5.8.0
author:				Tae Hoon Yoo
First date:			2017-08-24
Last modify:		2017-08-24

Copyright (c) 2017 All rights reserved by HDXWILL.

 *===============================================================================================**/

#include "base_transform.h"

class TransformImage : public BaseTransform
{
public:
	TransformImage() = default;

	TransformImage(const TransformImage&) = delete;
	TransformImage& operator=(const TransformImage&) = delete;

public:
	void Initialize(int img_width, int img_height);

	virtual void Initialize(const glm::vec3& image_scale) override;

	inline int img_width() const { return img_width_; }
	inline int img_height() const { return img_height_; }
private:
	virtual void SetViewMatrix() override;

	int img_width_ = 0;
	int img_height_ = 0;
};
