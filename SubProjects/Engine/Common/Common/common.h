#pragma once
/*=========================================================================

File:			class Common
Language:		C++11
Library:		Qt 5.2.0
Author:			Hong Jung
First date:		2015-11-21
Last date:		2016-04-26

Version:		1.0

Copyright (c) 2015 All rights reserved by HDXWILL.

=========================================================================*/

#include <vector>

#include <QPointF>

#if defined(__APPLE__)
#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>
#else
#define GLM_SWIZZLE
#include <GL/glm/glm.hpp>
#endif

#include "common_global.h"
class COMMON_EXPORT Common {
 public:
  Common();
  ~Common();

  static void generateCubicSpline(std::vector<QPointF>& lstControlPoints,
                                  std::vector<QPointF>& vecPointList,
                                  int nLineSeg = 30);
  static void generateCubicSpline(std::vector<glm::vec3>& lstControlPoints,
                                  std::vector<glm::vec3>& vecPointList,
                                  int nLineSeg = 30);
  static int projPointToSpline(const std::vector<glm::vec3> lstSpline,
                               const glm::vec3 point,
                               const glm::vec3 planeVector,
                               const float projRange);

  //점 간의 거리를 1px로 만들어줌.
  static void equidistanceSpline(std::vector<glm::vec3>& out,
                                 const std::vector<glm::vec3>& in);
  static void equidistanceSpline(std::vector<QPointF>& out,
                                 const std::vector<QPointF>& in);
  static void SmoothingHisto(int* histo, int histo_size,
                             std::vector<int>& smooth_histo, int histo_bin,
                             int seg_histo, bool is_sqrt_histo);
  static void GetBinningCurve(int* histo, int histo_size, int histo_bin,
                              std::vector<QPointF>& curve_points, bool is_sqrt);
  static void SamplingSpline(const std::vector<QPointF>& spline,
                             std::vector<QPointF>& out_sampled_spline);

  static std::vector<std::string> ReadCSV(const QString& file_path);
};
