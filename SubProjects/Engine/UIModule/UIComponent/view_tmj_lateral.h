#pragma once
/**=================================================================================================

Project:		UIComponent
File:			view_tmj_lateral.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-12-03
Last modify: 	2018-12-03

		Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/
#include "../../Common/Common/W3Enum.h"
#include "uicomponent_global.h"
#include "view.h"

class ViewControllerSlice;
class CW3TextItem;
class GuideLineListItem;
class TMJlateralResource;
class TMJresource;

enum SharpenLevel;

class UICOMPONENT_EXPORT ViewTmjLateral : public View
{
	Q_OBJECT
public:
	ViewTmjLateral(int lateral_id, const common::ViewTypeID& view_type, TMJDirectionType type, QWidget* parent = 0);
	virtual ~ViewTmjLateral();

	ViewTmjLateral(const ViewTmjLateral&) = delete;
	ViewTmjLateral& operator=(const ViewTmjLateral&) = delete;

signals:
	void sigWindowingDone();
	void sigZoomDone(const float& scene_scale);
	void sigPanDone(const QPointF& trans);

	void sigSetSharpen(SharpenLevel level);
	void sigLateralViewSelect(const TMJDirectionType& direction_type, const int& lateral_id);
	void sigWheelEvent(const int& wheel_step);
	void sigSetFrontalSlice(const glm::vec3& pt_vol);
	void sigSetAxialSlice(const glm::vec3& pt_vol);

	void sigMeasureCreated(const TMJDirectionType& direction_type, const int& lateral_id, const unsigned int& measure_id);
	void sigMeasureDeleted(const TMJDirectionType& direction_type, const int& lateral_id, const unsigned int& measure_id);
	void sigMeasureModified(const TMJDirectionType& direction_type, const int& lateral_id, const unsigned int& measure_id);
	void sigMeasureResourceUpdateNeeded(const int& lateral_id, bool& is_need_update);

public:
	void SetEnabledSharpenUI(const bool& is_enabled);
	void SetSharpen(const SharpenLevel& level);
	void SetSelectedStatus(const bool& selected);
	void UpdateThickness();
	void UpdateUI();
	void UpdateLateral();

	virtual void HideAllUI(bool is_hide) override;
	virtual void SetCommonToolOnOff(const common::CommonToolTypeOnOff& type) override;
	
	inline const TMJDirectionType& direction_type() const noexcept { return direction_type_; }

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

	bool IsInLateralPlane(const int& x, const int& y) const;
	void RenderSlice();

	inline int lateral_id() const { return lateral_id_; }
	inline ViewControllerSlice* controller_slice() const { return controller_slice_.get(); }

private:
	virtual void SetGraphicsItems() override;
	virtual void InitializeController() override;
	virtual bool IsReadyController() override;
	virtual void ClearGL() override;
	virtual void ActiveControllerViewEvent() override;

	virtual void drawBackground(QPainter* painter, const QRectF& rect) override;

	void RenderScreen();
	void SetLateralPlane();

	void UpdateMeasurePlaneInfo();
	void RequestDICOMInfo(const QPointF& pt_scene);
	void DisplayDICOMInfo(const glm::vec4& vol_info);
	void SetTMJUISelectedStatus();

	bool IsEventAxialLineHovered() const;
	bool IsEventFrontalLineHovered() const;
	bool IsValidLateralResource() const;
	const TMJlateralResource& GetLateralResource() const;
	const TMJresource& GetTMJresource() const;

private slots:
	virtual void slotGetProfileData(const QPointF& start_pt_scene, const QPointF& end_pt_scene, std::vector<short>& data) override;
	virtual void slotGetROIData(const QPointF& start_pt_scene, const QPointF& end_pt_scene, std::vector<short>& data) override;

private:
	std::unique_ptr<ViewControllerSlice> controller_slice_;
	std::unique_ptr<CW3TextItem> text_id_;
	std::unique_ptr<GuideLineListItem> reference_frontal_line_;
	std::unique_ptr<GuideLineListItem> reference_axial_line_;

	int lateral_id_ = -1;
	bool is_selected_ = false;
	TMJDirectionType direction_type_ = TMJDirectionType::TMJ_TYPE_UNKNOWN;
};
