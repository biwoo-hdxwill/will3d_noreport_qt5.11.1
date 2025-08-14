#include "project_io_tmj.h"
#include <H5Cpp.h>

#include "../../Common/Common/W3Logger.h"
#include "../../Common/Common/define_tmj.h"
#include "io_functions.h"
#include "project_io_view.h"
#include "project_path_info.h"

using namespace H5;
using namespace project;

namespace {
H5::Group GetTMJGroup(H5File* file, const ProjectIOTMJ::ViewType& view_type,
                      int view_id = 0) {
  switch (view_type) {
    case ProjectIOTMJ::ViewType::AXIAL:
      return file->openGroup(group::kViewTMJAxial);
    case ProjectIOTMJ::ViewType::FRONTAL_LEFT: {
      Group view_grp = file->openGroup(group::kViewTMJFrontalLeft);
      return view_grp.openGroup(std::to_string(view_id));
    }
    case ProjectIOTMJ::ViewType::FRONTAL_RIGHT: {
      Group view_grp = file->openGroup(group::kViewTMJFrontalRight);
      return view_grp.openGroup(std::to_string(view_id));
    }
    case ProjectIOTMJ::ViewType::LATERAL_LEFT: {
      Group view_grp = file->openGroup(group::kViewTMJLateralLeft);
      return view_grp.openGroup(std::to_string(view_id));
    }
    case ProjectIOTMJ::ViewType::LATERAL_RIGHT: {
      Group view_grp = file->openGroup(group::kViewTMJLateralRight);
      return view_grp.openGroup(std::to_string(view_id));
    }
    case ProjectIOTMJ::ViewType::THREE_D_LEFT:
      return file->openGroup(group::kViewTMJ3DLeft);
    case ProjectIOTMJ::ViewType::THREE_D_RIGHT:
      return file->openGroup(group::kViewTMJ3DRight);
    default:
      common::Logger::instance()->Print(common::LogType::ERR,
                                        "Project IO TMJ : invalid TMJ Type");
      return Group();
  }
}
}  // end of namespace

ProjectIOTMJ::ProjectIOTMJ(const project::Purpose& purpose,
                           const std::shared_ptr<H5::H5File>& file)
    : file_(file) {
  if (purpose == project::Purpose::SAVE) {
    file_->createGroup(group::kTabTMJ);
    file_->createGroup(group::kViewTMJAxial);
    Group view_grp = file_->createGroup(group::kViewTMJFrontalLeft);
    for (int cnt = 0; cnt < common::kMaxFrontalCount; ++cnt) {
      view_grp.createGroup(std::to_string(cnt));
    }
    view_grp = file_->createGroup(group::kViewTMJFrontalRight);
    for (int cnt = 0; cnt < common::kMaxFrontalCount; ++cnt) {
      view_grp.createGroup(std::to_string(cnt));
    }
    view_grp = file_->createGroup(group::kViewTMJLateralLeft);
    for (int cnt = 0; cnt < common::kMaxLateralCount; ++cnt) {
      view_grp.createGroup(std::to_string(cnt));
    }
    view_grp = file_->createGroup(group::kViewTMJLateralRight);
    for (int cnt = 0; cnt < common::kMaxLateralCount; ++cnt) {
      view_grp.createGroup(std::to_string(cnt));
    }
    file_->createGroup(group::kViewTMJ3DLeft);
    file_->createGroup(group::kViewTMJ3DRight);
  }
}

ProjectIOTMJ::~ProjectIOTMJ() {}

void ProjectIOTMJ::InitTMJTab() {
  Group tab_grp = file_->openGroup(group::kTabTMJ);
  IOFunctions::WriteBool(tab_grp, ds::kIsTabInit, true);
  tab_grp.close();
}

bool ProjectIOTMJ::IsInit() {
  bool exists = false;
  Group tab_grp = file_->openGroup(group::kTabTMJ);
  if (tab_grp.exists(ds::kIsTabInit)) exists = true;
  tab_grp.close();
  return exists;
}

void ProjectIOTMJ::InitializeView(ProjectIOTMJ::ViewType view_type,
                                  const int& view_id) {
  curr_view_type_ = view_type;
  view_io_.reset(new ProjectIOView(
      file_, GetTMJGroup(file_.get(), curr_view_type_, view_id)));
}

ProjectIOView& ProjectIOTMJ::GetViewIO() { return *(view_io_.get()); }

void ProjectIOTMJ::SaveTMJRect(const float& l_width, const float& l_height,
                               const float& r_width, const float& r_height) {
  project::TMJRect rect;
  rect.left_width = l_width;
  rect.left_height = l_height;
  rect.right_width = r_width;
  rect.right_height = r_height;

  CompType tmj_rect_type(sizeof(project::TMJRect));
  tmj_rect_type.insertMember(ds_member::kLeftWidth,
                             HOFFSET(project::TMJRect, left_width),
                             PredType::NATIVE_FLOAT);
  tmj_rect_type.insertMember(ds_member::kLeftHeight,
                             HOFFSET(project::TMJRect, left_height),
                             PredType::NATIVE_FLOAT);
  tmj_rect_type.insertMember(ds_member::kRightWidth,
                             HOFFSET(project::TMJRect, right_width),
                             PredType::NATIVE_FLOAT);
  tmj_rect_type.insertMember(ds_member::kRightHeight,
                             HOFFSET(project::TMJRect, right_height),
                             PredType::NATIVE_FLOAT);

  Group tmj_grp = file_->openGroup(group::kTabTMJ);
  DataSet ds = tmj_grp.createDataSet(ds::kTMJRect, tmj_rect_type,
                                     project::io::kDSScalar);
  ds.write(&rect, tmj_rect_type);
  ds.close();
  tmj_grp.close();
}

void ProjectIOTMJ::LoadTMJRect(float& l_width, float& l_height, float& r_width,
                               float& r_height) {
  CompType tmj_rect_type(sizeof(project::TMJRect));
  tmj_rect_type.insertMember(ds_member::kLeftWidth,
                             HOFFSET(project::TMJRect, left_width),
                             PredType::NATIVE_FLOAT);
  tmj_rect_type.insertMember(ds_member::kLeftHeight,
                             HOFFSET(project::TMJRect, left_height),
                             PredType::NATIVE_FLOAT);
  tmj_rect_type.insertMember(ds_member::kRightWidth,
                             HOFFSET(project::TMJRect, right_width),
                             PredType::NATIVE_FLOAT);
  tmj_rect_type.insertMember(ds_member::kRightHeight,
                             HOFFSET(project::TMJRect, right_height),
                             PredType::NATIVE_FLOAT);

  project::TMJRect rect;
  Group tmj_grp = file_->openGroup(group::kTabTMJ);
  DataSet ds = tmj_grp.openDataSet(ds::kTMJRect);
  ds.read(&rect, tmj_rect_type);
  ds.close();
  tmj_grp.close();

  l_width = rect.left_width;
  l_height = rect.left_height;
  r_width = rect.right_width;
  r_height = rect.right_height;
}

void ProjectIOTMJ::SaveLateralParams(const float& l_interval,
                                     const float& l_thickness,
                                     const float& r_interval,
                                     const float& r_thickness) {
  project::TMJLateralParms params;
  params.left_interval = l_interval;
  params.left_thickness = l_thickness;
  params.right_interval = r_interval;
  params.right_thickness = r_thickness;

  CompType tmj_lateral_type(sizeof(project::TMJLateralParms));
  tmj_lateral_type.insertMember(
      ds_member::kLeftWidth, HOFFSET(project::TMJLateralParms, left_interval),
      PredType::NATIVE_FLOAT);
  tmj_lateral_type.insertMember(
      ds_member::kLeftHeight, HOFFSET(project::TMJLateralParms, left_thickness),
      PredType::NATIVE_FLOAT);
  tmj_lateral_type.insertMember(
      ds_member::kRightWidth, HOFFSET(project::TMJLateralParms, right_interval),
      PredType::NATIVE_FLOAT);
  tmj_lateral_type.insertMember(
      ds_member::kRightHeight,
      HOFFSET(project::TMJLateralParms, right_thickness),
      PredType::NATIVE_FLOAT);

  Group tmj_grp = file_->openGroup(group::kTabTMJ);
  DataSet ds = tmj_grp.createDataSet(ds::kTMJLateralParams, tmj_lateral_type,
                                     project::io::kDSScalar);
  ds.write(&params, tmj_lateral_type);
  ds.close();
  tmj_grp.close();
}

void ProjectIOTMJ::LoadLateralParams(float& l_interval, float& l_thickness,
                                     float& r_interval, float& r_thickness) {
  CompType tmj_lateral_type(sizeof(project::TMJLateralParms));
  tmj_lateral_type.insertMember(
      ds_member::kLeftWidth, HOFFSET(project::TMJLateralParms, left_interval),
      PredType::NATIVE_FLOAT);
  tmj_lateral_type.insertMember(
      ds_member::kLeftHeight, HOFFSET(project::TMJLateralParms, left_thickness),
      PredType::NATIVE_FLOAT);
  tmj_lateral_type.insertMember(
      ds_member::kRightWidth, HOFFSET(project::TMJLateralParms, right_interval),
      PredType::NATIVE_FLOAT);
  tmj_lateral_type.insertMember(
      ds_member::kRightHeight,
      HOFFSET(project::TMJLateralParms, right_thickness),
      PredType::NATIVE_FLOAT);

  project::TMJLateralParms params;

  Group tmj_grp = file_->openGroup(group::kTabTMJ);
  DataSet ds = tmj_grp.openDataSet(ds::kTMJLateralParams);
  ds.read(&params, tmj_lateral_type);
  ds.close();
  tmj_grp.close();

  l_interval = params.left_interval;
  l_thickness = params.left_thickness;
  r_interval = params.right_interval;
  r_thickness = params.right_thickness;
}

void ProjectIOTMJ::SaveTMJMode(const bool& mode) {
  Group tmj_grp = file_->openGroup(group::kTabTMJ);
  IOFunctions::WriteBool(tmj_grp, ds::kTMJMode, mode);
  tmj_grp.close();
}

void ProjectIOTMJ::LoadTMJMode(bool& mode) {
  Group tmj_grp = file_->openGroup(group::kTabTMJ);
  mode = IOFunctions::ReadBool(tmj_grp, ds::kTMJMode);
  tmj_grp.close();
}

void ProjectIOTMJ::SaveMemo(const std::string& memo) {
  Group tmj_grp = file_->openGroup(group::kTabTMJ);
  IOFunctions::WriteString(tmj_grp, ds::kTMJMemo, memo);
  tmj_grp.close();
}

void ProjectIOTMJ::LoadMemo(std::string& memo) {
  Group tmj_grp = file_->openGroup(group::kTabTMJ);
  memo = IOFunctions::ReadString(tmj_grp, ds::kTMJMemo);
  tmj_grp.close();
}

void ProjectIOTMJ::SaveOrientationAngle(const float& d1, const float& d2,
                                        const float& d3) {
  Group tmj_grp = file_->openGroup(group::kTabTMJ);
  IOFunctions::WriteVec3(tmj_grp, ds::kTMJReorientAngle, glm::vec3(d1, d2, d3));
  tmj_grp.close();
}

void ProjectIOTMJ::LoadOrientationAngle(float& d1, float& d2, float& d3) {
  Group tmj_grp = file_->openGroup(group::kTabTMJ);
  glm::vec3 result = IOFunctions::ReadVec3(tmj_grp, ds::kTMJReorientAngle);
  tmj_grp.close();
  d1 = result.x;
  d2 = result.y;
  d3 = result.z;
}

void ProjectIOTMJ::SaveTMJROI(const float& bottom, const float& top,
                              const float& slice) {
  Group tmj_grp = file_->openGroup(group::kTabTMJ);
  IOFunctions::WriteVec3(tmj_grp, ds::kTMJROI, glm::vec3(top, bottom, slice));
  tmj_grp.close();
}

void ProjectIOTMJ::LoadTMJROI(float& bottom, float& top, float& slice) {
  Group tmj_grp = file_->openGroup(group::kTabTMJ);
  glm::vec3 result = IOFunctions::ReadVec3(tmj_grp, ds::kTMJROI);
  tmj_grp.close();
  top = result.x;
  bottom = result.y;
  slice = result.z;
}

void ProjectIOTMJ::SaveReorientation(const glm::mat4& matrix) {
  Group tmj_grp = file_->openGroup(group::kTabTMJ);
  IOFunctions::WriteMatrix(tmj_grp, ds::kTMJReoriMat, matrix);
  tmj_grp.close();
}

void ProjectIOTMJ::LoadReorientation(glm::mat4& matrix) {
  Group tmj_grp = file_->openGroup(group::kTabTMJ);
  matrix = IOFunctions::ReadMatrix(tmj_grp, ds::kTMJReoriMat);
  tmj_grp.close();
}

void ProjectIOTMJ::SaveLateralUp(const TMJDirectionType& direction,
                                     const glm::vec3& center) {
  Group tmj_grp = file_->openGroup(group::kTabTMJ);
  const std::string ds_name = direction == TMJDirectionType::TMJ_LEFT
                                  ? ds::kTMJLateralUpVecLeft
                                  : ds::kTMJLateralUpVecRight;
  IOFunctions::WriteVec3(tmj_grp, ds_name, center);
  tmj_grp.close();
}

bool ProjectIOTMJ::IsValidTMJ(const TMJDirectionType& direction) {
  Group tmj_grp = file_->openGroup(group::kTabTMJ);
  const std::string ds_name = direction == TMJDirectionType::TMJ_LEFT
                                  ? ds::kTMJLateralUpVecLeft
                                  : ds::kTMJLateralUpVecRight;
  bool exist = tmj_grp.exists(ds_name) ? true : false;
  tmj_grp.close();
  return exist;
}

void ProjectIOTMJ::LoadLateralUp(const TMJDirectionType& direction,
                                     glm::vec3& center) {
  Group tmj_grp = file_->openGroup(group::kTabTMJ);
  const std::string ds_name = direction == TMJDirectionType::TMJ_LEFT
                                  ? ds::kTMJLateralUpVecLeft
                                  : ds::kTMJLateralUpVecRight;
  center = IOFunctions::ReadVec3(tmj_grp, ds_name);
  tmj_grp.close();
}

void ProjectIOTMJ::SaveRectCenter(const TMJDirectionType& direction,
                                  const glm::vec3& center) {
  Group tmj_grp = file_->openGroup(group::kTabTMJ);
  const std::string ds_name = direction == TMJDirectionType::TMJ_LEFT
                                  ? ds::kTMJRectCenterLeft
                                  : ds::kTMJRectCenterRight;
  IOFunctions::WriteVec3(tmj_grp, ds_name, center);
  tmj_grp.close();
}

void ProjectIOTMJ::LoadRectCenter(const TMJDirectionType& direction,
                                  glm::vec3& center) {
  Group tmj_grp = file_->openGroup(group::kTabTMJ);
  const std::string ds_name = direction == TMJDirectionType::TMJ_LEFT
                                  ? ds::kTMJRectCenterLeft
                                  : ds::kTMJRectCenterRight;
  center = IOFunctions::ReadVec3(tmj_grp, ds_name);
  tmj_grp.close();
}

void ProjectIOTMJ::SaveCutPolygonPoints(const TMJDirectionType& direction,
                                        const std::vector<QPointF>& points) {
  Group tmj_grp = file_->openGroup(group::kTabTMJ);
  Group cut_pt_grp, cut_pt_direction_grp;
  if (!file_->exists(group::kTMJCutPoints)) {
    cut_pt_grp = tmj_grp.createGroup(group::kTMJCutPoints);
  } else {
    cut_pt_grp = tmj_grp.openGroup(group::kTMJCutPoints);
  }
  if (direction == TMJDirectionType::TMJ_LEFT) {
    cut_pt_direction_grp = cut_pt_grp.createGroup(group::kTMJCutPointsLeft);
  } else {
    cut_pt_direction_grp = cut_pt_grp.createGroup(group::kTMJCutPointsRight);
  }

  IOFunctions::WriteInt(cut_pt_direction_grp, ds::kTMJCutPolygonCount,
                        points.size());

  DataSet ds;
  for (int idx = 0; idx < points.size(); ++idx) {
    ds = cut_pt_direction_grp.createDataSet(
        std::to_string(idx), PredType::NATIVE_FLOAT, io::kDSPair);
    float data[2] = {static_cast<float>(points[idx].x()),
                     static_cast<float>(points[idx].y())};
    ds.write(data, PredType::NATIVE_FLOAT);
  }
  ds.close();
  cut_pt_direction_grp.close();
  cut_pt_grp.close();
  tmj_grp.close();
}

void ProjectIOTMJ::LoadCutPolygonPoints(const TMJDirectionType& direction,
                                        std::vector<QPointF>& points) {
  Group tmj_grp = file_->openGroup(group::kTabTMJ);
  if (!file_->exists(group::kTMJCutPoints)) return;

  Group cut_pt_grp, cut_pt_direction_grp;
  cut_pt_grp = tmj_grp.openGroup(group::kTMJCutPoints);

  if (direction == TMJDirectionType::TMJ_LEFT) {
    if (!cut_pt_grp.exists(group::kTMJCutPointsLeft)) {
      tmj_grp.close();
      return;
	}
    cut_pt_direction_grp = cut_pt_grp.openGroup(group::kTMJCutPointsLeft);
  } else {
    if (!cut_pt_grp.exists(group::kTMJCutPointsRight)) {
      tmj_grp.close();
      return;
    }
    cut_pt_direction_grp = cut_pt_grp.openGroup(group::kTMJCutPointsRight);
  }

  int poly_cnt = IOFunctions::ReadInt(cut_pt_direction_grp, ds::kTMJCutPolygonCount);
  points.clear();
  points.reserve(poly_cnt);

  DataSet ds;
  for (int idx = 0; idx < poly_cnt; ++idx) {
    ds = cut_pt_direction_grp.openDataSet(std::to_string(idx));
    float data[2] = {static_cast<float>(points[idx].x()),
                     static_cast<float>(points[idx].y())};
    ds.read(data, PredType::NATIVE_FLOAT);
    points.push_back(QPointF(data[0], data[1]));
  }
  ds.close();

  cut_pt_direction_grp.close();
  cut_pt_grp.close();
  tmj_grp.close();
}
