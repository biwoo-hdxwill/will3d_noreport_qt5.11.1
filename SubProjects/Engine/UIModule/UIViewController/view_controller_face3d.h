#pragma once
/**=================================================================================================

Project: 			UIViewController
File:				base_view_controller_3d.h
Language:			C++11
Library:			Qt 5.8.0
author:				Tae Hoon Yoo
First date:			2017-08-31
Last modify:		2017-08-31

Copyright (c) 2017 All rights reserved by HDXWILL.

 *===============================================================================================**/
#include "base_view_controller_3d.h"
#include "uiviewcontroller_global.h"

#include "transform_basic_vr.h"

class CW3SurfaceItem;

class UIVIEWCONTROLLER_EXPORT ViewControllerFace3D : public BaseViewController3D {
public:
	explicit ViewControllerFace3D();
	~ViewControllerFace3D();

	ViewControllerFace3D(const ViewControllerFace3D&) = delete;
	ViewControllerFace3D& operator=(const ViewControllerFace3D&) = delete;

public:
	virtual void ClearGL() override;

	void SetTFupdated(bool is_min_max_changed);
	void SetTransparencySurfaceFace(float alpha);
	void SetVisibleSurfaceFace(bool is_visible);

	void LoadFace3D();

	virtual void ApplyPreferences() override;

	virtual void SetFitMode(BaseTransform::FitMode mode) override;

private:
	virtual void InitVAOs() override;
	virtual VolumeRenderer& Renderer() const override;

	virtual void SetSurfaceMVP() override;
	virtual void DrawBackFaceSurface() override;
	virtual void DrawSurface() override;
	virtual void DrawTransparencySurface() override;
	virtual BaseTransform& transform() const override;

	void SetSurfaceFaceMVP();

	void DrawSurfaceFaceBackFace(uint prog);
	void DrawSurfaceFace(uint prog);

private:
	std::unique_ptr<TransformBasicVR> transform_;

	//TODO. surface 관리 클래스 만들어서 이동 할 것.
	std::unique_ptr<CW3SurfaceItem> surface_face_;

	uint tex_handler_face_;
};
