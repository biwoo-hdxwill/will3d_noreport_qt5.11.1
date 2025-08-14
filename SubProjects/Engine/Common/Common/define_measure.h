#pragma once

#include <QTransform>

#include <map>
#include <memory>
#include <vector>

#if defined(__APPLE__)
#include <glm/vec3.hpp>
#else
#include <gl/glm/vec3.hpp>
#endif
#include <qpoint.h>

#include "define_view.h"

class MeasureData;

namespace common {
namespace measure {
/*
        2D annotation 이 현재 보이는지를 체크하기 위한 변수들.
        Update 가 되기 전까지 이 값들을 이용하여 새로 입력되는 annotation 들의
        위치 및 방향을 설정한다. 입력한 위치 및 방향 값은 변하지 않으며,
        annotation 의 고유 위치로 기억되어 변경된 view plane 의 위치와 비교하여
        annotation 들의 visibility 를 결정한다.
*/
struct VisibilityParams {
  glm::vec3 center;
  glm::vec3 up;
  glm::vec3 back;
  float thickness = 0.0f;
};

typedef struct _ViewInfo {
  QPointF scene_center;
  QPointF translate;
  float spacing = 0.0f;
  float scale = 0.0f;  // scene_to_gl / vol_to_gl
  float zoom_factor = 1.0f;
  QTransform transform;
  QTransform translate_transform;
  QTransform scale_transform;
} ViewInfo;

typedef std::map<ViewTypeID, std::vector<std::weak_ptr<MeasureData>>>
    MeasureDataContainer;

enum class PathType { LINE, CURVE };

enum class DrawMode { LENGTH, AREA, ROI, JUSTDRAW };

enum class GuideType { NONE, LINE, ARC };

enum class Shape { RECT, CIRCLE, POLYGON };

enum class MeasureType : int {
  NONE = 0,
  LENGTH_LINE,
  LENGTH_TAPELINE,
  LENGTH_CURVE,
  ANGLE_THREEPOINT,
  PROFILE,
  AREA_LINE,
  ROI_RECT,
  DRAW_ARROW,
  DRAW_CIRCLE,
  DRAW_RECT,
  DRAW_LINE,
  DRAW_FREEDRAW,
  NOTE,
  DELETE_ONE
};
}  // end of namespace measure
}  // end of namespace common
