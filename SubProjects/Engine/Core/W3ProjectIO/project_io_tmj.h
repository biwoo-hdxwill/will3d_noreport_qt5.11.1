#pragma once
/*=========================================================================

File:			class ProjectIOTMJ
Language:		C++11
Library:        Qt 5.8.0
Author:			Seo Seok Man
First date:		2018-07-16
Last modify:	2016-07-16

=========================================================================*/
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
#include "datatypes.h"
#include "w3projectio_global.h"
namespace H5 {
class H5File;
}
class ProjectIOView;

class W3PROJECTIO_EXPORT ProjectIOTMJ {
 public:
  ProjectIOTMJ(const project::Purpose& purpose,
               const std::shared_ptr<H5::H5File>& file);
  ~ProjectIOTMJ();

  ProjectIOTMJ(const ProjectIOTMJ&) = delete;
  ProjectIOTMJ& operator=(const ProjectIOTMJ&) = delete;

  enum class ViewType {
    AXIAL,
    FRONTAL_LEFT,
    FRONTAL_RIGHT,
    LATERAL_LEFT,
    LATERAL_RIGHT,
    THREE_D_LEFT,
    THREE_D_RIGHT
  };

 public:
  void InitTMJTab();
  bool IsInit();

  void InitializeView(ProjectIOTMJ::ViewType view_type, const int& view_id = 0);
  ProjectIOView& GetViewIO();

  void SaveTMJRect(const float& left_width, const float& left_height,
                   const float& right_width, const float& right_height);
  void LoadTMJRect(float& left_width, float& left_height, float& right_width,
                   float& right_height);

  void SaveLateralParams(const float& left_interval,
                         const float& left_thickness,
                         const float& right_interval,
                         const float& right_thickness);
  void LoadLateralParams(float& left_interval, float& left_thickness,
                         float& right_interval, float& right_thickness);

  /**********************************************************************************************
  Saves a tmj mode.
  @param	mode = True : 2D, False : 3D
   **********************************************************************************************/
  void SaveTMJMode(const bool& mode);
  void LoadTMJMode(bool& mode);

  void SaveMemo(const std::string& memo);
  void LoadMemo(std::string& memo);

  void SaveOrientationAngle(const float& d1, const float& d2, const float& d3);
  void LoadOrientationAngle(float& d1, float& d2, float& d3);

  void SaveTMJROI(const float& bottom, const float& top, const float& slice);
  void LoadTMJROI(float& bottom, float& top, float& slice);

  void SaveReorientation(const glm::mat4& matrix);
  void LoadReorientation(glm::mat4& matrix);

  bool IsValidTMJ(const TMJDirectionType& direction);
  void SaveLateralUp(const TMJDirectionType& direction,
                     const glm::vec3& center);
  void LoadLateralUp(const TMJDirectionType& direction, glm::vec3& center);

  void SaveRectCenter(const TMJDirectionType& direction,
                      const glm::vec3& center);
  void LoadRectCenter(const TMJDirectionType& direction, glm::vec3& center);

  void SaveCutPolygonPoints(const TMJDirectionType& direcion,
                            const std::vector<QPointF>& points);
  void LoadCutPolygonPoints(const TMJDirectionType& direcion,
                            std::vector<QPointF>& points);

 private:
  std::shared_ptr<H5::H5File> file_;
  std::unique_ptr<ProjectIOView> view_io_;
  ViewType curr_view_type_;
  int curr_view_id_;
};
