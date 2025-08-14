#include "mpr_engine.h"

#include <QPointF>

#include "../../Common/Common/W3Logger.h"
#include "../../Common/Common/W3Math.h"
#include "../../Resource/ResContainer/resource_container.h"
#include "../../Resource/Resource/W3Image3D.h"
#include "../../Resource/Resource/lightbox_resource.h"

#include "mpr_module.h"

MPREngine::MPREngine() {}
MPREngine::~MPREngine() {}

void MPREngine::Initialize(const glm::vec3& mpr_cross_point) {
  mpr_cross_point_ = mpr_cross_point;

  const CW3Image3D& volume = ResourceContainer::GetInstance()->GetMainVolume();
  if (&volume == nullptr) {
    common::Logger::instance()->Print(
        common::LogType::ERR, "MPREngine::Initialize : volume is empty");
    assert(false);
    return;
  }

  const unsigned int width = volume.width();
  const unsigned int height = volume.height();
  const unsigned int depth = volume.depth();
  const float z_spacing = volume.sliceSpacing() / volume.pixelSpacing();
  if (z_spacing >= 1.0f) {
    mpr_vol_range_ = glm::vec3(width, height, depth * z_spacing);
    mpr_vol_center_ = glm::vec3(0.5f * (width - 1), 0.5f * (height - 1),
                                0.5f * (depth - 1) * z_spacing);
  } else {
    mpr_vol_range_ = glm::vec3(width / z_spacing, height / z_spacing, depth);
    mpr_vol_center_ =
        glm::vec3(0.5f * (width - 1) / z_spacing,
                  0.5f * (height - 1) / z_spacing, 0.5f * (depth - 1));
  }
}

void MPREngine::InitLightboxResource(
    const glm::vec3& plane_center,
    const lightbox_resource::LightboxParams& lightbox_params,
    const lightbox_resource::PlaneParams& plane_params) {
  res_lightbox_.reset(
      new LightboxResource(plane_center, lightbox_params, plane_params));
  ResourceContainer::GetInstance()->SetLightboxResource(res_lightbox_);

  const CW3Image3D& volume = ResourceContainer::GetInstance()->GetMainVolume();
  MPRModule::InitLightboxPlanes(res_lightbox(), mpr_vol_center_, mpr_vol_range_,
                                volume.pixelSpacing(), volume.sliceSpacing(),
                                mpr_cross_point_);
  mpr_cross_point_ = res_lightbox_->first_cross_pt();
}

/**********************************************************************************************
Sets lightbox to maximize mode.
@param	lightbox_id	Identifier for the lightbox.
@return	True if res_lightbox is on, false if res_lightbox is off.
 **********************************************************************************************/
bool MPREngine::SetLightboxToMaximizeMode(const int& lightbox_id) {
  int maximized_lightbox_id = -1;
  if (res_lightbox()->IsMaximzeMode(
          maximized_lightbox_id)) {  // to default mode
    int prev_row, prev_col, prev_lightbox_id;
    res_lightbox()->LoadMaximizeParams(prev_row, prev_col, prev_lightbox_id);
    ChangeLightboxCount(prev_row, prev_col);
    float tranlate_delta = static_cast<float>(-prev_lightbox_id);
    TranslateLightbox(tranlate_delta);
    return false;
  } else {  // to maximize mode
    res_lightbox()->SaveMaximizeParams(lightbox_id);
    ChangeLightboxCount(1, 1);
    float tranlate_delta = static_cast<float>(lightbox_id);
    TranslateLightbox(tranlate_delta);
    return true;
  }
}

void MPREngine::ChangeLightboxCount(const int& lightbox_cnt_row,
                                    const int& lightbox_cnt_col) {
  res_lightbox()->ChangeLightboxCount(lightbox_cnt_row, lightbox_cnt_col);

  const CW3Image3D& volume = ResourceContainer::GetInstance()->GetMainVolume();
  MPRModule::CalcLightboxPositions(res_lightbox(), mpr_cross_point_,
                                   mpr_vol_center_, mpr_vol_range_);
  mpr_cross_point_ = res_lightbox_->first_cross_pt();
}

void MPREngine::GetLightboxCount(int& lightbox_cnt_row, int& lightbox_cnt_col) {
  lightbox_cnt_row = res_lightbox()->GetLightboxCountRow();
  lightbox_cnt_col = res_lightbox()->GetLightboxCountCol();
}

void MPREngine::GetLightboxSliderPositions(std::vector<int>& slider_positions) {
  MPRModule::GetLightboxSliderPosition(res_lightbox(), mpr_vol_center_,
                                       slider_positions);
}

void MPREngine::GetProfileDataInLightboxPlane(const int& lightbox_id,
                                              const QPointF& start_pt_plane,
                                              const QPointF& end_pt_plane,
                                              std::vector<short>& data) {
  std::vector<QPointF> plane_points;
  W3::GenerateSequencialPlanePointsInLine(start_pt_plane, end_pt_plane,
                                          plane_points);

  std::vector<glm::vec3> vol_points;
  MPRModule::MapLightboxPlaneToVol(res_lightbox(), lightbox_id, plane_points,
                                   vol_points);
  ResourceContainer::GetInstance()->GetMainVolume().GetVolumeHU(vol_points,
                                                                data);
}

void MPREngine::GetROIDataInLightboxPlane(const int& lightbox_id,
                                          const QPointF& start_pt_plane,
                                          const QPointF& end_pt_plane,
                                          std::vector<short>& data) {
  std::vector<QPointF> plane_points;
  W3::GenerateSequencialPlanePointsInRect(start_pt_plane, end_pt_plane,
                                          plane_points);

  std::vector<glm::vec3> vol_points;
  MPRModule::MapLightboxPlaneToVol(res_lightbox(), lightbox_id, plane_points,
                                   vol_points);
  ResourceContainer::GetInstance()->GetMainVolume().GetVolumeHU(vol_points,
                                                                data);
}

void MPREngine::TranslateLightbox(const int& lightbox_id,
                                  const int& slider_value) {
  MPRModule::TranslateLightboxPlanes(res_lightbox(), mpr_vol_center_,
                                     mpr_vol_range_, lightbox_id, slider_value);
  mpr_cross_point_ = res_lightbox_->first_cross_pt();
}

void MPREngine::TranslateLightbox(const float& translate_delta) {
  MPRModule::TranslateLightboxPlanes(res_lightbox(), mpr_vol_center_,
                                     mpr_vol_range_, translate_delta);
  mpr_cross_point_ = res_lightbox_->first_cross_pt();
}

glm::vec4 MPREngine::MapLightboxPlaneToVol(const int& lightbox_id,
                                           const QPointF& pt_lightbox_plane) {
  glm::vec3 vol_pt = MPRModule::MapLightboxPlaneToVol(
      res_lightbox(), lightbox_id, pt_lightbox_plane);
  return ResourceContainer::GetInstance()->GetMainVolume().GetVolumeInfo(
      vol_pt);
}

const LightboxViewType MPREngine::GetLightboxViewType() const {
  return res_lightbox()->GetLightboxViewType();
}

const int MPREngine::GetLightboxAvailableDepth() const {
  return res_lightbox()->GetPlaneAvailableDepth();
}

void MPREngine::SetLightboxInterval(const float& real_interval) {
  const CW3Image3D& volume = ResourceContainer::GetInstance()->GetMainVolume();
  MPRModule::SetLightboxInterval(
      res_lightbox(), real_interval, volume.pixelSpacing(),
      volume.sliceSpacing(), mpr_cross_point_, mpr_vol_center_, mpr_vol_range_);
  mpr_cross_point_ = res_lightbox_->first_cross_pt();
}

void MPREngine::SetLightboxThickness(const float& real_thickness) {
  const CW3Image3D& volume = ResourceContainer::GetInstance()->GetMainVolume();
  MPRModule::SetLightboxThickness(res_lightbox(), real_thickness,
                                  volume.pixelSpacing(), volume.sliceSpacing());
}

bool MPREngine::IsLightboxAvailableMaximize() const {
  int maximized_lightbox_id = -1;
  if (!res_lightbox()->IsMaximzeMode(maximized_lightbox_id)) {
    if (res_lightbox()->GetLightboxCount() < 2) return false;
  }
  return true;
}
