#pragma once
/**=================================================================================================
Project:		UIComponent
File:			view_pano_cross_section.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2017-12-29
Last modify: 	2017-12-29
Copyright (c) 2017 HDXWILL. All rights reserved.
 *===============================================================================================**/

#include "uicomponent_global.h"
#include "view.h"

enum SharpenLevel;

class ViewControllerSlice;
class CW3TextItem;
class GuideLineListItem;
class ImplantTextItem;
class ImplantData;
class UICOMPONENT_EXPORT BaseViewPanoCrossSection : public View
{
	Q_OBJECT
public:
	BaseViewPanoCrossSection(int cross_section_id, QWidget* parent = 0);
	virtual ~BaseViewPanoCrossSection();

	BaseViewPanoCrossSection(const BaseViewPanoCrossSection&) = delete;
	BaseViewPanoCrossSection& operator=(const BaseViewPanoCrossSection&) = delete;

signals:
	void sigSetSharpen(SharpenLevel level);
	void sigHoverView(bool is_hovered);
	void sigWheelDelta(int delta);
	void sigSetAxialSlice(const int& cross_section_id, const glm::vec3& pt_vol);

	void sigImplantHovered(const int& cross_section_id, const QPointF& pt_cs_plane, int& hovered_implant_id, QPointF& implant_pos_in_scene);
	void sigMeasureCreated(const int& cross_section_id, const unsigned int& measure_id);
	void sigMeasureDeleted(const int& cross_section_id, const unsigned int& measure_id);
	void sigMeasureModified(const int& cross_section_id, const unsigned int& measure_id);
	void sigMeasureResourceUpdateNeeded(const int& cross_section_id, bool& is_need_update);
	void sigMaximize();

public:
	void UpdateImplantHandleAndSpec();
	void SetEnabledSharpenUI(const bool& is_enabled);
	void SetSharpen(const SharpenLevel& level);
	void SetPanoThickness(float thickness_mm);
	void SetAxialLine(const QPointF& axial_position_in_cross_plane);
	glm::vec3 GetUpVector() const;

	virtual void UpdateCrossSection();
	virtual void HideAllUI(bool is_hide) override;
	virtual void ApplyPreferences() override;

	void MoveAxialLineToViewCenter();
	inline bool maximize() { return maximize_; }

protected slots:
	virtual void slotActiveSharpenItem(const int index) override;

protected:
	virtual void TransformItems(const QTransform& transform) override;
	virtual void leaveEvent(QEvent* event) override;
	virtual void enterEvent(QEvent* event) override;
	virtual void wheelEvent(QWheelEvent* event) override;
	virtual void keyPressEvent(QKeyEvent* event) override;
	virtual void resizeEvent(QResizeEvent* pEvent) override;
	virtual void mouseMoveEvent(QMouseEvent* event) override;
	virtual void mousePressEvent(QMouseEvent* event) override;
	virtual void mouseReleaseEvent(QMouseEvent* event) override;
	virtual void mouseDoubleClickEvent(QMouseEvent* event) override;

	QPointF GetImplantSpecPosition(const int& implant_id, const bool& selected) const;
	void UpdateImplantSpecPosition();
	void DeleteImplantSpec(const int& implant_id);
	void DeleteImplantSpecs();
	bool IsInCrossSectionPlane(const int& x, const int& y) const;
	void RenderSlice();

	virtual void UpdateImplantHandlePosition() {}
	virtual void SetCommonToolOnOff(const common::CommonToolTypeOnOff& type) override;

	inline int cross_section_id() const { return cross_section_id_; }
	inline ViewControllerSlice* controller_slice() const { return controller_slice_.get(); }
	inline std::map<int, std::unique_ptr<ImplantTextItem>>& implant_specs() { return implant_specs_; }

private:
	virtual void SetGraphicsItems() override;
	virtual void InitializeController() override;
	virtual bool IsReadyController() override;
	virtual void ClearGL() override;
	virtual void ActiveControllerViewEvent() override;

	virtual void drawBackground(QPainter* painter, const QRectF& rect) override;

	void CreateImplantSpec(ImplantData* implant_data);
	virtual void UpdateSelectedImplantHandle(const std::map<int, std::unique_ptr<ImplantData>>& implant_datas, int selected_id) {};
	void RenderScreen();
	void SetCrossSectionPlane();

	void UpdateMeasurePlaneInfo();

	void RequestDICOMInfo(const QPointF& pt_scene);
	void DisplayDICOMInfo(const glm::vec4& vol_info);

	bool IsEventAxialLineHovered() const;

private slots:
	virtual void slotGetProfileData(const QPointF& start_pt_scene, const QPointF& end_pt_scene, std::vector<short>& data) override;
	virtual void slotGetROIData(const QPointF& start_pt_scene, const QPointF& end_pt_scene, std::vector<short>& data) override;

	void slotMeasureCreated(const unsigned int& measure_id);
	void slotMeasureDeleted(const unsigned int& measure_id);
	void slotMeasureModified(const unsigned int& measure_id);

private:
	std::unique_ptr<ViewControllerSlice> controller_slice_;
	std::unique_ptr<CW3TextItem> text_id_;
	// 동작 주체가 이 뷰가 아니기 때문에 reference line임.
	std::unique_ptr<GuideLineListItem> reference_pano_line_;
	std::unique_ptr<GuideLineListItem> reference_axial_line_;
	std::map<int, std::unique_ptr<ImplantTextItem>> implant_specs_;

	int cross_section_id_ = -1;

	bool maximize_ = false;
};
