#pragma once
/**=================================================================================================

Project:		UIViewController
File:			transform_pano_orientation.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-02-19
Last modify: 	2018-02-19

	Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/
#include "../../../Engine/Common/Common/W3Enum.h"
#include "base_transform.h"

class TransformPanoOrientation : public BaseTransform {
public:
	TransformPanoOrientation(const ReorientViewID& type);

	TransformPanoOrientation(const TransformPanoOrientation&) = delete;
	TransformPanoOrientation& operator=(const TransformPanoOrientation&) = delete;

public:
	virtual void Initialize(const glm::vec3& world_scale) override {
		BaseTransform::Initialize(world_scale);
	}

private:
	virtual void SetViewMatrix() override;
	ReorientViewID type_;
};
