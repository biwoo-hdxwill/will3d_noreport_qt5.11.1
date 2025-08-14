#include "base_transform.h"

#include "../../Common/GLfunctions/WGLHeaders.h"
#include "../../Common/GLfunctions/gl_helper.h"
#include "base_transform.h"

using glm::vec3;
using glm::vec4;

using namespace UIViewController;


void BaseTransform::Initialize(const glm::vec3 & model_scale)
{
	cam_fov_ = glm::length(model_scale)*sqrt(2.0f);
	model_ = glm::scale(model_scale);
	
	SetViewMatrix();
	UpdateMVP();
}


void BaseTransform::SetProjection(const PackViewProj & arg,
	float & map_scene_to_gl)
{
	if (arg.width == 0 ||
		arg.height == 0)
		return;

	float left, right, bottom, top;

	if ((arg.width > arg.height && fit_mode_ == FitMode::CONTAIN) ||
		(arg.width < arg.height && fit_mode_ == FitMode::COVER))
	{
		float ratio = (float)arg.width / (float)arg.height*cam_fov_;
		map_scene_to_gl = cam_fov_ / arg.scale / arg.height;

		left = -ratio / arg.scale + arg.trans_x;
		right = ratio / arg.scale + arg.trans_x;
		bottom = -cam_fov_ / arg.scale - arg.trans_y;
		top = cam_fov_ / arg.scale - arg.trans_y;
	}
	else
	{
		float ratio = (float)arg.height / (float)arg.width*cam_fov_;
		map_scene_to_gl = cam_fov_ / arg.scale / arg.width;

		left = -cam_fov_ / arg.scale + arg.trans_x;
		right = cam_fov_ / arg.scale + arg.trans_x;
		bottom = -ratio / arg.scale - arg.trans_y;
		top = ratio / arg.scale - arg.trans_y;
	}
	map_scene_to_gl = GLhelper::ScaleSceneToGL(map_scene_to_gl);

	projection_ = glm::ortho(left, right, bottom, top, 0.0f, cam_fov_*2.0f);
	projection_rect_ = QRectF(left, bottom, right - left, top - bottom);

	UpdateMVP();
}

void BaseTransform::SetProjectionFitIn(
	const UIViewController::PackViewProj & arg,
	const float& fit_world_width, const float& fit_world_height,
	float & map_scene_to_gl)
{
	set_projection(SetProjectionFitIn(arg, fit_world_width, fit_world_height, map_scene_to_gl, 0.0f, cam_fov_ * 2.0f));

	UpdateMVP();
}

glm::mat4 BaseTransform::SetProjectionFitIn(const UIViewController::PackViewProj& arg,
	const float& fit_world_width, const float& fit_world_height,
	float& map_scene_to_gl, float near, float far)
{
	if (arg.width == 0 ||
		arg.height == 0)
		return glm::mat4();

	if (fit_world_width == 0 ||
		fit_world_height == 0)
		return glm::mat4();

	const float fit_world_haf_width = fit_world_width * 0.5f;
	const float fit_world_haf_height = fit_world_height * 0.5f;
	float left, right, bottom, top;

	const float view_width = (float)arg.width;
	const float view_height = (float)arg.height;

	const float view_ratio = view_width / view_height;
	const float fit_ratio = fit_world_haf_width / fit_world_haf_height;

	if ((view_ratio > fit_ratio && fit_mode_ == FitMode::CONTAIN) ||
		(view_ratio < fit_ratio && fit_mode_ == FitMode::COVER))
	{
		map_scene_to_gl = fit_world_haf_height / (arg.scale * view_height);

		left = -fit_world_haf_height*view_ratio / arg.scale + arg.trans_x;
		right = fit_world_haf_height*view_ratio / arg.scale + arg.trans_x;
		bottom = -fit_world_haf_height / arg.scale - arg.trans_y;
		top = fit_world_haf_height / arg.scale - arg.trans_y;
	}
	else
	{
		map_scene_to_gl = fit_world_haf_width / (arg.scale * view_width);

		left = -fit_world_haf_width / arg.scale + arg.trans_x;
		right = fit_world_haf_width / arg.scale + arg.trans_x;
		bottom = -fit_world_haf_width / (view_ratio*arg.scale) - arg.trans_y;
		top = fit_world_haf_width / (view_ratio*arg.scale) - arg.trans_y;
	}

	map_scene_to_gl = GLhelper::ScaleSceneToGL(map_scene_to_gl);

	projection_rect_ = QRectF(left, bottom, right - left, top - bottom);

	return glm::ortho(left, right, bottom, top, near, far);
}

void BaseTransform::SetCamFOV(float cam_fov) {
	cam_fov_ = cam_fov;
}

void BaseTransform::Rotate(float angle, const glm::vec3 & rotAxis)
{
	int sign;
	if (glm::dot(rotAxis, vec3(-1.0, 1.0, -1.0)) > 0.0f)
		sign = 1;
	else sign = -1;


	angle_ += sign * angle;

	rotate_ = glm::rotate(glm::radians(angle), rotAxis)*rotate_;
	UpdateMVP();
}

void BaseTransform::ForceRotate(const glm::mat4 & mat)
{
	//implant sagittal view의 angle초기화 때문에 angle을 초기화 한 것으로 추정된다.
	//angle_ = kInitAngle; 
	rotate_ = mat;
	UpdateMVP();
}

void BaseTransform::SetReorien(const glm::mat4& mat) {
	reorien_ = mat;
	UpdateMVP();
}

void BaseTransform::UpdateMVP()
{
	mvp_ = projection_*GetCameraMatrix()*model_;
}

glm::mat4 BaseTransform::GetCameraMatrix() const
{
	return view()*rotate()*reorien();
}
