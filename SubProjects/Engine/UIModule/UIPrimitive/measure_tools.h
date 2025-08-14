#pragma once
/*=========================================================================

File:			class MeasureTools
Language:		C++11
Library:		Qt 5.2.0, Standard C++ Library
Author:			Seo Seok Man
First Date:		2018-02-02
Modify Date:	2018-05-15
Version:		2.0

Copyright (c) 2015~2018 All rights reserved by HDXWILL.

=========================================================================*/
#include <memory>

#if defined(__APPLE__)
#include <glm/glm.hpp>
#else
#include <gl/glm/glm.hpp>
#endif

#include <qobject.h>
#include <qpoint.h>

#include <Engine/Common/Common/W3Enum.h>
#include <Engine/Common/Common/define_measure.h>
#include "uiprimitive_global.h"

class QGraphicsScene;
class MeasureBase;
class MeasureData;

class UIPRIMITIVE_EXPORT MeasureTools : public QObject
{
	Q_OBJECT

public:
	MeasureTools(const common::ViewTypeID& view_type, QGraphicsScene* pScene);
	MeasureTools(const common::ViewTypeID& view_type, int view_sub_id, QGraphicsScene* scene);
	virtual ~MeasureTools();

signals:
	void sigProtterUpdate();
	void sigGetProfileData(const QPointF& start_pt, const QPointF& end_pt,
		std::vector<short>& data);
	void sigGetROIData(const QPointF& start_pt_scene, const QPointF& end_pt_scene,
		std::vector<short>& data);
	void sigMeasureCreated(const unsigned int& id);
	void sigMeasureDeleted(const unsigned int& id);
	void sigMeasureModified(const unsigned int& id);

private:
	enum class EventType { DRAW, END, SELECT, MODIFY, DEL, NONE };

	/** Values that represent Synchronise types.
		  COUNTERPART : 다른 탭의 같은 타입의 뷰의 경우. ex) PanoTab:ArchView <->
	 ImplantTab:ArchView SIBLINGS : 같은 탭의 다른 뷰의 경우. ex) PanoTab:CS_1 <->
	 PanoTab:CS_2
  */
	enum class SyncType { COUNTERPART, SIBLINGS };

	typedef std::vector<std::shared_ptr<MeasureBase> > MeasureUIList;

public:
	void Initialize(const float& pixel_spacing, const float& scene_to_gl);

	bool IsMeasureInteractionAvailable(const common::CommonToolTypeOnOff& anno_type);
	void ProcessMousePressed(const QPointF& pt, const glm::vec3& ptVol);
	void ProcessMouseMove(const QPointF& pt, const glm::vec3& ptVol);
	void ProcessMouseReleased(Qt::MouseButton button, const QPointF& pt,
		const glm::vec3& ptVol);
	void ProcessMouseDoubleClick(Qt::MouseButton button, const QPointF& pt, const glm::vec3& ptVol);

	void DeleteAllMeasure();
	void ClearUnfinishedItem();

	void SetVisible(bool is_visible_from_toolbar, bool is_visible_from_view,
		const common::ReconTypeID& mode);
	void SetVisible(bool is_visible_from_toolbar, bool is_visible_from_view);
	void SetVisibleFromToolbar(bool visible);
	void SetVisibleFromView(bool visible);
	void SetViewRenderMode(const common::ReconTypeID& mode);
	void SetCenter(const QPointF& point);
	void SetScale(const float scale);
	void SetZoomFactor(const float zoom_factor);
	void SetSceneTrans(const QPointF& trans);
	void SetPixelSpacing(float pixel_spacing);
	void SetMeasureType(const common::CommonToolTypeOnOff& toolType);

	void TransformItems(const QTransform& transform, const bool translate = false);

	void Update(const glm::vec3& vp_center, const glm::vec3& vp_up,
		const glm::vec3& vp_back);
	void UpdateProjection();

	const bool IsDrawing() const;
	const bool IsSelected() const;

	void ApplyPreferences();
	void ImportMeasureResource(const bool& is_update_resource = true);

	void SyncMeasureResourceSiblings(const bool& is_update_resource);
	void SyncMeasureResourceCounterparts(const bool& is_update_resource, const bool need_transform = true);
	void SyncCreateMeasureUI(const unsigned int& id);
	void SyncDeleteMeasureUI(const unsigned int& id);
	void SyncModifyMeasureUI(const unsigned int& id);
	void GetMeasureParams(const common::ViewTypeID& view_type,
		const unsigned int& measure_id,
		common::measure::VisibilityParams* measure_params);

	inline bool visible_from_toolbar() const noexcept { return visible_from_toolbar_; }
	inline bool visible_from_view() const noexcept { return visible_from_view_; }

	void TransformPointsForCounterparts();

public slots:
	void slotSyncSelectedStatus(bool selected);

private:
	void ActionDrawMeasure(const QPointF& pt, const glm::vec3& ptVol);
	void ActionDeleteMeasure(const QPointF& pt, const glm::vec3& ptVol);
	void ActionHighlightMeasure();

	void AddMeasureToMeasureList(MeasureBase* obj, const QPointF& pt,
		const glm::vec3& ptVol);
	void NewMeasure(const QPointF& pt, const glm::vec3& ptVol);
	void CreateMeasureUI(const common::measure::MeasureType& measure_type,
		MeasureBase*& obj);
	void RestoreMeasureFromMeasureData(
		const std::weak_ptr<MeasureData>& measure_data,
		const std::vector<QPointF>& points);
	void SyncMeasureInternal(const common::measure::ViewInfo& counter_view_info, const bool& is_update_resource);
	void EndMeasure();

	void DeleteLastMeasure();
	void DeleteSelectedMeasure();

	void CheckVisibility();
	void SelectEventType();
	void UpdateMeasure();
	const bool IsSiblingType() const;
	const bool IsMPRType() const;
	void SyncMeasureResource(const SyncType& sync_type,
		const bool& is_update_resource, const bool need_transform = true);

private:
	common::ReconTypeID view_render_mode_ = common::ReconTypeID::MPR;
	EventType event_type_ = EventType::NONE;
	common::ViewTypeID view_id_ = common::ViewTypeID::UNKNOWN;
	int view_sub_id_ = 0;

	common::measure::MeasureType curr_measure_type_ = common::measure::MeasureType::NONE;
	common::measure::VisibilityParams view_visibility_params_;
	common::measure::ViewInfo curr_view_info_;

	// hide measure 에 의해 visibility를 변경하기 위한 변수
	bool visible_from_toolbar_ = true;
	// view 개별 visibility를 변경하기 위한 변수(tab key pressed)
	bool visible_from_view_ = true;

	MeasureUIList measure_ui_list_;
	std::weak_ptr<MeasureBase> selected_measure_;
	QGraphicsScene* scene_ = nullptr;
};
