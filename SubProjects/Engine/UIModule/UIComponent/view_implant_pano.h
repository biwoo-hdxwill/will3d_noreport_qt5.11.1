#pragma once
/**=================================================================================================

Project:		UIComponent
File:			view_implant_pano.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-04-13
Last modify: 	2018-04-13

	Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/
#include <map>
#include "base_view_pano.h"
#include "uicomponent_global.h"

class QMenu;
class QAction;
class ImplantHandle;
class ImplantData;
class ViewControllerImplant3Dpano;
class UICOMPONENT_EXPORT ViewImplantPano : public BaseViewPano 
{
	Q_OBJECT
public:
	explicit ViewImplantPano(QWidget* parent = 0);
	~ViewImplantPano();

	ViewImplantPano(const ViewImplantPano&) = delete;
	ViewImplantPano& operator=(const ViewImplantPano&) = delete;

public:
	//리소스에서 선택된 임플란트를 가져와서 핸들을 갱신 해줌
	virtual void UpdateImplantHandleAndSpec() override;
	void ChangeSelectedImplantSpec();

	virtual void DeleteImplant(int implant_id) override;
	virtual void DeleteAllImplants() override;

	// Bone density view에서 현재 보고 있는 뷰와 일치시키기 위함
	glm::mat4 GetCameraMatrix() const;

	bool tmpIsRender2D();

	virtual void HideAllUI(bool is_hide) override;
	virtual void HideText(bool is_hide) override;

signals:
	void sigTranslateImplant(int implant_id, const QPointF& pt_pano_plane);
	void sigRotateImplant(int implant_id, float delta_degree);
	void sigTranslateImplantIn3D(const int& implant_id, const glm::vec3& delta_trans);
	void sigRotateImplantIn3D(const int& implant_id, const glm::vec3& rotate_axes, const float& delta_degree);

	void sigUpdateImplantImages(int implant_id, const QPointF& pt_pano_plane); // translate 또는 rotate결과를 모든 뷰에 업데이트한다.
	void sigUpdateImplantImagesIn3D(int implant_id); // translate 또는 rotate결과를 모든 뷰에 업데이트한다.
	void sigSelectImplant(int implant_id);
	void sigPlacedImplant(); // place and selection
	void sigImplantHovered(const QPointF& pt_pano_plane, int* hovered_implant_id) const; // hovered_implant_id == -1인 경우 not hovered..	
	void sigDeleteImplant(int implant_id);

protected:
	//virtual void keyPressEvent(QKeyEvent* event) override;

private slots:
	void slotTranslateImplant();
	void slotRotateImplant(float degree_angle);
	void slotUpdateImplantPos();
	void slotDeleteImplant();

private:
	void InitializeImplantMenu();
	void UpdateSelectedImplantHandle(const std::map<int, std::unique_ptr<ImplantData>>& implant_datas,
		int selected_id);
	virtual void TransformItems(const QTransform& transform) override;
	virtual void SetCommonToolOnOff(const common::CommonToolTypeOnOff& type) override;

	virtual void mouseMoveEvent(QMouseEvent * event) override;
	virtual void mousePressEvent(QMouseEvent * event) override;
	virtual void mouseReleaseEvent(QMouseEvent * event) override;
	virtual void keyPressEvent(QKeyEvent* event) override;
	virtual void keyReleaseEvent(QKeyEvent* event) override;
	virtual void leaveEvent(QEvent* event) override;
	virtual void enterEvent(QEvent* event) override;

	void PickAndMoveImplant();
	virtual void RenderPanoVolume() override;
	bool IsEventImplantSelected() const;

	void SelectImplantIfRecon3D();

	// -1일 경우 not hovered.
	int GetImplantHoveredID(const QPointF& pt_scene) const;

	void TranslationDetailedControlImplant();
	void RotationDetailedControlImplant();
	void InputKeyClear();

private:
	std::shared_ptr<ViewControllerImplant3Dpano> controller_implant_3d_;
	std::unique_ptr<ImplantHandle> implant_handle_;
	std::unique_ptr<QMenu> menu_implant_;
	std::unique_ptr<QAction> act_delete_implant_;

	std::map<int, bool> input_key_map_;
	bool is_update_implant_ = false;
};
