#pragma once
/*=========================================================================
File:			view_controller_pacs_3d.h
Language:		C++11
Library:        Qt 5.9.9
Author:			Lim Tae Kyun
First date:		2021-07-15
Last modify:	2021-07-15

Copyright (c) 2018 HDXWILL. All rights reserved.
=========================================================================*/

#include "base_view_controller_3d.h"
#include "transform_basic_vr.h"

class UIVIEWCONTROLLER_EXPORT ViewControllerPACS3D : public BaseViewController3D
{
public:
	ViewControllerPACS3D();
	~ViewControllerPACS3D();

	ViewControllerPACS3D(const ViewControllerPACS3D&) = delete;
	ViewControllerPACS3D& operator=(const ViewControllerPACS3D&) = delete;

public:
	void SetTFupdated(bool is_min_max_changed);

	virtual void ClearGL() override;
	virtual void SetFitMode(BaseTransform::FitMode mode) override;
	bool GetTextureData(unsigned char*& out_data, const int width, const int height, const glm::mat4& rot_mat);

private:
	virtual void InitVAOs() override;
	virtual VolumeRenderer& Renderer() const override;
	virtual BaseTransform& transform() const override;

private:
	std::unique_ptr<TransformBasicVR> transform_;

	unsigned int off_fb_ = 0;

	std::vector<uint> off_depth_handler_;
	std::vector<uint> off_tex_buffer_;
	std::vector<uint> off_tex_handler_;
	std::vector<uint> off_tex_num_;	
};
