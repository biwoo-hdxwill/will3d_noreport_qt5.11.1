#pragma once

/**=================================================================================================

Project:		UIViewController
File:			view_controller_tmj3d.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-12-07
Last modify: 	2018-12-07

	Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/
#include "base_view_controller_3d.h"
#include "uiviewcontroller_global.h"

#include "../../Common/Common/W3Enum.h"
#include "transform_roi_vr.h"

class CW3SurfaceAxesItem;
class CW3Image3D;

class UIVIEWCONTROLLER_EXPORT ViewControllerTMJ3D : public BaseViewController3D {
public:
	explicit ViewControllerTMJ3D(const TMJDirectionType& type);
	~ViewControllerTMJ3D();

	ViewControllerTMJ3D(const ViewControllerTMJ3D&) = delete;
	ViewControllerTMJ3D& operator=(const ViewControllerTMJ3D&) = delete;

public:
	void UpdateCutting(const int& curr_step);

	void SetCutting(bool is_enable);
	virtual void SetCliping(const std::vector<glm::vec4>& planes, bool is_enable) override;
	virtual void SetProjection() override;
	virtual void InitVAOs() override;
	virtual void ClearGL() override;
	virtual bool IsReady() override;

	void InitVAOVBOROIVolume();

	glm::mat4 GetRotateMatrix();
	glm::mat4 GetViewMatrix();

	virtual void ApplyPreferences() override;

	virtual void SetFitMode(BaseTransform::FitMode mode) override;

private:
	enum GL_TEXTURE_HANDLE_IMP {
		TEX_CUT_TMJ_MASK = GL_TEXTURE_HANDLE::TEX_END,
		TEX_IMP_END
	};

	struct CutInfo {
		bool is_cut = false;
		bool is_update = false;
		int curr_step = 0;
		glm::mat4 map_vol_to_mask;
		PackTexture pack;
	};

	bool IsValidVol(const CW3Image3D & vol) const;
	void SetCutMaskTexture();
	virtual void RayCasting() override;

	virtual VolumeRenderer& Renderer() const override;
	virtual void SetPackTexture() override;
	virtual void ReadyBufferHandles() override;
	virtual BaseTransform& transform() const override;

private:
	std::unique_ptr<TransformROIVR> transform_;

	TMJDirectionType direction_type_ = TMJDirectionType::TMJ_TYPE_UNKNOWN;
	std::vector<unsigned int> vbo_vol_;
	CutInfo cut_info_;
	bool is_tmj_ = false;
};
