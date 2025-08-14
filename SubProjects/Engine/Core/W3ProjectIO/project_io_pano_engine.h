#pragma once
/*=========================================================================

File:			class ProjectIOPanoEngine
Language:		C++11
Library:        Qt 5.8.0
Author:			Seo Seok Man
First date:		2018-07-14
Last modify:	2016-07-14

=========================================================================*/
#include <map>
#include <memory>
#include <vector>
#if defined(__APPLE__)
#include <glm/glm.hpp>
#else
#include <GL/glm/glm.hpp>
#endif
#include "../../Common/Common/W3Enum.h"

#include "datatypes.h"
#include "w3projectio_global.h"

namespace H5 {
class H5File;
}

class ImplantResource;

class W3PROJECTIO_EXPORT ProjectIOPanoEngine {
 public:
  ProjectIOPanoEngine(const project::Purpose& purpose,
                      const std::shared_ptr<H5::H5File>& file);
  ~ProjectIOPanoEngine();

  ProjectIOPanoEngine(const ProjectIOPanoEngine&) = delete;
  ProjectIOPanoEngine& operator=(const ProjectIOPanoEngine&) = delete;

 public:
  void SavePanoROI(float top, float bottom, float slice);
  void SaveReoriMatrix(const ArchTypeID& arch_type,
                       const glm::mat4& reorientation);
  void SaveImplant3DMVP(const glm::mat4& mvp);
  void SaveCSShiftedValue(float shifted_value);
  void SaveCurrArchType(const ArchTypeID& arch_type);

  void SavePanoCtrlPoints(const ArchTypeID& arch_type,
                          const std::vector<glm::vec3>& pano_ctrl_points);
  void SavePanoShiftValue(const ArchTypeID& arch_type,
                          const float& pano_shift_value);
  void SaveNervePoints(
      const std::map<int, std::vector<glm::vec3>>& nerve_ctrl_points,
      const std::map<int, std::vector<glm::vec3>>& nerve_spline_points);
  void SaveNerveParams(const int& idx, const int& nerve_id, int color_r,
                       int color_g, int color_b, bool visible, float radius,
                       double diameter_mm);
  void SaveNerveCount(const int& nerve_cnt);
  void SaveImplantResource(const ImplantResource& imp_res, QString appfile_path = "");

  void LoadPanoROI(float& top, float& bottom, float& slice);
  void LoadReoriMatrix(const ArchTypeID& arch_type, glm::mat4& reorientation);
  void LoadImplant3DMVP(glm::mat4& mvp);
  void LoadCSShiftedValue(float& shifted_value);
  void LoadCurrArchType(ArchTypeID& arch_type);

  void LoadPanoCtrlPoints(const ArchTypeID& arch_type,
                          std::vector<glm::vec3>& pano_ctrl_points);
  void LoadPanoShiftValue(const ArchTypeID& arch_type, float& pano_shift_value);
  void LoadNervePoints(
      std::map<int, std::vector<glm::vec3>>& nerve_ctrl_points,
      std::map<int, std::vector<glm::vec3>>& nerve_spline_points);
  void LoadNerveParams(const int& idx, int& nerve_id, int& color_r,
                       int& color_g, int& color_b, bool& visible, float& radius,
                       double& diameter_mm);
  void LoadNerveCount(int& nerve_cnt);
  void LoadImplantResource(project::ImplantResParams& imp_resource);

 private:
  std::shared_ptr<H5::H5File> file_;
};
