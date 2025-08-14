#pragma once
/**=======================================================================================

Project: 	UIViewController
File:		view_controller_basic_vr.h
Language:	C++11
Library:	Qt 5.8.0
author:		Seo Seok Man
date:		2018-08-31

Copyright (c) 2018 All rights reserved by HDXWILL.

*=====================================================================================**/
#include "base_view_controller_3d.h"
#include "uiviewcontroller_global.h"

#include "transform_implant_3d.h"

class CW3SurfaceAxesItem;
class ImplantData;

class UIVIEWCONTROLLER_EXPORT ViewControllerImplant3D : public BaseViewController3D
{
public:
	explicit ViewControllerImplant3D();
	~ViewControllerImplant3D();

	ViewControllerImplant3D(const ViewControllerImplant3D&) = delete;
	ViewControllerImplant3D& operator=(const ViewControllerImplant3D&) = delete;

public:
	glm::mat4 GetCollisionProjectionViewMatrix() const;
	void MoveImplant(int* implant_id,
		glm::vec3* delta_translate,
		glm::vec3* rotate_axes,
		float* delta_degree);
	void PickAxesItem(bool* is_update_scene);
	void RenderForPickAxes();

	virtual void SetProjection() override;
	virtual void InitVAOs() override;
	virtual void ClearGL() override;

	void SelectImplant(int* implant_id);
	bool IsPickImplant() const;
	int GetPickImplantID() const;

	const glm::mat4& GetRotateMatrix() const;
	const glm::mat4& GetReorienMatrix() const;
	glm::mat4 GetNavigatorViewMatrix();

	inline const bool is_selected_implant() const { return is_selected_implant_; }

	virtual BaseTransform& transform() const override;

	virtual void ApplyPreferences() override;

	virtual void SetFitMode(BaseTransform::FitMode mode) override;

	void ClipPanoArea(const bool clip);

private:
	enum GL_TEXTURE_HANDLE_IMP
	{
		TEX_PICK_IMPLANT = GL_TEXTURE_HANDLE::TEX_END,
		TEX_PICK_AXIS,
		TEX_IMP_END
	};

	void InitVAOVBOArchVolume();
	ImplantData* GetImplantData() const;

	virtual void SetSurfaceMVP() override;
	void SetSurfaceAxesMVP();
	virtual void DrawOverwriteSurface() override;

	virtual VolumeRenderer& Renderer() const override;

	virtual void SetPackTexture() override;
	virtual void SetPackTextureHandle() override;
	virtual void ReadyBufferHandles() override;

private:
	PackTexture pack_pick_implant_;
	PackTexture pack_pick_axes_;

	std::unique_ptr<TransformImplant3D> transform_;
	std::unique_ptr<CW3SurfaceAxesItem> axes_item_;

	bool is_selected_implant_ = false;

	std::vector<unsigned int> vbo_vol_;

	bool clip_pano_area_ = true;
};
