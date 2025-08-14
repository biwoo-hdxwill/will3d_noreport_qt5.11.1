#pragma once

/**=================================================================================================

Project:		UIComponent
File:			base_view_pano.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-04-10
Last modify: 	2018-04-10

	Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/
#include <vector>

#if defined(__APPLE__)
#include <glm/detail/type_vec.hpp>
#else
#include <GL/glm/detail/type_vec.hpp>
#endif

#include <QMenu>
#include "../../Common/Common/W3Enum.h"
#include "../UIViewController/view_render_param.h"
#include "view_3d.h"

class BaseViewController;
class ViewControllerImage;
class ViewControllerPano3D;
class ViewControllerImplant3Dpano;
class GuideLineListItem;
class PanoRulerItem;
class ImplantTextItem;
class ImplantData;
class Measure3DManager;
#ifndef WILL3D_VIEWER
class ProjectIOView;
#endif

class UICOMPONENT_EXPORT BaseViewPano : public View3D {
	Q_OBJECT
public:
	explicit BaseViewPano(QWidget* parent = 0);
	virtual ~BaseViewPano();

	BaseViewPano(const BaseViewPano&) = delete;
	BaseViewPano& operator=(const BaseViewPano&) = delete;

#ifndef WILL3D_VIEWER
	void ExportProjectForMeasure3D(ProjectIOView& out);
	void ImportProjectForMeasure3D(ProjectIOView& in);
#endif

	enum ReconType {
		RECON_MPR = 0,
		RECON_XRAY,
		RECON_3D,
		RECON_TYPE_END
	};

signals:
	void sigReconTypeChanged();
	void sigTranslateZ(float value);
	void sigReconPanoResource();
	void sigTranslatedCrossSection(const float& trans_pano_plane);
	void sigSetAxialSlice(const QPointF& pt_pano_plane);
	void sigRotateCrossSection(float delta_angle);

	void sigDisplayDICOMInfo(const QPointF& pt_pano_plane,
							 glm::vec4& vol_info);
	void sigGetProfileData(const QPointF& start_pt,
						   const QPointF& end_pt,
						   std::vector<short>& data);
	void sigGetROIData(const QPointF& start_pt,
					   const QPointF& end_pt,
					   std::vector<short>& data);
	void sigCrossSectionLineReleased();
	void sigToggleImplantRenderingType();

public:
	void UpdateVR(bool is_high_quality);
	void SetCliping(const std::vector<glm::vec4>& planes, bool is_enable);
	virtual void SetCommonToolOnOff(const common::CommonToolTypeOnOff& type) override;

	void SetHighlightCrossSection(int cross_id, bool is_highlight);
	void ClearCrossSectionLine();
	void CrossSectionUpdated();
	void SetAxialLine(const QPointF& axial_position_in_pano_plane);
	void SetArchLine(const QPointF & arch_position_in_pano_plane);
	void SetEnableSlider(bool is_enable);
	void SetSliceRange(float min, float max);
	void SetSliceNum(int num);
	int GetSliceNum() const;
	void UpdatedPano();
	
	virtual void HideAllUI(bool is_hide) override;

	virtual void UpdateImplantHandleAndSpec() = 0;

	const ReconType& recon_type() const { return recon_type_; }

	virtual BaseViewController3D* controller_3d() override;

	virtual void DeleteImplant(int implant_id);
	virtual void DeleteAllImplants();

	virtual void ApplyPreferences() override;

	void MoveAxialLineToViewCenter();

protected slots:
	virtual void slotActiveSharpenItem(const int index) override;
	virtual void slotActiveFilteredItem(const QString& text) override;

protected:
	enum ControllerType {
		CTRL_IMAGE,
		CTRL_3D,
		CONTROLLER_TYPE_END
	};

	void UpdateNotSelectedImplantSpec(const std::map<int, std::unique_ptr<ImplantData>>& implant_datas,
									  int selected_id, int add_implant_id);
	void UpdateSelectedImplantSpec(const std::map<int, std::unique_ptr<ImplantData>>& implant_datas,
									  int selected_id);

	virtual void ActiveControllerViewEvent() override;
	virtual void TransformItems(const QTransform& transform) override;

	virtual void resizeEvent(QResizeEvent *pEvent) override;
	virtual void mouseMoveEvent(QMouseEvent * event) override;
	virtual void mousePressEvent(QMouseEvent * event) override;
	virtual void mouseReleaseEvent(QMouseEvent * event) override;
	virtual void mouseDoubleClickEvent(QMouseEvent * event) override;
	virtual void wheelEvent(QWheelEvent * event) override;
	virtual void keyPressEvent(QKeyEvent* event) override;
	virtual void SetWorldAxisDirection() override;

	virtual void leaveEvent(QEvent * event) override;
	virtual void enterEvent(QEvent * event) override;

	virtual void ResetView() override;

	bool IsEventRotate3D() const;
	bool IsEventCrossSectionHovered() const;
	bool IsEventAxialLineHovered() const;

	void SetAxialLineItem(const QPointF& pt_scene);
	void SetArchLineItem(const QPointF& pt_scene);

	void CreateImplantSpec(ImplantData* implant_data);

	inline ViewControllerImage* controller_image() const { return controller_image_.get(); }
	inline ViewControllerPano3D* controller_3d() const { return controller_3d_.get(); }
	inline GuideLineListItem* cross_line() const { return cross_line_.get(); }
	inline GuideLineListItem* axial_line() const { return reference_axial_line_.get(); }

	inline bool is_pressed_double_click() const { return is_pressed_double_click_; }
	inline const QString& filtered_texts(ReconType recon_type) const { return filtered_texts_[recon_type]; }

	inline const ControllerType& ctrl_type() const { return ctrl_type_; }
	inline std::map<int, std::unique_ptr<ImplantTextItem>>& implant_specs() { return implant_specs_; }
	
	QPointF GetImplantSpecPosition(const int& implant_id, const bool& selected) const;

	virtual void UpdateImplantHandlePosition() {}

	void SetControllerImage(ViewControllerImage* controller);
	void SetController3D(ViewControllerPano3D* controller);
	void SetController3D(const std::shared_ptr<ViewControllerImplant3Dpano>& controller);

	virtual void RenderPanoVolume();

	virtual void HideMeasure(bool toggled) override;
	virtual void DeleteAllMeasure() override;
	virtual void DeleteUnfinishedMeasure() override;

	virtual void RotateOneDegree(RotateOneDegreeDirection direction) override;

private:
	virtual void SetGraphicsItems() override;
	virtual void InitializeController() override;
	virtual bool IsReadyController() override;
	virtual void ClearGL() override;
	virtual void drawBackground(QPainter *painter, const QRectF &rect) override;

	void MouseCrossSectionRotateEvent();

	void SetFilteredItems();
	void SetPanoImage();
	void SetPanoRuler();
	void SetVisibleLineItems(bool visible);
	void RenderScreen();
	void UpdateMeasurePlaneInfo();

	BaseViewController& GetCurrentController() const;
	void RestoreViewRenderParamByChangeController(ControllerType prev_ctrl_type,
												  ControllerType curr_ctrl_type);

	void RequestDICOMInfo(const QPointF& pt_scene);
	void DisplayDICOMInfo(const glm::vec4& vol_info);

	QPointF ClampImagePosition(const QPointF & image_pos);

	glm::vec3 VolumeToGLVertex(glm::vec3 volume_pos);

private slots:
	virtual void slotChangedValueSlider(int) override;
	virtual void slotRotateMatrix(const glm::mat4 & mat) override;
	void slotTranslateCrossSection(const QPointF& trans);
	virtual void slotGetProfileData(const QPointF& start_pt_scene,
									const QPointF& end_pt_scene,
									std::vector<short>& data) override;
	virtual void slotGetROIData(const QPointF& start_pt_scene,
								const QPointF& end_pt_scene,
								std::vector<short>& data) override;

protected:
	Measure3DManager* measure_3d_manager_ = nullptr;

private:
	std::unique_ptr<ViewControllerImage> controller_image_;
	std::shared_ptr<ViewControllerPano3D> controller_3d_;
	std::unique_ptr<GuideLineListItem> cross_line_;
	// 동작 주체가 이 뷰가 아니기 때문에 reference line임. cross_line_ 과 동작방식 차이를 참조
	std::unique_ptr<GuideLineListItem> reference_axial_line_;
	std::unique_ptr<GuideLineListItem> reference_arch_line_;
	std::unique_ptr<PanoRulerItem> pano_ruler_;

	bool is_pressed_double_click_ = false;

	QString filtered_texts_[RECON_TYPE_END];
	ReconType recon_type_ = RECON_MPR;
	ControllerType ctrl_type_ = CTRL_IMAGE;

	std::unique_ptr<QMenu> menu_nerve_ell_;
	std::unique_ptr<QMenu> menu_nerve_spl_;

	std::unique_ptr<QAction> menu_act_delete_nerve_;
	std::unique_ptr<QAction> menu_act_remove_pnt_nerve_;
	std::unique_ptr<QAction> menu_act_insert_pnt_nerve_;

	std::map<int, std::unique_ptr<ImplantTextItem>> implant_specs_;
	ViewRenderParam prev_view_render_param_[CONTROLLER_TYPE_END];
	int id_highlighted_cross_line_ = -1;
};
