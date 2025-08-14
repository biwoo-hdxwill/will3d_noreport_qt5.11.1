#pragma once
/**=================================================================================================

Project: 			MPREngine
File:				mpr_module.h
Language:			C++11
Library:			Qt 5.8.0
author:				Seo Seok Man
First date:			2018-10-10
Last modify:		2018-10-10

 *===============================================================================================**/
#include <vector>
#if defined(__APPLE__)
#include <glm\glm.hpp>
#else
#include <GL\glm\glm.hpp>
#endif

class QPointF;
class LightboxResource;

class MPRModule {
 public:
  MPRModule(const MPRModule&) = delete;
  MPRModule& operator=(const MPRModule&) = delete;

 public:
  // lightbox functions
  static void InitLightboxPlanes(LightboxResource* res_lightbox,
                                 const glm::vec3& vol_center,
                                 const glm::vec3& vol_range,
                                 const float& pixel_spacing,
                                 const float& slice_spacing,
                                 const glm::vec3& cross_pt);
  static void CalcLightboxPositions(LightboxResource* res_lightbox,
                                    const glm::vec3& cross_pt,
                                    const glm::vec3& vol_center,
                                    const glm::vec3& vol_range);
  static void TranslateLightboxPlanes(LightboxResource* res_lightbox,
                                      const glm::vec3& vol_center,
                                      const glm::vec3& vol_range,
                                      const int& lightbox_id,
                                      const int& slider_value);
  static void TranslateLightboxPlanes(LightboxResource* res_lightbox,
                                      const glm::vec3& vol_center,
                                      const glm::vec3& vol_range,
                                      const float& delta);
  static void GetLightboxSliderPosition(LightboxResource* res_lightbox,
                                        const glm::vec3& vol_center,
                                        std::vector<int>& slider_positions);

  static glm::vec3 MapLightboxPlaneToVol(LightboxResource* res_lightbox,
                                         const int& lightbox_id,
                                         const QPointF& pt_lightbox_plane);
  static void MapLightboxPlaneToVol(LightboxResource* res_lightbox,
                                    const int& lightbox_id,
                                    const std::vector<QPointF>& plane_points,
                                    std::vector<glm::vec3>& vol_points);

  static void SetLightboxInterval(LightboxResource* res_lightbox,
                                  const float& real_interval,
                                  const float& pixel_spacing,
                                  const float& slice_spacing,
                                  const glm::vec3& cross_pt,
                                  const glm::vec3& vol_center,
                                  const glm::vec3& vol_range);
  static void SetLightboxThickness(LightboxResource* res_lightbox,
                                   const float& real_thickness,
                                   const float& pixel_spacing,
                                   const float& slice_spacing);

 private:
  static void TranslateLightbox(LightboxResource* res_lightbox,
                                const glm::vec3& trans,
                                const glm::vec3& vol_range);
  static void CalcLightboxPlaneCenter(LightboxResource* res_lightbox);
  static void CalcLightboxInterval(LightboxResource* res_lightbox,
                                   const float& real_interval,
                                   const float& pixel_spacing,
                                   const float& slice_spacing);
  static void CalcLightboxThickness(LightboxResource* res_lightbox,
                                    const float& real_interval,
                                    const float& pixel_spacing,
                                    const float& slice_spacing);

  static glm::vec3 FitVolRange(const glm::vec3& source, const glm::vec3& dest,
                               const glm::vec3& vol_range);
  static bool IsInVolRange(const glm::vec3& pos, const glm::vec3& vol_range);
};
