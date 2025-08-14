#include "transform_roi_vr.h"

#include "../../Common/GLfunctions/WGLHeaders.h"

void TransformROIVR::SetVolumeCenterPosition(const glm::vec3& center_gl) {
  vol_center_position_gl_ = center_gl;
  to_rotate_center = glm::translate(-center_gl);
  SetRotateArcball(rotate_arcball_);
  UpdateMVP();
}

void TransformROIVR::Rotate(float angle, const glm::vec3& rotAxis) {
  int sign;
  if (glm::dot(rotAxis, vec3(-1.0, 1.0, -1.0)) > 0.0f)
	sign = 1;
  else
	sign = -1;

  this->set_angle(sign * angle + this->angle());

  rotate_arcball_ = glm::rotate(glm::radians(angle), rotAxis) * rotate_arcball_;

  this->set_rotate(glm::inverse(to_rotate_center) * rotate_arcball_ *
				   to_rotate_center);
  UpdateMVP();
}

void TransformROIVR::ForceRotate(const glm::mat4& mat) {
  SetRotateArcball(mat);
  UpdateMVP();
}

void TransformROIVR::SetRotateArcball(const glm::mat4& rotate) {
  rotate_arcball_ = rotate;
  this->set_rotate(glm::inverse(to_rotate_center) * rotate_arcball_ *
				   to_rotate_center);

}

void TransformROIVR::SetViewMatrix() {
  set_view(
	  glm::lookAt(vol_center_position_gl_ + glm::vec3(0.0f, -cam_fov(), 0.0f),
				  vol_center_position_gl_, glm::vec3(0.0f, 0.0f, -1.0f)));
  UpdateMVP();
}
