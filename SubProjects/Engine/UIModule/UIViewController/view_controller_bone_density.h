#pragma once
/**=======================================================================================

Project: 	UIViewController
File:		view_controller_bone_density.h
Language:	C++11
Library:	Qt 5.8.0
author:		Tae Hoon Yoo
date:		2018-05-28

Copyright (c) 2018 All rights reserved by HDXWILL.

*=====================================================================================**/

#include "base_view_controller_3d.h"
#include "uiviewcontroller_global.h"

#include "transform_basic_vr.h"

class ImplantObjGL;
class UIVIEWCONTROLLER_EXPORT ViewControllerBoneDensity : public BaseViewController 
{
public:
	explicit ViewControllerBoneDensity();
	~ViewControllerBoneDensity();

	ViewControllerBoneDensity(const ViewControllerBoneDensity&) = delete;
	ViewControllerBoneDensity& operator=(const ViewControllerBoneDensity&) = delete;

public:
	void RenderingBoneDensity();
	void RenderScreen(uint dfbo);

	void SyncImplant3DCameraMatrix(const glm::mat4& rotate_mat,
		const glm::mat4& reorien_mat,
		const glm::mat4& view_mat);

	virtual void ClearGL() override;
	virtual void ProcessViewEvent(bool* need_render) override;
	virtual void SetProjection() override;
	virtual bool IsReady() override;

	void InitRotateMatrix();
	const glm::mat4& GetRotateMatrix() const;

	float GetRotateAngle();

	virtual void ApplyPreferences() override;
	virtual void SetFitMode(BaseTransform::FitMode mode) override;

private:
	enum GL_TEXTURE_HANDLE { TEX_SCREEN = 0, TEX_END };
	enum GL_DEPTH_HANDLE { DEPTH_DEFAULT = 0, DEPTH_END };

	struct ImplantSpec
	{
		QString file_name;
		QString manufacturer;
		QString product;
		float length = 0.f;
		float diameter = 0.f;
	};

	void ReadyImplantObject();
	VolumeRenderer& Renderer() const;
	void RotateArcBall();
	void SetProjectionFitIn(float world_fit_width, float world_fit_height);

	virtual bool SetRenderer() override;
	virtual void SetPackTexture() override;
	virtual void SetPackTextureHandle() override;
	virtual void ReadyBufferHandles() override;

	BaseTransform& transform();
	int GetSelectedImplantID() const;
	bool IsImplantModelChanged(int selected_id);

	glm::vec3 GetMouseVector(const QPointF& pt_scene);
	glm::vec3 GetArcBallVector(const QPointF& pt_gl);

	QString GetFileName();

private:
	PackTexture pack_screen_;

	std::unique_ptr<TransformBasicVR> transform_;
	std::unique_ptr<ImplantObjGL> implant_;
	ImplantSpec spec_;

	glm::mat4 camera_;
};
