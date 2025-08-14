#pragma once
/**=================================================================================================
Project:		UIComponent
File:			view_pano_cross_section.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Seo Seok Man
First date:		2018-04-17
Last modify: 	2018-04-17

Copyright (c) 2017 ~ 2018 HDXWILL. All rights reserved.
 *===============================================================================================**/

#include <map>

#include "view.h"
#include "uicomponent_global.h"

class ViewControllerSlice;
class SimpleTextItem;
class GuideLineListItem;
class ImplantHandle;
class ImplantData;
class CW3EllipseItem;

class UICOMPONENT_EXPORT ViewImplantSagittal : public View 
{
	Q_OBJECT
public:
	explicit ViewImplantSagittal(QWidget* parent = 0);
	~ViewImplantSagittal();

	ViewImplantSagittal(const ViewImplantSagittal&) = delete;
	ViewImplantSagittal& operator=(const ViewImplantSagittal&) = delete;

signals:
	void sigRotateView(float delta);
	void sigSetAxialSlice(const glm::vec3& pt_vol);
	void sigGetImplantPlanePos(int implant_id, QPointF& pt_sagittal_plane);

	// hovered_implant_id == -1인 경우 not hovered..	
	void sigImplantHovered(const QPointF& pt_sagittal_plane,
						   int* hovered_implant_id) const;
	void sigTranslateImplant(int implant_id, const QPointF& pt_sagittal_plane);
	void sigRotateImplant(int implant_id, float delta_degree);
	// translate 또는 rotate결과를 모든 뷰에 업데이트한다.
	void sigUpdateImplantImages(int implant_id, const QPointF& pt_sagittal_plane);
	void sigSelectImplant(int implant_id);

public:
	virtual void SetCommonToolOnOff(const common::CommonToolTypeOnOff& type) override;
	void RenderSlice();
	virtual void HideAllUI(bool is_hide) override;
	virtual void HideText(bool is_hide) override;

	void SetAxialLine(const QPointF & axial_position_in_sagittal_plane);
	void SceneUpdate();

	void UpdateImplantHandleAndSpec();

	void UpdateSelectedImplantHandleAndInfo(const std::map<int, std::unique_ptr<ImplantData>>& implant_datas,
											int selected_id);

	void DisableImplantHandleAndSpec();

	//리소스에서 선택된 임플란트 위치를 가져와서 핸들을 갱신 해줌
	void DeleteImplant(int implant_id);
	void DeleteAllImplants();
	glm::vec3 GetUpVector();

	virtual void ApplyPreferences() override;

protected:
	virtual void wheelEvent(QWheelEvent* event) override;
	virtual void keyPressEvent(QKeyEvent* event) override;
	virtual void keyReleaseEvent(QKeyEvent* event) override;
	virtual void resizeEvent(QResizeEvent* event) override;
	virtual void mouseMoveEvent(QMouseEvent* event) override;
	virtual void mousePressEvent(QMouseEvent* event) override;
	virtual void mouseReleaseEvent(QMouseEvent* event) override;
	virtual void leaveEvent(QEvent * event) override;
	virtual void enterEvent(QEvent * event) override;

	const bool IsInSagittalPlane(const int& x, const int& y) const;

	inline ViewControllerSlice* controller_slice() const { return controller_slice_.get(); }

private:
	virtual void InitializeController() override;
	virtual bool IsReadyController() override;
	virtual void ClearGL() override;
	virtual void ActiveControllerViewEvent() override;

	virtual void drawBackground(QPainter *painter, const QRectF &rect) override;

	virtual void TransformItems(const QTransform& transform) override;

	void RenderScreen();
	void SetSagittalPlane();
	void UpdateMeasurePlaneInfo();
	void UpdateImplantInfoText(int implant_id);

	void RequestDICOMInfo(const QPointF& pt_scene);
	void DisplayDICOMInfo(glm::vec4& vol_info);

	bool IsEventAxialLineHovered() const;
	QPointF GetImplantScenePosition(const int & implant_id);

	// -1일 경우 not hovered.
	int GetImplantHoveredID(const QPointF& pt_scene) const;

	void TranslationDetailedControlImplant();
	void RotationDetailedControlImplant();
	void InputKeyClear();
	void ImplantHandleVisible();

private slots:
	virtual void slotActiveSharpenItem(const int index) override;
	virtual void slotGetProfileData(const QPointF& start_pt_scene,
									const QPointF& end_pt_scene,
									std::vector<short>& data) override;
	virtual void slotGetROIData(const QPointF& start_pt_scene,
								const QPointF& end_pt_scene,
								std::vector<short>& data) override;

	void slotTranslateImplant();
	void slotRotateImplant(float degree_angle);
	void slotUpdateImplantPos();

private:
	std::unique_ptr<ViewControllerSlice> controller_slice_;
	std::unique_ptr<GuideLineListItem> reference_axial_line_;
	std::unique_ptr<SimpleTextItem> implant_info_;
	std::unique_ptr<ImplantHandle> implant_handle_;
	std::unique_ptr<SimpleTextItem> hovered_implant_spec_;
	std::unique_ptr<CW3EllipseItem> sagittal_position_ui_;

	QPointF axial_position_in_sagittal_plane_;

	std::map<int, bool> input_key_map_;
	bool is_update_implant_ = false;
};
