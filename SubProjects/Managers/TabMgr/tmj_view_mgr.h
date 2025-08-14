#pragma once

/**=================================================================================================

Project:		TabMgr
File:			tmj_view_mgr.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-11-13
Last modify: 	2018-11-13

				Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/
#include <vector>
#if defined(__APPLE__)
#include <glm/vec4.hpp>
#else
#include <gl/glm/vec4.hpp>
#endif

#include <Engine/Common/Common/define_tmj.h>
#include <Engine/UIModule/UITools/tmj_task_tool.h>
#include "base_view_mgr.h"

class ViewTMJaxial;
class ViewTMJorientation;
class ViewTMJ3D;
class TMJengine;
class ViewTmjLateral;
class ViewTmjFrontal;
#ifndef WILL3D_VIEWER
class ProjectIOTMJ;
#endif

enum SharpenLevel;

class TMJViewMgr : public BaseViewMgr
{
	Q_OBJECT
public:
	explicit TMJViewMgr(QObject* parent = nullptr);
	virtual ~TMJViewMgr();

	TMJViewMgr(const TMJViewMgr&) = delete;
	TMJViewMgr& operator=(const TMJViewMgr&) = delete;

public:
#ifndef WILL3D_VIEWER
	void exportProject(ProjectIOTMJ& out);
	void importProject(ProjectIOTMJ& in);
#endif

	void MoveViewsToSelectedMeasure(const common::ViewTypeID& view_type,
		const unsigned int& measure_id);

	void DeleteTMJROIRect(const TMJDirectionType& type);
	void ResetOrientationParams();
	void GridOnOffOrientation(const bool& on);

	void SyncTMJItemVisibleUI();

	void SetTMJengine(const std::shared_ptr<TMJengine>& tmj_engine);
	void SelectLayoutLateral(const TMJDirectionType& type, const int& row,
		const int& column);
	void SelectLayoutFrontal(const TMJDirectionType& type, const int& count);
	void set_task_tool(const std::shared_ptr<TMJTaskTool>& task_tool);

	QWidget* GetViewAxial() const;
	QWidget* GetViewOrient(const ReorientViewID& id) const;
	QWidget* GetViewFrontal3DWidget(const TMJDirectionType& type) const;
	std::vector<QWidget*> GetViewFrontalWidget(
		const TMJDirectionType& type) const;
	std::vector<QWidget*> GetViewLateralWidget(
		const TMJDirectionType& type) const;

	bool ShiftLateralView(const TMJDirectionType& direction);
	bool ShiftFrontalView(const TMJDirectionType& direction);

	void SetFrontalLineIndex(const TMJDirectionType& direction, int index);
	void SetLateralLineIndex(const TMJDirectionType& direction, int index);

#ifdef WILL3D_EUROPE
	void SetSyncControlButtonOut();
#endif // WILL3D_EUROPE

private:
	void InitializeViews();
	void connections();
	void ConnectTMJMenus();

	void MoveFrontalViewToSelectedMeasure(
		const TMJDirectionType& type,
		const common::measure::VisibilityParams& visibility_param);
	void MoveLateralViewToSelectedMeasure(
		const TMJDirectionType& type,
		const common::measure::VisibilityParams& visibility_param);
	void MoveArchViewToSelectedMeasure(
		const common::measure::VisibilityParams& visibility_param);

	void UpdateTMJ();
	void UpdateResourceLateral(const TMJDirectionType& type,
		bool* is_update_lateral, bool* is_update_cut,
		bool* is_update_rect);
	void UpdateResourceFrontal(const TMJDirectionType& type,
		bool* is_update_frontal, bool* is_update_cut,
		bool* is_update_rect);
	void UpdateAxialLines();
	void UpdateFrontalLines();

	void SyncTMJparamsFromUI();
	void SyncTMJRectSizeUI();

	int GetLateralViewCount(const TMJDirectionType& type);
	void DeleteMeasuresInTMJRectViews(const TMJDirectionType& type);

	void TranslateLateral(const TMJDirectionType& type,
		const glm::vec3& pt_center_translated,
		const glm::vec3& pt_center_current);
	void TranslateFrontal(const TMJDirectionType& type,
		const glm::vec3& pt_center_translated,
		const glm::vec3& pt_center_current);

#ifdef WILL3D_EUROPE
	void SetAllSyncControlButton(const bool is_on);
#endif // WILL3D_EUROPE

private slots:
	void slotSyncWindowing();
	void slotSyncLateralZoom(const float& scene_scale);
	void slotSyncLateralPan(const QPointF& trans);
	void slotSyncFrontalZoom(const float& scene_scale);
	void slotSyncFrontalPan(const QPointF& trans);

	void slotAxialPositionChanged();
	void slotRequestInitializeFromAxial();
	void slotUpdateTMJ(const TMJDirectionType& type);
	void slotOrientationViewRotate(const ReorientViewID& view_type,
		const glm::mat4& orien);
	void slotOrientationFromTaskTool(const ReorientViewID& view_type, int angle);
	void slotOrientationViewRenderQuality(const ReorientViewID& view_type);
	void slotChangeROIFromOrienView();
	void slotUpdateDoneROIFromOrienView();
	void slotChangeSliceFromOrienView();

	void slotTMJCutEnable(const bool& cut_on, const VRCutTool& cut_tool);
	void slotTMJCutReset(const TMJDirectionType& direction_type);
	void slotTMJCutUndo(const TMJDirectionType& direction_type);
	void slotTMJCutRedo(const TMJDirectionType& direction_type);
	void slotTMJCutFrontal(const TMJDirectionType& direction_type,
		const QPolygonF& cut_area, const bool& is_inside);
	void slotTMJClipParamsChange(const TMJTaskTool::ClipID& clip_id,
		const std::vector<float>& values,
		bool clip_enable);
	void slotLateralParamChanged(const TMJLateralID& id, double value);
	void slotLateralViewSelected(const TMJDirectionType& direction_type,
		const int& lateral_id);

	void slotWheelEventFromLateralView(const int& wheel_step);
	void slotWheelEventFromFrontalView(const int& wheel_step);
	void slotSetLateralSliceFromFrontalView(const glm::vec3& pt_vol);
	void slotSetFrontalSliceFromLateralView(const glm::vec3& pt_vol);
	void slotSetAxialZ(const glm::vec3& pt_vol);

	void slotTMJLateralSharpen(const TMJDirectionType& type, SharpenLevel level);
	void slotTMJFrontalSharpen(const TMJDirectionType& type, SharpenLevel level);

	void slotSetLateralViewStatus(float* scene_scale, QPointF* gl_translate);
	void slotSetFrontalViewStatus(float* scene_scale, QPointF* gl_translate);

	void slotLateralMeasureCreated(const TMJDirectionType& type,
		const int& lateral_id,
		const unsigned int& measure_id);
	void slotLateralMeasureDeleted(const TMJDirectionType& type,
		const int& lateral_id,
		const unsigned int& measure_id);
	void slotLateralMeasureModified(const TMJDirectionType& type,
		const int& lateral_id,
		const unsigned int& measure_id);
	void slotFrontalMeasureCreated(const TMJDirectionType& type,
		const int& lateral_id,
		const unsigned int& measure_id);
	void slotFrontalMeasureDeleted(const TMJDirectionType& type,
		const int& lateral_id,
		const unsigned int& measure_id);
	void slotFrontalMeasureModified(const TMJDirectionType& type,
		const int& lateral_id,
		const unsigned int& measure_id);

private:
	std::shared_ptr<TMJengine> tmj_engine_;
	std::shared_ptr<TMJTaskTool> task_tool_;
	std::shared_ptr<ViewTMJaxial> view_axial_;
	std::shared_ptr<ViewTmjLateral> view_lateral_[TMJDirectionType::TMJ_TYPE_END][common::kMaxLateralCount];
	std::shared_ptr<ViewTmjFrontal> view_frontal_[TMJDirectionType::TMJ_TYPE_END][common::kMaxFrontalCount];
	std::shared_ptr<ViewTMJ3D> view_3d_[TMJDirectionType::TMJ_TYPE_END];
	std::shared_ptr<ViewTMJorientation> view_orient_[ReorientViewID::REORI_VIEW_END];

	int frontal_count_[TMJDirectionType::TMJ_TYPE_END] = { 1 };

	struct LayoutShape
	{
		int row = 0;
		int col = 0;
	};
	LayoutShape lateral_layout_shape_[TMJDirectionType::TMJ_TYPE_END];
};
