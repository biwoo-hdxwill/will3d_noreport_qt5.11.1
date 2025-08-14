#include "transform_slice.h"

#include "../../Common/GLfunctions/WGLHeaders.h"
#include "../../Common/GLfunctions/gl_transform_functions.h"
#include "../../Common/GLfunctions/gl_helper.h"
using glm::vec3;
using glm::vec4;

using namespace UIViewController;
namespace {
	const glm::vec3 kInvAxisX(-1.0f, 1.0f, 1.0f);
}

TransformSlice::TransformSlice() {
	plane_equation_ = vec4(kInvAxisX*kUpVector, 0.0f);
	plane_center_position_ = glm::vec3(0.0f);
	plane_right_vector_ = kInvAxisX*kRightVector;
	plane_back_vector_ = kInvAxisX*kBackVector;
	this->SetTransformMat();
}
inline void TransformSlice::Initialize(const glm::vec3 & model_scale) {
	BaseTransform::Initialize(model_scale);
}

void TransformSlice::SetProjection(const UIViewController::PackViewProj & arg, float & map_scene_to_gl) 
{
	this->SetProjectionFitIn(arg, GLhelper::ScaleVolToGL(plane_width_),
							 GLhelper::ScaleVolToGL(plane_height_), map_scene_to_gl);

	projection_for_implant_ = SetProjectionFitIn(arg, GLhelper::ScaleVolToGL(plane_width_),
		GLhelper::ScaleVolToGL(plane_height_), map_scene_to_gl, 0.0f, 0.001f);
}

void TransformSlice::SetPlane(const glm::vec3& center_pos_gl,
							  const glm::vec3& right_vector_gl,
							  const glm::vec3& back_vector_gl) {
	vec3 up_vector = glm::normalize(glm::cross(back_vector_gl, right_vector_gl));
	float plane_d = glm::dot(up_vector, center_pos_gl);

	plane_equation_ = vec4(up_vector, -plane_d);
	plane_right_vector_ = glm::normalize(right_vector_gl);
	plane_back_vector_ = glm::normalize(back_vector_gl);
	plane_center_position_ = center_pos_gl;

	plane_width_ = glm::length(right_vector_gl);
	plane_height_ = glm::length(back_vector_gl);
	this->SetViewMatrix();
	this->SetTransformMat();
	UpdateMVP();
}
void TransformSlice::SetRotate(const glm::mat4 & mat)
{
	set_rotate(mat);
	this->SetViewMatrix();
	this->SetTransformMat();
	UpdateMVP();
}
void TransformSlice::TranslateZ(float glZpos)
{
	gl_z_position_ = glZpos;
	z_translate_ = glm::translate(glm::vec3(0.0f, 0.0f, glZpos));
	this->SetTransformMat();
	UpdateMVP();
}

glm::vec3 TransformSlice::GetUpVector() const {
	return  kInvAxisX*glm::vec3(glm::inverse(rotate())*plane_equation_);
}

glm::vec3 TransformSlice::GetRightVector() const {
	return  kInvAxisX*glm::vec3(glm::inverse(rotate())*glm::vec4(plane_right_vector_, 1.0f));
}

glm::vec3 TransformSlice::GetBackVector() const
{
	return  kInvAxisX * glm::vec3(glm::inverse(rotate())*glm::vec4(plane_back_vector_, 1.0f));
}

void TransformSlice::SetViewMatrix() 
{
	glm::vec3 plane_up_vector = glm::vec3(plane_equation_.x,
										  plane_equation_.y,
										  plane_equation_.z);
	glm::vec3 dir = glm::normalize(plane_up_vector) * cam_fov();
	glm::vec3 eye = dir + plane_center_position_;
	glm::vec3 center = plane_center_position_;
	
	glm::vec3 cam_up_vector = -plane_back_vector_;
	set_view(glm::lookAt(eye, center, cam_up_vector));
	
	eye -= dir;
	center -= dir;
	view_for_implant_ = glm::lookAt(eye, center, cam_up_vector);
	UpdateMVP();
}

void TransformSlice::UpdateMVP() 
{
	set_mvp(projection() * view() * plane_transform_ * z_translate_ * model());
}

void TransformSlice::SetTransformMat()
{
	glm::vec3 plane_up_vector = glm::vec3(plane_equation_);
	glm::mat4 plane_rotate = GLTransformFunctions::GetRotMatrixVecToVec(kUpVector, plane_up_vector);
	glm::mat4 plane_translate = glm::translate(plane_center_position_);
	glm::mat4 plane_right_vector_rotate = GLTransformFunctions::GetRightRotateMatrix(plane_rotate,
																					 plane_right_vector_,
																					 kInvAxisX*kRightVector);

	plane_transform_ = plane_translate * plane_right_vector_rotate * plane_rotate;
	
	transform_mat_ = plane_transform_ * z_translate_ * glm::inverse(rotate());
	vol_tex_transform_mat_ = glm::inverse(model()) * transform_mat_ * model();
}
