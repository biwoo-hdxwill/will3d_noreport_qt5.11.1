#pragma once

/**=================================================================================================

Project:		UIViewController
File:			view_controller_orientation.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-02-19
Last modify: 	2018-02-19

	Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/
#include "../../Common/Common/W3Enum.h"

#include "base_view_controller_3d.h"
#include "uiviewcontroller_types.h"

#include "transform_pano_orientation.h"

class CW3LineItem;


class UIVIEWCONTROLLER_EXPORT ViewControllerOrientation : public BaseViewController3D
{
public:
	explicit ViewControllerOrientation(ReorientViewID type);
	~ViewControllerOrientation();

public:
	ViewControllerOrientation(const ViewControllerOrientation&) = delete;
	ViewControllerOrientation& operator=(const ViewControllerOrientation&) = delete;

public:
	virtual void ClearGL() override;

	void SetRotateAngle(int angle);
	float GetRotateAngle() const;

	virtual void ApplyPreferences() override;

	virtual void SetFitMode(BaseTransform::FitMode mode) override;

private:
	virtual VolumeRenderer& Renderer() const override;

	virtual BaseTransform& transform() const override;
	virtual glm::vec3 GetArcBallVector(const QPointF & pt_gl) override;

private:
	std::unique_ptr<TransformPanoOrientation> transform_;
	ReorientViewID type_;
};
