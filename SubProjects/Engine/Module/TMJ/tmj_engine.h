#pragma once

/**=================================================================================================

Project:		TMJ
File:			tmj_engine.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-11-14
Last modify: 	2018-11-14

		Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/
#include <functional>
#include <map>
#include <memory>
#include <vector>

#if defined(__APPLE__)
#include <glm/mat4x4.hpp>
#include <glm/vec3.hpp>
#else
#include <GL/glm/mat4x4.hpp>
#include <GL/glm/vec3.hpp>
#endif

#include <qpoint.h>
#include "../../Common/Common/W3Enum.h"
#include "tmj_global.h"

class QPolygonF;
class CW3Image3D;
class TMJresource;
class TMJfrontalResource;
class TMJlateralResource;
#ifndef WILL3D_VIEWER
class ProjectIOTMJ;
#endif

class TMJ_EXPORT TMJengine {
public:
  TMJengine();
  ~TMJengine();

  TMJengine(const TMJengine&) = delete;
  TMJengine& operator=(const TMJengine&) = delete;

  struct TmjROI {
	float top = 0.0f;
	float bottom = 0.0f;
	float slice = 0.0f;
  };

public:
  void Initialize(const CW3Image3D& volume);
  void InitSliceLocation(const CW3Image3D& volume);

  bool SetLateralParam(const TMJLateralID& id, const float& value);
  void SetTMJRectParam(const TMJRectID& id, const float& value);
  void SetTMJROISlice(const float& slice_pos);
  void SetAxialCenterPosition(const glm::vec3& pt_center);
  void SetTMJROITop(const float& top_pos);
  void SetTMJROIBottom(const float& bottom_pos);
  void SetLatertalCount(const TMJDirectionType& type, const int& value);
  void SetLateralSelectedID(const TMJDirectionType& type,
							const int& lateral_id);
  void SetBackVector(const glm::vec3& back_vector);

  void ReadyFrontalResource(const TMJDirectionType& type);
  void ReadyLateralResource(const TMJDirectionType& type);
  bool DeleteFrontalResource(const TMJDirectionType& type);
  bool DeleteLateralResource(const TMJDirectionType& type);

  void CutTMJ(const TMJDirectionType& direction_type, const QPolygonF& cut_area,
			  const bool& is_inside,
			  const std::function<void(const std::vector<glm::vec3>&,
			  std::vector<QPointF>&)>&
			  FrontalViewMapVolToScene);
  bool ResetCutTMJ(const TMJDirectionType& direction_type);
  void UndoCutTMJ(const TMJDirectionType& direction_type);
  void RedoCutTMJ(const TMJDirectionType& direction_type);

  bool SetTMJRectWidth(const TMJDirectionType& type, const float& width);
  bool SetTMJRectHeight(const TMJDirectionType& type, const float& height);
  bool SetTMJRectCenter(const TMJDirectionType& type,
						const glm::vec3& pt_center);
  bool SetLateralPositionInfo(const TMJDirectionType& type,
							  const std::map<int, glm::vec3>& positions,
							  const glm::vec3& up_vector);
  bool SetFrontalPositionInfo(const TMJDirectionType& type,
							  const glm::vec3& position,
							  const glm::vec3& up_vector);

  const TMJfrontalResource& GetFrontal(const TMJDirectionType& type) const;
  const TMJlateralResource& GetLateral(const TMJDirectionType& type) const;

  const glm::vec3& GetTMJBackVector() const;
  const glm::vec3& GetRectCenter(const TMJDirectionType& type) const;
  const glm::vec3& GetLateralUpVector(const TMJDirectionType& type) const;
  bool GetTMJRectSize(const TMJDirectionType& type, float* width,
					  float* height) const;

  const bool& initialized() const noexcept { return initialized_; }
  bool IsValidTMJ(const TMJDirectionType& type) const;

  inline void set_reorien(const glm::mat4& mat) { reorien_ = mat; }
  inline const glm::mat4& reorien() const { return reorien_; }
  inline const TmjROI& GetTmjROI() const { return roi_vol_; }
  float GetBasePixelSpacing() const;
  const int& GetLateralSelectedID(const TMJDirectionType& type) const;
  inline const int& GetCutCurrStep(const TMJDirectionType& type) const {
	return cut_params_[type].curr_step;
  }

  // serialize
#ifndef WILL3D_VIEWER
  void ExportProject(ProjectIOTMJ& out);
  void ImportProject(ProjectIOTMJ& in);
#endif

private:
  void CreateTMJMask(const TMJDirectionType& type);
  bool ResetCutParams(const TMJDirectionType& type);
  struct CutParams {
	std::shared_ptr<CW3Image3D> mask;
	int curr_step;
	int stack_index;
	bool is_over_stack;
  };

private:
  TmjROI roi_vol_;
  bool initialized_ = false;
  glm::mat4 reorien_;
  std::shared_ptr<TMJresource> tmj_resource_;

  CutParams cut_params_[TMJDirectionType::TMJ_TYPE_END];
};
