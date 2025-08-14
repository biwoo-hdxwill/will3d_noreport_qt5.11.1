#pragma once

/**=================================================================================================

Project:		UIComponent
File:			view_tmj_frontal.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-12-04
Last modify: 	2018-12-04

		Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/
#include <functional>
#include <memory>
#include <vector>

#include "../../Common/Common/W3Enum.h"
#include "uicomponent_global.h"
#include "view.h"

class QGraphicsPolygonItem;

class AnnotationFreedraw;
class ViewControllerSlice;
class GuideLineListItem;
class TMJfrontalResource;
class TMJresource;
#ifndef WILL3D_VIEWER
class ProjectIOTMJ;
#endif

enum SharpenLevel;

class UICOMPONENT_EXPORT ViewTmjFrontal : public View
{
	Q_OBJECT
public:
	ViewTmjFrontal(int frontal_id, const common::ViewTypeID& view_type, TMJDirectionType type, QWidget* parent = 0);
	virtual ~ViewTmjFrontal();

	ViewTmjFrontal(const ViewTmjFrontal&) = delete;
	ViewTmjFrontal& operator=(const ViewTmjFrontal&) = delete;

signals:
	void sigWindowingDone();
	void sigZoomDone(const float& scene_scale);
	void sigPanDone(const QPointF& trans);
	void sigSetSharpen(SharpenLevel level);
	void sigWheelEvent(const int& wheel_step);
	void sigSetLateralSlice(const glm::vec3& pt_vol);
	void sigSetAxialSlice(const glm::vec3& pt_vol);
	void sigCut(const TMJDirectionType& direction_type, const QPolygonF& cut_area, const bool& is_inside);
	void sigMeasureCreated(const TMJDirectionType& direction_type, const int& frontal_id, const unsigned int& measure_id);
	void sigMeasureDeleted(const TMJDirectionType& direction_type, const int& frontal_id, const unsigned int& measure_id);
	void sigMeasureModified(const TMJDirectionType& direction_type, const int& frontal_id, const unsigned int& measure_id);

public:
	void SetEnabledSharpenUI(const bool& is_enabled);
	void SetSharpen(SharpenLevel level);
	void SetLateralVisible(const bool& visible);
	void SetLateralSelected(const int& lateral_id);
	void SetTMJCutMode(const bool& cut_on, const VRCutTool& cut_tool);
	void ResetTMJCutUI();
	void UndoTMJCutUI();
	void RedoTMJCutUI();
	void UpdateThickness();
	void UpdateUI();
	void UpdateFrontal();
	void ClearLateralLine();

	void GetMapVolToSceneFunc(std::function<void(const std::vector<glm::vec3>&, std::vector<QPointF>&)>& callback);
	virtual void SetCommonToolOnOff(const common::CommonToolTypeOnOff& type) override;
	virtual void HideAllUI(bool is_hide) override;

	// serialize
#ifndef WILL3D_VIEWER
	void ExportProject(ProjectIOTMJ& out, float* scene_scale, float* scene_to_gl, QPointF* trans_gl);
	void ImportProject(ProjectIOTMJ& in, const float& scene_scale, const float& scene_to_gl, const QPointF& trans_gl, const bool& is_update_resource = true);
#endif

protected slots:
	virtual void slotActiveSharpenItem(const int index) override;

protected:
	virtual void TransformItems(const QTransform& transform) override;
	virtual void leaveEvent(QEvent* event) override;
	virtual void enterEvent(QEvent* event) override;
	virtual void wheelEvent(QWheelEvent* event) override;
	virtual void keyPressEvent(QKeyEvent* event) override;
	virtual void keyReleaseEvent(QKeyEvent* event) override;
	virtual void resizeEvent(QResizeEvent* pEvent) override;
	virtual void mousePressEvent(QMouseEvent* event) override;
	virtual void mouseMoveEvent(QMouseEvent* event) override;
	virtual void mouseReleaseEvent(QMouseEvent* event) override;
	virtual void mouseDoubleClickEvent(QMouseEvent* event) override;

	bool IsInFrontalPlane(const int& x, const int& y) const;
	void RenderSlice();

	inline ViewControllerSlice* controller_slice() const { return controller_slice_.get(); }

private:
	virtual void SetGraphicsItems() override;
	virtual void InitializeController() override;
	virtual bool IsReadyController() override;
	virtual void ClearGL() override;
	virtual void ActiveControllerViewEvent() override;

	virtual void drawBackground(QPainter* painter, const QRectF& rect) override;

	void RenderScreen();
	void SetFrontalPlane();

	void UpdateMeasurePlaneInfo();

	void RequestDICOMInfo(const QPointF& pt_scene);
	void DisplayDICOMInfo(const glm::vec4& vol_info);

	void CreateCutUI(const QPointF& curr_scene_pos);
	void Draw3DCutUI(QPolygonF& poly, const QPointF& curr_scene_pos);
	void DrawCutSelectAreaUI(QPolygonF& poly, const QPointF& curr_scene_pos);
	void DrawCutResultAreaUI();
	void EndDrawCutUI();
	void DeleteCutUI();
	void DeleteCutSelectedArea();
	void SelectCutArea();
	void InsertCutHistory();
	QList<QPolygonF> GetCutUISubpathPolygons();

	bool IsEventAxialLineHovered() const;
	bool IsEventLateralLineHovered() const;

	bool IsValidFrontalResource() const;
	const TMJresource& GetTMJresource() const;
	const TMJfrontalResource& GetFrontalResource() const;
	const glm::vec3 GetFrontalCenter() const;

private slots:
	virtual void slotGetProfileData(const QPointF& start_pt_scene, const QPointF& end_pt_scene, std::vector<short>& data) override;
	virtual void slotGetROIData(const QPointF& start_pt_scene, const QPointF& end_pt_scene, std::vector<short>& data) override;

private:
	enum class CutEventType { NONE, DRAW, SELECT };

private:
	std::unique_ptr<ViewControllerSlice> controller_slice_;
	std::unique_ptr<GuideLineListItem> reference_lateral_line_;
	std::unique_ptr<GuideLineListItem> reference_axial_line_;
	int frontal_id_ = 0;

	TMJDirectionType direction_type_ = TMJDirectionType::TMJ_TYPE_UNKNOWN;

	bool is_lateral_visible_ = true;

	VRCutTool cut_tool_ = VRCutTool::POLYGON;
	CutEventType cut_event_type_ = CutEventType::NONE;
	std::unique_ptr<QGraphicsPolygonItem> cut_polygon_;
	std::unique_ptr<AnnotationFreedraw> cut_freedraw_;
	std::unique_ptr<QGraphicsPolygonItem> cut_selected_area_for_draw_;
	std::unique_ptr<QGraphicsPolygonItem> cut_result_display_;  // P
	bool is_cut_inside_ = false;

	std::vector<QPolygonF> cut_ui_history_;
	int cut_step_ = -1;
	int cut_stack_index_ = -1;
	bool is_over_cut_stack_ = false;
};
