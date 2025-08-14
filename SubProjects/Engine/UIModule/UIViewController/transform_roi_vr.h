#pragma once

/**=================================================================================================

Project:		UIViewController
File:			transform_roi_vr.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-08-31
Last modify: 	2018-08-31

		Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/

#include "base_transform.h"

class TransformROIVR : public BaseTransform {
public:
  TransformROIVR() = default;

  TransformROIVR(const TransformROIVR&) = delete;
  TransformROIVR& operator=(const TransformROIVR&) = delete;

public:
  virtual void Initialize(const glm::vec3& model_scale) override {
	BaseTransform::Initialize(model_scale);
  }
  void SetVolumeCenterPosition(const glm::vec3& center_gl);
  virtual void SetViewMatrix() override;

  virtual void Rotate(float angle, const glm::vec3& rotAxis) override;
  virtual void ForceRotate(const glm::mat4& mat) override;

  inline const glm::mat4& rotate_arcball() const { return rotate_arcball_; }
  inline const glm::vec3& vol_center_position_gl() const {
	return vol_center_position_gl_;
  }

protected:
  void SetRotateArcball(const glm::mat4& rotate);
private:
  glm::mat4 rotate_arcball_;
  glm::mat4 to_rotate_center;
  glm::vec3 vol_center_position_gl_;
};
