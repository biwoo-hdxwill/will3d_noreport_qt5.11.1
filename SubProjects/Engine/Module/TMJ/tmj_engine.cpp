#include "tmj_engine.h"
#if defined(__APPLE__)
#include </usr/local/Cellar/llvm/5.0.0/lib/clang/5.0.0/include/omp.h>
#else
#include <omp.h>
#endif
#include <qpolygon.h>
#include <QPainterPath>
#include <iostream>

#include "../../Common/Common/W3Enum.h"
#include "../../Common/Common/W3Logger.h"
#include "../../Common/GLfunctions/gl_helper.h"
#ifndef WILL3D_VIEWER
#include "../../Core/W3ProjectIO/project_io_tmj.h"
#endif
#include "../../Resource/ResContainer/resource_container.h"
#include "../../Resource/Resource/W3Image3D.h"
#include "../../Resource/Resource/tmj_resource.h"

namespace {
const float kEpsEqual = std::numeric_limits<float>::epsilon();
using glm::mat4;
using glm::vec3;
using glm::vec4;
}  // end of namespace

TMJengine::TMJengine() {
  tmj_resource_.reset(new TMJresource);
  ResourceContainer::GetInstance()->SetTMJResource(tmj_resource_);
}

TMJengine::~TMJengine() {}

void TMJengine::Initialize(const CW3Image3D& volume) {
  if (initialized_) {
    return common::Logger::instance()->Print(
        common::LogType::ERR, "TMJengine::Initialize: already initialized.");
  }
  InitSliceLocation(volume);

  initialized_ = true;
}

void TMJengine::InitSliceLocation(const CW3Image3D& volume) {
  const auto& slice_loc = volume.getSliceLoc();

#if 0
  roi_vol_.top = (float)(slice_loc.nose * 2 - (slice_loc.teeth));
  roi_vol_.top = std::max(roi_vol_.top, 0.0f);
  roi_vol_.bottom = slice_loc.maxilla;

  if (roi_vol_.top == roi_vol_.bottom ||
	  roi_vol_.top > roi_vol_.bottom) {
	roi_vol_.top = 0; 
	roi_vol_.bottom = volume.depth() - 1;
  }
#else
  roi_vol_.top = 0;
  roi_vol_.bottom = static_cast<int>(static_cast<float>(volume.depth() - 1) * 0.5f);
#endif
  roi_vol_.slice = (roi_vol_.top + roi_vol_.bottom) * 0.5f;
}

bool TMJengine::SetLateralParam(const TMJLateralID& id, const float& value) {
  TMJDirectionType type = tmj_resource_->GetDirectionType(id);
  const auto& lateral = tmj_resource_->lateral(type);

  if (&lateral == nullptr) return false;

  float prev_value;
  switch (id) {
    case LEFT_INTERVAL:
    case RIGHT_INTERVAL:
      prev_value = lateral.param().interval;
      break;
    case LEFT_THICKNESS:
    case RIGHT_THICKNESS:
      prev_value = lateral.param().thickness;
      break;
    default:
      assert(false);
      break;
  }

  if (value == prev_value) return false;

  tmj_resource_->SetLateralParam(id, value);
  return true;
}

void TMJengine::SetTMJRectParam(const TMJRectID& id, const float& value) {
  tmj_resource_->SetTMJRectParam(id, value);
}

void TMJengine::SetTMJROISlice(const float& slice_pos) {
  roi_vol_.slice = slice_pos;
}

void TMJengine::SetAxialCenterPosition(const glm::vec3& pt_center) {
  tmj_resource_->set_axial_position(pt_center);
}

void TMJengine::SetTMJROITop(const float& top_pos) {
  roi_vol_.top = top_pos;
  tmj_resource_->set_height((int)(roi_vol_.bottom - roi_vol_.top));
}

void TMJengine::SetTMJROIBottom(const float& bottom_pos) {
  roi_vol_.bottom = bottom_pos;
  tmj_resource_->set_height((int)(roi_vol_.bottom - roi_vol_.top));
}

void TMJengine::SetLatertalCount(const TMJDirectionType& type,
                                 const int& value) {
  tmj_resource_->SetLateralCount(type, value);
}

void TMJengine::SetLateralSelectedID(const TMJDirectionType& type,
                                     const int& lateral_id) {
  tmj_resource_->SetSelectedLateralID(type, lateral_id);
}

void TMJengine::SetBackVector(const glm::vec3& back_vector) {
  tmj_resource_->set_back_vector(back_vector);
}

void TMJengine::ReadyFrontalResource(const TMJDirectionType& type) {
  if (&tmj_resource_->frontal(type) == nullptr) {
    tmj_resource_->CreateFrontalResource(type);
  }
}

void TMJengine::ReadyLateralResource(const TMJDirectionType& type) {
  if (&tmj_resource_->lateral(type) == nullptr) {
    tmj_resource_->CreateLateralResource(type);
  }
}
bool TMJengine::DeleteFrontalResource(const TMJDirectionType& type) {
  return tmj_resource_->DeleteFrontalResource(type);
}
bool TMJengine::DeleteLateralResource(const TMJDirectionType& type) {
  return tmj_resource_->DeleteLateralResource(type);
}

void TMJengine::CutTMJ(const TMJDirectionType& direction_type,
                       const QPolygonF& cut_area, const bool& is_inside,
                       const std::function<void(const std::vector<glm::vec3>&,
                                                std::vector<QPointF>&)>&
                           FrontalViewMapVolToScene) {
  clock_t start_time = clock();

  if (cut_params_[direction_type].mask == nullptr)
    CreateTMJMask(direction_type);

  const int mask_width =
      cut_params_[direction_type].mask->width();  // frontal width
  const int mask_height =
      cut_params_[direction_type].mask->height();  // lateral width
  const int mask_depth = cut_params_[direction_type].mask->depth();  // height
  unsigned short** data = cut_params_[direction_type].mask->getData();

  const auto& vol = ResourceContainer::GetInstance()->GetMainVolume();

  glm::vec3 vol_range = glm::vec3(static_cast<float>(vol.width()),
                                  static_cast<float>(vol.height()),
                                  static_cast<float>(vol.depth()));
  glm::vec3 vol_center = vol_range * 0.5f - 0.5f;

  float spacingz = vol.sliceSpacing() / vol.pixelSpacing();

  const float f_mask_width = static_cast<float>(mask_width);
  const float f_mask_height = static_cast<float>(mask_height);
  const float f_mask_depth = static_cast<float>(mask_depth);
  glm::mat4 mask_scale =
      glm::scale(glm::vec3(f_mask_width, f_mask_height, f_mask_depth));

  glm::vec3 pt_center_world = GLhelper::MapVolToWorldGL(
      tmj_resource_->GetTMJRectCenter(direction_type), vol_center, spacingz);
  glm::mat4 rotate = tmj_resource_->GetTMJRoateMatrix(direction_type);

  glm::mat4 mask_to_world = glm::translate(pt_center_world) * rotate *
                            mask_scale * glm::scale(vec3(2.0f)) *
                            glm::translate(-glm::vec3(0.5f)) *
                            glm::inverse(mask_scale);

  bool pop_front_and_shift = false;
  if (cut_params_[direction_type].curr_step == 15) {
    pop_front_and_shift = true;
    cut_params_[direction_type].is_over_stack = true;
  } else if (cut_params_[direction_type].curr_step < 15)
    cut_params_[direction_type].curr_step++;

  cut_params_[direction_type].stack_index =
      cut_params_[direction_type].curr_step;
  int prev_step = cut_params_[direction_type].curr_step - 1;

  const int num_thread = omp_get_max_threads();
  omp_set_num_threads(num_thread);

  std::vector<glm::vec3> vol_positions;
  vol_positions.resize(mask_depth * mask_width);

#pragma omp parallel for
  for (int k = 0; k < mask_depth; ++k) {
    unsigned short* mask_slice = data[k];
    glm::vec3* buf_vol_pos = &vol_positions[k * mask_width];
    int z_idx = mask_width * mask_height * k;
    for (int i = 0; i < mask_width; ++i) {
      const glm::vec4 pt_mask =
          glm::vec4(static_cast<float>(i), 0.0f, static_cast<float>(k), 1.0f);
      const glm::vec4 pt_world = mask_to_world * pt_mask;
      *buf_vol_pos++ =
          GLhelper::MapWorldGLtoVol(glm::vec3(pt_world), vol_center, spacingz);
    }
  }
  std::vector<QPointF> scene_positions;
  FrontalViewMapVolToScene(vol_positions, scene_positions);

  std::vector<unsigned short> mask_copy(mask_width * mask_depth, 0);
#pragma omp parallel for
  for (int k = 0; k < mask_depth; ++k) {
    QPointF* buf_scene_positions = &scene_positions[mask_width * k];
    unsigned short* buf_mask_copy = &mask_copy[k * mask_width];
    unsigned short* mask_slice = data[k];
    for (int i = 0; i < mask_width; ++i) {
      const QPointF& scene_pos = *buf_scene_positions++;
      bool is_contain = cut_area.containsPoint(scene_pos, Qt::WindingFill);
      is_contain = (is_inside) ? is_contain : !is_contain;

      unsigned short mask_value = 0;

      if (pop_front_and_shift) {
        mask_value = *mask_slice >> 1;
      }

      if (is_contain) {
        mask_value |= 0x0001 << (cut_params_[direction_type].curr_step);
      } else {
        if (prev_step > -1 && *mask_slice & 0x0001 << prev_step)
          mask_value |= 0x0001 << cut_params_[direction_type].curr_step;
        else
          mask_value &= 0x0000 << cut_params_[direction_type].curr_step;
      }
      *buf_mask_copy++ = mask_value;
      *mask_slice++;
    }
  }

#pragma omp parallel for
  for (int j = 0; j < mask_height; j++) {
    for (int k = 0; k < mask_depth; k++) {
      memcpy(&data[k][j * mask_width], &mask_copy[k * mask_width],
             sizeof(unsigned short) * mask_width);
    }
  }

  tmj_resource_->set_tmj_mask(direction_type, cut_params_[direction_type].mask);

#if DEVELOP_MODE
  clock_t end_time = clock();
  float elapsed_time = static_cast<float>(end_time - start_time);
  std::cout << "3D unsigned short mask elapsed_time :" << elapsed_time
            << std::endl;
#endif
}

bool TMJengine::ResetCutTMJ(const TMJDirectionType& direction_type) {
  return ResetCutParams(direction_type);
}

void TMJengine::UndoCutTMJ(const TMJDirectionType& direction_type) {
  int curr_step = cut_params_[direction_type].curr_step;
  if ((curr_step == 0 && !cut_params_[direction_type].is_over_stack) ||
      curr_step > 0)
    cut_params_[direction_type].curr_step--;
}

void TMJengine::RedoCutTMJ(const TMJDirectionType& direction_type) {
  int curr_step = cut_params_[direction_type].curr_step;
  if (curr_step < 15 && curr_step < cut_params_[direction_type].stack_index)
    cut_params_[direction_type].curr_step++;
}

bool TMJengine::SetTMJRectWidth(const TMJDirectionType& type,
                                const float& width) {
  const auto& frontal = this->GetFrontal(type);
  if (std::abs(frontal.Width() - width) < kEpsEqual) return false;

  TMJRectID rect_id = (type == TMJDirectionType::TMJ_LEFT) ? TMJRectID::LEFT_W
                                                           : TMJRectID::RIGHT_W;
  tmj_resource_->SetTMJRectParam(rect_id, width);

  return true;
}

bool TMJengine::SetTMJRectHeight(const TMJDirectionType& type,
                                 const float& height) {
  const auto& lateral = this->GetLateral(type);

  if (std::abs(lateral.Width() - height) < kEpsEqual) return false;

  TMJRectID rect_id = (type == TMJDirectionType::TMJ_LEFT) ? TMJRectID::LEFT_H
                                                           : TMJRectID::RIGHT_H;
  tmj_resource_->SetTMJRectParam(rect_id, height);

  return true;
}

bool TMJengine::SetTMJRectCenter(const TMJDirectionType& type,
                                 const glm::vec3& pt_center) {
  if (glm::length(tmj_resource_->GetTMJRectCenter(type) - pt_center) < 0.0001f)
    return false;

  tmj_resource_->SetTMJRectCenter(type, pt_center);
  return true;
}

bool TMJengine::SetLateralPositionInfo(
    const TMJDirectionType& type, const std::map<int, glm::vec3>& positions,
    const glm::vec3& up_vector) {
  const auto& lateral = this->GetLateral(type);
  auto prev_center_positions = lateral.center_positions();

  tmj_resource_->SetLateralPositionInfo(type, positions, up_vector);

  const auto& curr_center_positions = lateral.center_positions();
  bool is_changed = false;
  if (prev_center_positions.size() != curr_center_positions.size())
    is_changed = true;
  else {
    float diff = 0.0f;
    for (int i = 0; i < prev_center_positions.size(); i++) {
      diff += glm::length(prev_center_positions[i] - curr_center_positions[i]);
    }
    if (diff > kEpsEqual) {
      is_changed = true;
    }
  }
  if (!is_changed) return false;
  return true;
}
bool TMJengine::SetFrontalPositionInfo(const TMJDirectionType& type,
                                       const glm::vec3& position,
                                       const glm::vec3& up_vector) {
  const auto& frontal = this->GetFrontal(type);
  const auto& prev_center_position = frontal.center_position();

  bool is_changed = false;
  float diff = glm::length(prev_center_position - position);

  if (diff > kEpsEqual) {
    is_changed = true;
  }

  if (!is_changed) return false;

  tmj_resource_->SetFrontalPositionInfo(type, position, up_vector);

  return true;
}
const TMJfrontalResource& TMJengine::GetFrontal(
    const TMJDirectionType& type) const {
  const auto& frontal = tmj_resource_->frontal(type);
  if (&frontal == nullptr) {
    common::Logger::instance()->PrintAndAssert(common::LogType::ERR,
                                               "TMJengine::frontal: nullptr.");
  }
  return frontal;
}
const TMJlateralResource& TMJengine::GetLateral(
    const TMJDirectionType& type) const {
  const auto& lateral = tmj_resource_->lateral(type);
  if (&lateral == nullptr) {
    common::Logger::instance()->PrintAndAssert(common::LogType::ERR,
                                               "TMJengine::lateral: nullptr.");
  }
  return lateral;
}
float TMJengine::GetBasePixelSpacing() const {
  const CW3Image3D& vol = ResourceContainer::GetInstance()->GetMainVolume();
  return std::min(vol.pixelSpacing(), vol.sliceSpacing());
}

const int& TMJengine::GetLateralSelectedID(const TMJDirectionType& type) const {
  return tmj_resource_->GetSelectedLateralID(type);
}

void TMJengine::CreateTMJMask(const TMJDirectionType& type) {
  float width, height, depth;
  GetTMJRectSize(type, &width, &height);
  depth = tmj_resource_->height();
  ResetCutParams(type);

  int i_width = static_cast<unsigned int>(width);
  int i_height = static_cast<unsigned int>(height);
  int i_depth = static_cast<unsigned int>(depth);

  cut_params_[type].mask.reset(new CW3Image3D(i_width, i_height, i_depth));

  unsigned short** data = cut_params_[type].mask->getData();
  for (int i = 0; i < i_depth; i++)
    std::memset(data[i], 0, sizeof(unsigned short) * i_width * i_height);
}

bool TMJengine::ResetCutParams(const TMJDirectionType& type) {
  cut_params_[type].curr_step = -1;
  cut_params_[type].stack_index = -1;
  cut_params_[type].is_over_stack = false;

  if (cut_params_[type].mask) {
    cut_params_[type].mask.reset();
    return true;
  } else {
    return false;
  }
}
#ifndef WILL3D_VIEWER
void TMJengine::ExportProject(ProjectIOTMJ& out) {
  out.SaveTMJROI(roi_vol_.bottom, roi_vol_.top, roi_vol_.slice);
  out.SaveReorientation(reorien_);

  for (int id = 0; id < TMJDirectionType::TMJ_TYPE_END; ++id) {
    const TMJDirectionType type = static_cast<TMJDirectionType>(id);
    if (tmj_resource_->IsValidResource(type)) {
      out.SaveLateralUp(type, tmj_resource_->lateral(type).up_vector());
      out.SaveRectCenter(type, tmj_resource_->GetTMJRectCenter(type));
    }
  }
}

void TMJengine::ImportProject(ProjectIOTMJ& in) {
  in.LoadTMJROI(roi_vol_.bottom, roi_vol_.top, roi_vol_.slice);
  tmj_resource_->set_height((int)(roi_vol_.bottom - roi_vol_.top));

  in.LoadReorientation(reorien_);

  for (int id = 0; id < TMJDirectionType::TMJ_TYPE_END; ++id) {
    const TMJDirectionType type = static_cast<TMJDirectionType>(id);
    if (in.IsValidTMJ(type)) {
      glm::vec3 rect_center;
      in.LoadRectCenter(type, rect_center);
      glm::vec3 lateral_up;
      in.LoadLateralUp(type, lateral_up);

      tmj_resource_->ImportProject(type, rect_center, lateral_up);
    }
  }
}
#endif
const glm::vec3& TMJengine::GetTMJBackVector() const {
  if (!tmj_resource_) {
    common::Logger::instance()->PrintAndAssert(
        common::LogType::ERR, "TMJengine::GetTMJBackVector: nullptr.");
  }
  return tmj_resource_->back_vector();
}
const glm::vec3& TMJengine::GetRectCenter(const TMJDirectionType& type) const {
  if (!tmj_resource_) {
    common::Logger::instance()->PrintAndAssert(
        common::LogType::ERR, "TMJengine::GetRectCenter: nullptr.");
  }
  return tmj_resource_->GetTMJRectCenter(type);
}
const glm::vec3& TMJengine::GetLateralUpVector(
    const TMJDirectionType& type) const {
  if (!tmj_resource_) {
    common::Logger::instance()->PrintAndAssert(
        common::LogType::ERR, "TMJengine::GetLateralUpVector: nullptr.");
  }

  const auto& lateral = tmj_resource_->lateral(type);
  if (&lateral == nullptr) {
    common::Logger::instance()->PrintAndAssert(
        common::LogType::ERR, "TMJengine::GetLateralUpVector: nullptr.");
  }
  return lateral.up_vector();
}
bool TMJengine::GetTMJRectSize(const TMJDirectionType& type, float* width,
                               float* height) const {
  if (!tmj_resource_) {
    common::Logger::instance()->PrintAndAssert(
        common::LogType::ERR,
        "TMJengine::GetTMJRectSize: tmj_resource is nullptr.");
    return false;
  }
  const auto& frontal = tmj_resource_->frontal(type);
  if (&frontal == nullptr) {
    return false;
  }
  const auto& lateral = tmj_resource_->lateral(type);
  if (&lateral == nullptr) {
    return false;
  }
  *width = frontal.Width();
  *height = lateral.Width();
  return true;
}

bool TMJengine::IsValidTMJ(const TMJDirectionType& type) const {
  return tmj_resource_->IsValidResource(type);
}
