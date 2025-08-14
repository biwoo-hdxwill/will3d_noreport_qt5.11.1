#pragma once
/**=================================================================================================

Project:		MPREngine
File:			mpr_engine.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Seo Seok Man
First date:		2018-10-10
Last modify: 	2018-10-10

        Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/
#include <memory>
#include <vector>

#if defined(__APPLE__)
#include <glm\glm.hpp>
#else
#include <GL\glm\glm.hpp>
#endif

#include "../../Common/Common/W3Enum.h"
#include "mprengine_global.h"

class LightboxResource;
class QPointF;

namespace lightbox_resource {
typedef struct _LightboxParams LightboxParams;
typedef struct _PlaneParams PlaneParams;
}  // namespace lightbox_resource

class MPRENGINE_EXPORT MPREngine {
 public:
  MPREngine();
  ~MPREngine();

  MPREngine(const MPREngine&) = delete;
  MPREngine& operator=(const MPREngine&) = delete;

 public:
  // void Initialize(const CW3Image3D& volume); // 추후 볼륨만 넘겨받아
  // initialize 한다.
  void Initialize(const glm::vec3& mpr_cross_point);
  void InitLightboxResource(
      const glm::vec3& plane_center,
      const lightbox_resource::LightboxParams& lightbox_params,
      const lightbox_resource::PlaneParams& plane_params);
  bool SetLightboxToMaximizeMode(const int& lightbox_id);
  void ChangeLightboxCount(const int& lightbox_cnt_row,
                           const int& lightbox_cnt_col);

  void GetLightboxCount(int& lightbox_cnt_row, int& lightbox_cnt_col);
  void GetLightboxSliderPositions(std::vector<int>& slider_positions);
  const LightboxViewType GetLightboxViewType() const;
  void GetProfileDataInLightboxPlane(const int& lightbox_id,
                                     const QPointF& start_pt_plane,
                                     const QPointF& end_pt_plane,
                                     std::vector<short>& data);
  void GetROIDataInLightboxPlane(const int& lightbox_id,
                                 const QPointF& start_pt_plane,
                                 const QPointF& end_pt_plane,
                                 std::vector<short>& data);

  void TranslateLightbox(const int& lightbox_id, const int& slider_value);
  void TranslateLightbox(const float& translate_delta);
  glm::vec4 MapLightboxPlaneToVol(const int& lightbox_id,
                                  const QPointF& pt_lightbox_plane);
  const int GetLightboxAvailableDepth() const;
  void SetLightboxInterval(const float& real_interval);
  void SetLightboxThickness(const float& real_thickness);

  bool IsLightboxAvailableMaximize() const;

 private:
  inline LightboxResource* res_lightbox() const { return res_lightbox_.get(); }

 private:
  std::shared_ptr<LightboxResource> res_lightbox_;
  glm::vec3 mpr_cross_point_ = glm::vec3(0.0f);
  glm::vec3 mpr_vol_range_ = glm::vec3(0.0f);
  glm::vec3 mpr_vol_center_ = glm::vec3(0.0f);
};
