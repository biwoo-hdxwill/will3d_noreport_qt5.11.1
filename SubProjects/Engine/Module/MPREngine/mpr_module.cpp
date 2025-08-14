#include "mpr_module.h"

#include <QPointF>

#include "../../Common/Common/W3Enum.h"
#include "../../Common/Common/W3Math.h"
#include "../../Resource/ResContainer/resource_container.h"
#include "../../Resource/Resource/W3Image3D.h"
#include "../../Resource/Resource/lightbox_resource.h"

namespace 
{
	const float kRoundVal = 0.499999f;
}  // end of namespace

void MPRModule::InitLightboxPlanes(LightboxResource* res_lightbox,
                                   const glm::vec3& vol_center,
                                   const glm::vec3& vol_range,
                                   const float& pixel_spacing,
                                   const float& slice_spacing,
                                   const glm::vec3& cross_pt) {
  const float& real_interval = res_lightbox->GetLightboxInterval();
  CalcLightboxInterval(res_lightbox, real_interval, pixel_spacing,
                       slice_spacing);
  const float& real_thickness = res_lightbox->GetLightboxThickness();
  CalcLightboxThickness(res_lightbox, real_thickness, pixel_spacing,
                        slice_spacing);

  CalcLightboxPositions(res_lightbox, cross_pt, vol_center, vol_range);
}

void MPRModule::TranslateLightboxPlanes(LightboxResource* res_lightbox,
                                        const glm::vec3& vol_center,
                                        const glm::vec3& vol_range,
                                        const int& lightbox_id,
                                        const int& slider_value) {
  // Get previous slider position
  const glm::vec3 direction = res_lightbox->GetLighboxDirection();
  const float available_depth =
      static_cast<float>(res_lightbox->GetPlaneAvailableDepth());
  const glm::vec3& plane_center = res_lightbox->GetPlaneCenter(lightbox_id);
  const glm::vec3 distance = plane_center - vol_center;
  float prev_dist = glm::dot(distance, direction);
  int prev_slider_value =
      static_cast<int>(floorf(prev_dist + available_depth * 0.5f + kRoundVal)) -
      1;

  float delta = static_cast<float>(slider_value - prev_slider_value);
  const glm::vec3 dist = res_lightbox->GetLighboxDirection() *
                         res_lightbox->GetLightboxInterval() * delta;
  const glm::vec3 cross = (delta > 0.0f) ? res_lightbox->last_cross_pt()
                                         : res_lightbox->first_cross_pt();
  const glm::vec3 v_cross = cross + dist;
  if (IsInVolRange(v_cross, vol_range)) {
    TranslateLightbox(res_lightbox, dist, vol_center);
  } else {
    const glm::vec3 fit_cross = FitVolRange(cross, v_cross, vol_range);
    TranslateLightbox(res_lightbox, fit_cross - cross, vol_center);
  }
}

void MPRModule::TranslateLightboxPlanes(LightboxResource* res_lightbox,
                                        const glm::vec3& vol_center,
                                        const glm::vec3& vol_range,
                                        const float& delta) {
  const glm::vec3 dist = res_lightbox->GetLighboxDirection() *
                         res_lightbox->GetLightboxInterval() * delta;
  const glm::vec3 cross = (delta > 0.0f) ? res_lightbox->last_cross_pt()
                                         : res_lightbox->first_cross_pt();
  const glm::vec3 v_cross = cross + dist;
  if (IsInVolRange(v_cross, vol_range)) {
    TranslateLightbox(res_lightbox, dist, vol_center);
  } else {
    const glm::vec3 fit_cross = FitVolRange(cross, v_cross, vol_range);
    TranslateLightbox(res_lightbox, fit_cross - cross, vol_center);
  }
}

void MPRModule::GetLightboxSliderPosition(LightboxResource* res_lightbox,
                                          const glm::vec3& vol_center,
                                          std::vector<int>& slider_positions) {
  const int lightbox_count = res_lightbox->GetLightboxCount();
  slider_positions.reserve(lightbox_count);

  const glm::vec3 direction = res_lightbox->GetLighboxDirection();
  const float available_depth =
      static_cast<float>(res_lightbox->GetPlaneAvailableDepth());
  for (int id = 0; id < lightbox_count; ++id) {
    const glm::vec3& plane_center = res_lightbox->GetPlaneCenter(id);
    const glm::vec3 distance = plane_center - vol_center;
    float dist = glm::dot(distance, direction);
    int slider_value =
        static_cast<int>(floorf(dist + available_depth * 0.5f + kRoundVal)) - 1;

    slider_positions.push_back(slider_value);
  }
}

glm::vec3 MPRModule::MapLightboxPlaneToVol(LightboxResource* res_lightbox,
                                           const int& lightbox_id,
                                           const QPointF& pt_lightbox_plane) {
  const float plane_w = static_cast<float>(res_lightbox->GetPlaneWidth());
  const float plane_h = static_cast<float>(res_lightbox->GetPlaneHeight());
  const glm::vec3& vR = res_lightbox->GetPlaneRight();
  const glm::vec3& vB = res_lightbox->GetPlaneBottom();

  return res_lightbox->GetPlaneCenter(lightbox_id) +
         vR * (static_cast<float>(pt_lightbox_plane.x()) -
               (plane_w - 1.0f) * 0.5f) +
         vB * (static_cast<float>(pt_lightbox_plane.y()) -
               (plane_h - 1.0f) * 0.5f);
}

void MPRModule::MapLightboxPlaneToVol(LightboxResource* res_lightbox,
                                      const int& lightbox_id,
                                      const std::vector<QPointF>& plane_points,
                                      std::vector<glm::vec3>& vol_points) {
  vol_points.clear();
  vol_points.reserve(plane_points.size());

  for (const auto& pt_plane : plane_points)
    vol_points.push_back(
        MapLightboxPlaneToVol(res_lightbox, lightbox_id, pt_plane));
}

void MPRModule::SetLightboxInterval(LightboxResource* res_lightbox,
                                    const float& real_interval,
                                    const float& pixel_spacing,
                                    const float& slice_spacing,
                                    const glm::vec3& cross_pt,
                                    const glm::vec3& vol_center,
                                    const glm::vec3& vol_range) {
  CalcLightboxInterval(res_lightbox, real_interval, pixel_spacing,
                       slice_spacing);
  CalcLightboxPositions(res_lightbox, cross_pt, vol_center, vol_range);
}

void MPRModule::SetLightboxThickness(LightboxResource* res_lightbox,
                                     const float& real_thickness,
                                     const float& pixel_spacing,
                                     const float& slice_spacing) {
  CalcLightboxThickness(res_lightbox, real_thickness, pixel_spacing,
                        slice_spacing);
}

void MPRModule::TranslateLightbox(LightboxResource* res_lightbox,
                                  const glm::vec3& trans,
                                  const glm::vec3& vol_center) {
  res_lightbox->first_cross_pt() += trans;
  res_lightbox->last_cross_pt() += trans;

  glm::vec3 up_vector = res_lightbox->GetPlaneUp();
  float sign = glm::dot(trans, up_vector) > 0.0f ? 1.0f : -1.0f;
  float dist_from_vol_center = res_lightbox->dist_from_vol_center();
  dist_from_vol_center += sign * glm::length(trans);
  res_lightbox->dist_from_vol_center() = dist_from_vol_center;
  res_lightbox->InitLightboxData(0,
                                 vol_center + up_vector * dist_from_vol_center);
  CalcLightboxPlaneCenter(res_lightbox);
}

void MPRModule::CalcLightboxPositions(LightboxResource* res_lightbox,
                                      const glm::vec3& cross_pt,
                                      const glm::vec3& vol_center,
                                      const glm::vec3& vol_range) {
  /* // pseudo code //
          calc last plane's virtual cross_pt(last_v_cross)
          if last_v_cross is in range
                  calc lightbox planes
          else
                  fit last_v_cross to vol_range(last_fit_cross)
                  get vector dist from last_v_cross to last_fit_cross
                  translate first plane with dist
                  calc lightbox planes
  */

  const float dist = res_lightbox->GetLightboxInterval() *
                     static_cast<float>(res_lightbox->GetLightboxCount());
  const glm::vec3 last_v_cross =
      cross_pt + res_lightbox->GetLighboxDirection() * dist;
  res_lightbox->first_cross_pt() = cross_pt;
  res_lightbox->last_cross_pt() = last_v_cross;
  if (IsInVolRange(last_v_cross, vol_range)) {
    CalcLightboxPlaneCenter(res_lightbox);
  } else {
    const glm::vec3 last_fit_cross =
        FitVolRange(cross_pt, last_v_cross, vol_range);
    TranslateLightbox(res_lightbox, last_fit_cross - last_v_cross, vol_center);
  }
}

void MPRModule::CalcLightboxPlaneCenter(LightboxResource* res_lightbox) {
  /* // pseudo code //
          get first plane center
          for each LightboxResource
                  set plane center
                  calc next plane center
  */
  const glm::vec3& first_plane_center = res_lightbox->GetPlaneCenter(0);
  glm::vec3 plane_center = first_plane_center;
  const glm::vec3 direction = res_lightbox->GetLighboxDirection();
  const float& interval = res_lightbox->GetLightboxInterval();
  for (int id = 0; id < res_lightbox->GetLightboxCount(); ++id) {
    res_lightbox->InitLightboxData(id, plane_center);
    plane_center += interval * direction;
  }
}

void MPRModule::CalcLightboxInterval(LightboxResource* res_lightbox,
                                     const float& real_interval,
                                     const float& pixel_spacing,
                                     const float& slice_spacing) {
  const glm::vec3 direction = res_lightbox->GetLighboxDirection();
  glm::vec3 up_vector = glm::normalize(direction);
  glm::vec3 spacing(pixel_spacing, pixel_spacing, slice_spacing);
  float plane_up_spacing = glm::length(up_vector * spacing);
  res_lightbox->SetLightboxInterval(real_interval / plane_up_spacing);
}

void MPRModule::CalcLightboxThickness(LightboxResource* res_lightbox,
                                      const float& real_thickness,
                                      const float& pixel_spacing,
                                      const float& slice_spacing) {
  const glm::vec3 direction = res_lightbox->GetLighboxDirection();
  glm::vec3 up_vector = glm::normalize(direction);
  glm::vec3 spacing(pixel_spacing, pixel_spacing, slice_spacing);
  float plane_up_spacing = glm::length(up_vector * spacing);
  res_lightbox->SetLightboxThickness(real_thickness / plane_up_spacing);
}

glm::vec3 MPRModule::FitVolRange(const glm::vec3& source, const glm::vec3& dest,
                                 const glm::vec3& vol_range) {
  const glm::vec3 direction = dest - source;

  glm::vec3 plane_point;
  for (int vol_id = 0; vol_id < VolPlaneType::VOL_PLANE_END; ++vol_id) {
    switch (vol_id) {
      case VolPlaneType::X_MAX:
      case VolPlaneType::Y_MAX:
      case VolPlaneType::Z_MAX:
        plane_point = vol_range;
        break;
      case VolPlaneType::X_MIN:
      case VolPlaneType::Y_MIN:
      case VolPlaneType::Z_MIN:
        plane_point = glm::vec3(0.0f);
        break;
    }

    const float dot_up_direction = glm::dot(kAxis[vol_id], direction);
    if (std::fabsf(dot_up_direction) <= kEpsEqual) continue;

    const glm::vec3 inv_w = plane_point - source;
    const float meet_position =
        glm::dot(kAxis[vol_id], inv_w) / dot_up_direction;
    if (W3::isInRange(meet_position, 0.0f, 1.0f)) {
      const glm::vec3 ret_vec = source + meet_position * direction;
      if (IsInVolRange(ret_vec, vol_range)) {
        return ret_vec;
      }
    }
  }

  return source;
}

bool MPRModule::IsInVolRange(const glm::vec3& pos, const glm::vec3& vol_range) {
  if (W3::isInRange(pos.x, 0.0f, vol_range.x) &&
      W3::isInRange(pos.y, 0.0f, vol_range.y) &&
      W3::isInRange(pos.z, 0.0f, vol_range.z)) {
    return true;
  }
  return false;
}
