#pragma once

/**=================================================================================================

Project: 			UIViewController
File:				base_transform
Language:			C++11
Library:			Qt 5.8.0
author:				Tae Hoon Yoo
First date:			2017-08-24
Last modify:		2017-08-24

Copyright (c) 2017 All rights reserved by HDXWILL.

 *===============================================================================================**/
#include <QRectF>

#if defined(__APPLE__)
#include <glm/vec3.hpp>
#include <glm/detail/type_mat4x4.hpp>
#else
#include <GL/glm/vec3.hpp>
#include <GL/glm/detail/type_mat4x4.hpp>
#endif

#include "uiviewcontroller_types.h"

class BaseTransform {
public:
	enum FitMode
	{
		CONTAIN = 0,
		COVER
	};

	BaseTransform() = default;

	BaseTransform(const BaseTransform&) = delete;
	BaseTransform& operator=(const BaseTransform&) = delete;

public:

	virtual void Initialize(const glm::vec3& model_scale) = 0;
	virtual void SetProjection(const UIViewController::PackViewProj& arg, float & map_scene_to_gl);
	virtual void SetProjectionFitIn(const UIViewController::PackViewProj& arg,
									const float& fit_world_width, const float& fit_world_height,
									float& map_scene_to_gl);
	virtual glm::mat4 SetProjectionFitIn(const UIViewController::PackViewProj& arg,
		const float& fit_world_width, const float& fit_world_height,
		float& map_scene_to_gl, float near, float far);

	void SetCamFOV(float cam_fov);
	virtual void SetViewMatrix() = 0;

	virtual void Rotate(float angle, const glm::vec3& rotAxis);
	virtual void ForceRotate(const glm::mat4& mat);
	virtual void SetReorien(const glm::mat4& mat);

	inline const glm::mat4& mvp() const { return mvp_; }
	inline const glm::mat4& model() const { return model_; }
	inline const glm::mat4& view() const { return view_; }
	inline const glm::mat4& projection() const { return projection_; }
	inline const float cam_fov() const { return cam_fov_; }

	inline const glm::mat4& rotate() const { return rotate_; }
	inline const glm::mat4& reorien() const { return reorien_; }

	glm::mat4 GetCameraMatrix() const;

	inline void set_angle(const float& angle) { angle_ = angle; }
	inline float angle() const { return angle_; }
	inline const QRectF& projection_rect() const { return projection_rect_; }

	inline void set_fit_mode(FitMode mode) { fit_mode_ = mode; }

protected:
	inline void set_view(const glm::mat4& mat) { view_ = mat; }
	inline void set_projection(const glm::mat4& mat) { projection_ = mat; }
	inline void set_projection_rect(const QRectF& rect) { projection_rect_ = rect; }
	inline void set_mvp(const glm::mat4& mat) { mvp_ = mat; }
	inline void set_cam_fov(const float& fov) { cam_fov_ = fov; }
	inline void set_rotate(const glm::mat4& mat) { rotate_ = mat; }
	inline void set_reoren(const glm::mat4& mat) { reorien_ = mat; }

	virtual void UpdateMVP();

private:
	glm::mat4 model_;
	glm::mat4 view_;
	glm::mat4 projection_;
	glm::mat4 mvp_;

	glm::mat4 rotate_;
	glm::mat4 reorien_;

	float cam_fov_ = 1.0f;
	float angle_ = 180.0f;

	QRectF projection_rect_;

	FitMode fit_mode_ = FitMode::CONTAIN;
};
