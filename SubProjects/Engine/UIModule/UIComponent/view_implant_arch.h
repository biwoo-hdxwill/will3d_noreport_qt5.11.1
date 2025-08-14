#pragma once
/**=================================================================================================

Project:		UIComponent
File:			view_implant_pano_arch.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-04-13
Last modify: 	2018-04-13

	Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/
#include "base_view_pano_arch.h"
#include "uicomponent_global.h"

class RotateLineItem;
class ImplantHandle;
class ImplantData;
class CW3EllipseItem;

class UICOMPONENT_EXPORT ViewImplantArch : public BaseViewPanoArch {
	Q_OBJECT

public:
	explicit ViewImplantArch(QWidget* parent = 0);
	~ViewImplantArch();

	ViewImplantArch(const ViewImplantArch&) = delete;
	ViewImplantArch& operator=(const ViewImplantArch&) = delete;

	static constexpr float kSzSagittalImage = 60.0f; // mm단위

signals:
	void sigTranslateImplant(int implant_id, const glm::vec3& pt_vol);
	void sigRotateImplant(int implant_id, const glm::vec3& axis,
						  float delta_degree);
	void sigUpdateImplantImages(int implant_id,
								const glm::vec3& pt_vol); // translate 또는 rotate결과를 모든 뷰에 업데이트한다.
	void sigSelectImplant(int implant_id);

	void sigRotateSagittal(float angle);
	void sigRequestPanoPosition(const glm::vec3& vol_pos);

public:
	void SetReconPlane(const glm::vec3 & center_pos, const glm::vec3 & right_vector, const glm::vec3 & back_vector);
	//리소스에서 선택된 임플란트 위치를 가져와서 핸들을 갱신 해줌
	void UpdateImplantHandleAndSpec();
	void UpdateSelectedImplantHandle(const std::map<int, std::unique_ptr<ImplantData>>& implant_datas,
									 int selected_id);
	void ChangeSelectedImplantSpec();
	void DeleteImplant(int implant_id);
	void DeleteAllImplants();	

	void SetSagittalLineFromVolPos(const glm::vec3& vol_pos,
								   const glm::vec3& vol_prev,
								   const glm::vec3& vol_next);

	void RotateSagittalLine();
	glm::vec3 GetCurrentSagittalCenterPos();
	glm::mat4 GetCameraMatrix() const;
	virtual void HideAllUI(bool is_hide) override;
	virtual void HideText(bool is_hide) override;

	virtual void ApplyPreferences() override;

private slots:
	void slotTranslateImplant();
	void slotRotateImplant(float degree_angle);
	void slotUpdateImplantPos();
	void slotSyncSagittalLineStatusMovable(bool hovered);
	void slotSyncSagittalPositionUIStatusMovable(bool hovered);

private:
	virtual void mousePressEvent(QMouseEvent * event) override;
	virtual void mouseReleaseEvent(QMouseEvent *event) override;
	virtual void mouseMoveEvent(QMouseEvent *event) override;
	virtual void keyPressEvent(QKeyEvent* event) override;
	virtual void keyReleaseEvent(QKeyEvent* event) override;
	virtual void TransformItems(const QTransform& transform) override;
	virtual void SetCommonToolOnOff(const common::CommonToolTypeOnOff& type) override;

	void SetConnections();

	// -1일 경우 not hovered.
	int GetImplantHoveredID();
	void CreateImplantSpec(ImplantData* implant_data);
	QPointF GetImplantScenePosition(const int & implant_id) const;

private:
	QPointF sagittal_line_center_;
	float arch_line_angle_radian_ = 0.0f;
	std::unique_ptr<RotateLineItem> sagittal_line_;
	std::unique_ptr<CW3EllipseItem> sagittal_position_ui_;
	std::unique_ptr<ImplantHandle> implant_handle_;
};
