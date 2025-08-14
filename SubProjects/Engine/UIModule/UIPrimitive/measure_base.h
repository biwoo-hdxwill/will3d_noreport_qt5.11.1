#pragma once
/*=========================================================================

File:			base_measure.h
Language:		C++11
Library:		Qt 5.2.0, Standard C++ Library
Author:			Seo Seok Man
First Date:		2018-10-19
Modify Date:	2018-10-19
Version:		1.0

Copyright (c) 2018 All rights reserved by HDXWILL.

=========================================================================*/
#include <memory>
#include <vector>

#if defined(__APPLE__)
#include <glm/glm.hpp>
#else
#include <gl/glm/glm.hpp>
#endif

#include <qpoint.h>
#include <qstring.h>

#include <Engine/Common/Common/define_measure.h>
#include "uiprimitive_global.h"

class QGraphicsScene;
class MeasureData;

class UIPRIMITIVE_EXPORT MeasureBase {
 public:
  MeasureBase() {}
  virtual ~MeasureBase() {}

 public:
  virtual void setVisible(bool bShow) = 0;
  virtual void setSelected(bool bSelected) = 0;
  virtual void setNodeDisplay(bool bDisplay) = 0;
  virtual void UpdateMeasure(){};

  virtual void processMouseMove(const QPointF& pt, const glm::vec3& ptVol = glm::vec3(0.0f)) = 0;
  virtual void processMouseReleased(const QPointF& pt, const glm::vec3& ptVol = glm::vec3(0.0f)) = 0;
  virtual void processMouseDoubleClicked(const QPointF& pt, const glm::vec3& ptVol = glm::vec3(0.0f)) = 0;
  virtual bool InputParam(QGraphicsScene* pScene, const QPointF& pt, const glm::vec3& ptVol, bool& done) = 0;

  virtual bool IsSelected() const = 0;
  virtual bool TransformItems(const QTransform& transform) = 0;
  virtual std::vector<QPointF> GetMeasurePoints() const {
    return std::vector<QPointF>();
  }
  virtual void ApplyPreferences(){};
  virtual QString GetValue() { return QString(); }

  inline const bool& is_drawing() const noexcept { return is_drawing_; }

  unsigned int id() const noexcept;
  common::measure::MeasureType type() const noexcept;
  const common::measure::VisibilityParams& GetVisibilityParams();
  bool VisibilityCheck(common::measure::VisibilityParams& view_vp) const;
  void SetMeasureData(const std::weak_ptr<MeasureData>& data);

 protected:
  // ui 속성들.
  bool is_drawing_ = false;
  bool is_selected_ = false;
  int node_count_ = 0;

  // read-only resource. MeasureResourceMgr에서만 Add/remove/modify가능
  std::weak_ptr<MeasureData> data_;
};
