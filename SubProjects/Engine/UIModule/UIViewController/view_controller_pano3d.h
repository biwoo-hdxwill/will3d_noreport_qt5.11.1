#pragma once

/**=================================================================================================

Project: 			UIViewController
File:				view_controller_pano3d.h
Language:			C++11
Library:			Qt 5.8.0
author:				Tae Hoon Yoo
First date:			2017-08-29
Last modify:		2017-08-29

Copyright (c) 2017 All rights reserved by HDXWILL.

 *===============================================================================================**/
#include "base_view_controller_3d.h"

#include "transform_pano_vr.h"

class GLNerveWidget;
class GLImplantWidget;

class UIVIEWCONTROLLER_EXPORT ViewControllerPano3D : public BaseViewController3D {
public:
	explicit ViewControllerPano3D();
	~ViewControllerPano3D();

	ViewControllerPano3D(const ViewControllerPano3D&) = delete;
	ViewControllerPano3D& operator=(const ViewControllerPano3D&) = delete;

public:
	virtual void SetProjection() override;
	virtual VolumeRenderer& Renderer() const override;
	virtual BaseTransform& transform() const override;

	virtual void ApplyPreferences() override;

	virtual void SetFitMode(BaseTransform::FitMode mode) override;

private:
	virtual glm::vec3 GetArcBallVector(const QPointF & pt_gl) override;

private:
	std::unique_ptr<TransformPanoVR> transform_;
};
