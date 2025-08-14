#pragma once

/**=================================================================================================

Project: 			UIComponent
File:				scene.h
Language:			C++11
Library:			Qt 5.8.0
author:				Tae Hoon Yoo
First date:			2017-08-31
Last modify:		2017-08-31

Copyright (c) 2017 All rights reserved by HDXWILL.

 *===============================================================================================**/
#include <QGraphicsScene>

#include <Engine/Common/Common/define_view.h>
#include "viewitems.h"

namespace common {
enum class ReconTypeID;
namespace measure {
enum class SyncType;
struct VisibilityParams;
}  // end of namespace measure
}  // end of namespace common

class ViewRenderParam;
class MeasureTools;

class Scene : public QGraphicsScene {
  Q_OBJECT

 public:
  Scene(QObject* parent = nullptr);
  ~Scene();

  Scene(const Scene&) = delete;
  Scene& operator=(const Scene&) = delete;

  const bool MeasureVisibleFromToolbar();

signals:

  void sigRotateMatrix(const glm::mat4&);
  void sigSliderValueChanged(int);
  void sigActiveFilteredItem(const QString& text);
  void sigActiveSharpenItem(const int index);

  void sigGetProfileData(const QPointF& pt_scene_start,
						 const QPointF& pt_scene_end, std::vector<short>& data);
  void sigGetROIData(const QPointF& start_pt_scene, const QPointF& end_pt_scene,
					 std::vector<short>& data);
  void sigMeasureCreated(const unsigned int& measure_id);
  void sigMeasureDeleted(const unsigned int& measure_id);
  void sigMeasureModified(const unsigned int& measure_id);

 public:
  void InitViewItem(const Viewitems::ItemType& type);

  void ViewEnableStatusChanged(const bool& view_enable);
  void SetEnabledItem(const Viewitems::ItemType& type, const bool& is_enabled);
  void SetRulerColor(const QColor& color);
  void SetBorderColor(const QColor& color);
  void SetSliderTextColor(const QColor& color);

  void HideAllUI(const bool& is_hide);
  void HideText(const bool& is_hide);

  void SetWorldAxisDirection(const glm::mat4& rotate_mat,
							 const glm::mat4& view_mat);
  void SetSliderValue(const int& value);
  void SetSliderRange(const int& min, const int& max);
  void SetSliderInvertedAppearance(const bool& is_enable);
  void SetSliderInterval(const int& interval);
  void SetViewRulerItem(const ViewRenderParam& param);
  void SetGridItem(const ViewRenderParam& view_param);
  void AddFilteredItem(const QString& text);
  void ChangeFilteredItemText(const QString& text);
  void ChangeSharpenLevel(const int level);
  void SetDirectionTextItem(const QString& text, const bool& is_align_left);
  bool IsMouseOnViewItems();

  void SetHUValue(const QString& text);
  void SetGridOnOff(const bool& visible);

  void GetSliceRange(int* min, int* max);

  int GetSliderValue() const;

  void resizeEvent(const ViewRenderParam& param);
  bool EditSliderValue(const int& delta);

  // measure tools를 위한 access functions
  void InitMeasure(const common::ViewTypeID& view_type, int view_sub_type = 0);
  void UpdatePlaneInfo(const glm::vec3& vp_center, const glm::vec3& vp_up,
					   const glm::vec3& vp_back);
  void SetMeasureType(const common::CommonToolTypeOnOff& type);
  void DeleteAllMeasure();
  void DeleteUnfinishedMeasure();
  void SetMeasureReconType(const common::ReconTypeID& recon_type);
  void SetMeasureParams(const ViewRenderParam& view_param);
  void SetMeasureParamPixelSpacing(const float& pixel_spacing);
  void SetMeasureParamScale(const float& scale);
  void SetMeasureParamZoomFactor(const float& zoom_factor);
  void SetMeasureParamTrans(const QPointF& trans);
  bool IsMeasureInteractionAvailable(const common::CommonToolTypeOnOff& anno_type);
  void TransformMeasure(const QTransform& transform, const bool translate = false);
  void HideMeasure(const bool& toggled);

  void MeasureMousePress(const QPointF& scene_pos);
  bool MeasureMouseMove(Qt::MouseButtons buttons,
						const common::CommonToolTypeOnOff& anno_type,
						const QPointF& scene_pos);
  void MeasureMouseRelease(Qt::MouseButton button, const QPointF& scene_pos);
  void MeasureMouseDoubleClick(Qt::MouseButton button, const QPointF& scene_pos);
  void ImportMeasureResource(const bool& is_update_resource = true);
  void SyncMeasureResourceSiblings(const bool& is_update_resource);
  void SyncMeasureResourceCounterparts(const bool& is_update_resource, const bool need_transform = true);
  void SyncCreateMeasureUI(const unsigned int& measure_id);
  void SyncDeleteMeasureUI(const unsigned int& measure_id);
  void SyncModifyMeasureUI(const unsigned int& measure_id);
  void GetMeasureParams(const common::ViewTypeID& view_type,
						const unsigned int& measure_id,
						common::measure::VisibilityParams* measure_params);

  void TransformMeasureForCounterparts();

  // end of measure tools access functions
  void ApplyPreferences();
  const bool IsTextVisible() const;
  const bool IsUIVisible() const;
  const bool IsMeasureSelected() const;

  void SetVisible(const bool visible);

 private:
  Viewitems* viewitems_ = nullptr;
  std::unique_ptr<MeasureTools> measure_tool_;
};
