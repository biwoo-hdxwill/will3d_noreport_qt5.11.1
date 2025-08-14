#pragma once

/**=================================================================================================

Project: 			UIViewController
File:				transform_slice.h
Language:			C++11
Library:			Qt 5.8.0
author:				Tae Hoon Yoo
First date:			2017-07-20
Last modify:		2017-07-20

 *===============================================================================================**/

#include "base_transform.h"


class TransformSlice : public BaseTransform
{
public:
	TransformSlice();

	TransformSlice(const TransformSlice&) = delete;
	TransformSlice& operator=(const TransformSlice&) = delete;

public:
	virtual void Initialize(const glm::vec3& model_scale) override;
	virtual void SetProjection(const UIViewController::PackViewProj& arg, float & map_scene_to_gl) override;
	void SetPlane(const glm::vec3& center_pos_gl,
				  const glm::vec3& right_vector,
				  const glm::vec3& back_vector);
	
	virtual void SetRotate(const glm::mat4& mat);
	void TranslateZ(float z_pos_gl);

	glm::vec3 GetUpVector() const;
	glm::vec3 GetRightVector() const;
	glm::vec3 GetBackVector() const;

	inline const glm::mat4& transform_mat() const { return transform_mat_; }
	inline const glm::mat4& vol_tex_transform_mat() const { return vol_tex_transform_mat_; }
	inline const glm::mat4& plane_transform() const { return plane_transform_; }

	inline const glm::mat4& z_translate() const { return z_translate_; }
	inline const glm::vec4& plane_equation() const { return plane_equation_; }
	inline const float& gl_z_position() const { return gl_z_position_; }
	inline const glm::vec3& plane_center_position() const { return plane_center_position_; }
	inline const glm::vec3& plane_right_vector() const { return plane_right_vector_; }
	inline const glm::vec3& plane_back_vector() const { return plane_back_vector_; }

	inline const float& plane_width() const { return plane_width_; }
	inline const float& plane_height() const { return plane_height_; }

	inline const glm::mat4& view_for_implant() const { return view_for_implant_; }
	inline const glm::mat4& projection_for_implant() const { return projection_for_implant_; }

private:
	virtual void SetViewMatrix() override;
	virtual void UpdateMVP() override;
	void SetTransformMat();

private:
	glm::mat4 view_for_implant_;
	glm::mat4 projection_for_implant_;

	glm::mat4 transform_mat_;
	glm::mat4 vol_tex_transform_mat_;

	glm::vec4 plane_equation_;
	glm::vec3 plane_right_vector_;
	glm::vec3 plane_back_vector_;
	glm::vec3 plane_center_position_;
	glm::mat4 plane_transform_;

	glm::mat4 z_translate_;
	float gl_z_position_ = 0.0f;
	float plane_width_ = 0.0f;
	float plane_height_ = 0.0f;
};
