#pragma once

/**=================================================================================================

Project:		Resource
File:			tmj_resource.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-11-27
Last modify: 	2018-11-27

        Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/
#include <map>
#include <memory>
#include <vector>

#if defined(__APPLE__)
#include <glm/gtx/transform.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#else
#include <GL/glm/gtx/transform.hpp>
#include <GL/glm/mat4x4.hpp>
#include <GL/glm/vec3.hpp>
#include <GL/glm/vec4.hpp>
#endif

#include "../../Common/Common/W3Enum.h"
#include "W3Resource.h"

class CW3Image3D;

class RESOURCE_EXPORT TMJfrontalResource {
 public:
  struct Params {
    // volume 에서의 크기 ( real size / spacing ) 을 갖고있음
    float width = 0.0f;
    float thickness = 0.0f;

    Params() : width(0), thickness(0.0f) {}

    Params(float w, float thk) : width(w), thickness(thk) {}
  };

  TMJfrontalResource();
  ~TMJfrontalResource();

 public:
  inline void SetWidth(float width) { params_.width = width; }
  inline void SetThickness(float thickness) { params_.thickness = thickness; }
  inline const float& Width() const noexcept { return params_.width; }

  inline void set_center_position(const glm::vec3& position) {
    center_position_ = position;
  }
  inline void set_up_vector(const glm::vec3& vector) { up_vector_ = vector; }

  inline const glm::vec3& center_position() const { return center_position_; }
  inline const glm::vec3& up_vector() const { return up_vector_; }
  inline const Params& param() const { return params_; }

 private:
  glm::vec3 up_vector_ = glm::vec3(0.0f);
  glm::vec3 center_position_ = glm::vec3(0.0f);

  Params params_;
};

class RESOURCE_EXPORT TMJlateralResource {
 public:
  struct Params {
    int count;
    // volume 에서의 크기 ( real size / spacing ) 을 갖고있음
    float width;
    float thickness;
    float interval;

    Params() : count(0), width(0), thickness(0.0f), interval(0.0f) {}

    Params(int cnt, float w, float thk, float inv)
        : count(cnt), width(w), thickness(thk), interval(inv) {}
  };

 public:
  TMJlateralResource();
  ~TMJlateralResource();

 public:
  inline void set_center_positions(const std::vector<glm::vec3>& positions) {
    center_positions_ = positions;
  }
  inline void set_number(const std::vector<int>& number) { number_ = number; }
  inline void set_up_vector(const glm::vec3& vector) { up_vector_ = vector; }
  inline void set_selected_id(const int& selected_id) noexcept {
    selected_id_ = selected_id;
  }

  inline void SetWidth(float width) { params_.width = width; }
  inline void SetCount(int count) { params_.count = count; }
  inline void SetThickness(float thickness) { params_.thickness = thickness; }
  inline void SetInterval(float interval) { params_.interval = interval; }

  inline bool IsValidCenterPosition(int lateral_id) const;
  inline const float& Width() const noexcept { return params_.width; }
  inline bool GetCenterPosition(int lateral_id,
                                glm::vec3* center_position) const;
  inline bool GetNumber(int lateral_id, int* number) const;
  inline const std::vector<glm::vec3>& center_positions() const {
    return center_positions_;
  }
  inline const std::vector<int>& number() const { return number_; }
  inline const glm::vec3& up_vector() const { return up_vector_; }
  inline const int& selected_id() const noexcept { return selected_id_; }
  inline const Params& param() const { return params_; }

 private:
  glm::vec3 up_vector_ = glm::vec3(0.0f);
  std::vector<glm::vec3> center_positions_;
  std::vector<int> number_;

  int selected_id_ = 0;

  Params params_;
};

class RESOURCE_EXPORT TMJresource : public CW3Resource {
 public:
  TMJresource();
  ~TMJresource();

  TMJresource(const TMJresource&) = delete;
  TMJresource& operator=(const TMJresource&) = delete;

  struct ProjectInfo {
    glm::vec3 rect_center;
    glm::vec3 lateral_up_vector;
    bool is_imported = false;
  };

 public:
  void CreateFrontalResource(const TMJDirectionType& type);
  void CreateLateralResource(const TMJDirectionType& type);
  bool DeleteFrontalResource(const TMJDirectionType& type);
  bool DeleteLateralResource(const TMJDirectionType& type);

  void ImportProject(const TMJDirectionType& type, const glm::vec3& rect_center,
                     const glm::vec3& lateral_up_vector);

      inline void set_axial_position(const glm::vec3& vol_pos) noexcept {
    axial_position_ = vol_pos;
  }
  void SetLateralParam(const TMJLateralID& id, const float& value);
  void SetTMJRectParam(const TMJRectID& id, const float& value);
  void set_height(const float& value) { height_ = value; }
  void set_back_vector(const glm::vec3& back_vector) {
    back_vector_ = back_vector;
  }

  void SetLateralCount(const TMJDirectionType& type, const int& count);

  void SetLateralPositionInfo(const TMJDirectionType& type,
                              const std::map<int, glm::vec3>& positions,
                              const glm::vec3& up_vector);
  void SetFrontalPositionInfo(const TMJDirectionType& type,
                              const glm::vec3& position,
                              const glm::vec3& up_vector);
  void SetSelectedLateralID(const TMJDirectionType& type,
                            const int& lateral_id);
  void SetTMJRectCenter(const TMJDirectionType& type,
                        const glm::vec3& pt_center);

  void set_tmj_mask(const TMJDirectionType& type,
                    const std::weak_ptr<CW3Image3D>& mask) {
    tmj_mask_[type] = mask;
  }
  glm::mat4 GetTMJRoateMatrix(const TMJDirectionType& type) const;
  inline const glm::vec3& axial_position() const noexcept {
    return axial_position_;
  }
  inline const glm::vec3& GetTMJRectCenter(const TMJDirectionType& type) const
      noexcept {
    return rect_center_[type];
  }
  TMJDirectionType GetDirectionType(const TMJLateralID& id) const;
  bool IsValidResource(const TMJDirectionType& type) const;

  inline const TMJfrontalResource& frontal(TMJDirectionType type) const {
    return *frontal_[type].get();
  }
  inline const TMJlateralResource& lateral(TMJDirectionType type) const {
    return *lateral_[type].get();
  }
  inline const CW3Image3D& tmj_mask(TMJDirectionType type) const {
    return *tmj_mask_[type].lock().get();
  }

  inline const float& height() const { return height_; }
  inline const glm::vec3& back_vector() const { return back_vector_; }
  const int& GetSelectedLateralID(const TMJDirectionType& type) const;
  inline const ProjectInfo& project_info(const TMJDirectionType& type) const { return project_info_[type]; }

 private:
  glm::vec3 axial_position_ = glm::vec3();
  glm::vec3 back_vector_ = glm::vec3();
  float height_ = 0.0f;
  glm::vec3 rect_center_[TMJDirectionType::TMJ_TYPE_END];
  std::unique_ptr<TMJfrontalResource> frontal_[TMJDirectionType::TMJ_TYPE_END];
  std::unique_ptr<TMJlateralResource> lateral_[TMJDirectionType::TMJ_TYPE_END];
  std::weak_ptr<CW3Image3D> tmj_mask_[TMJDirectionType::TMJ_TYPE_END];

  ProjectInfo project_info_[TMJDirectionType::TMJ_TYPE_END];
};
