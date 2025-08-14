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

#include "base_view_pano_cross_section.h"
#include "uicomponent_global.h"

class ViewControllerSlice;
class SimpleTextItem;
class GuideLineListItem;
class ImplantHandle;
class ImplantData;
class CW3EllipseItem;

class UICOMPONENT_EXPORT ViewImplantCrossSection : public BaseViewPanoCrossSection {
	Q_OBJECT
public:
	explicit ViewImplantCrossSection(int cross_section_id, QWidget* parent = 0);
	~ViewImplantCrossSection();

	ViewImplantCrossSection(const ViewImplantCrossSection&) = delete;
	ViewImplantCrossSection& operator=(const ViewImplantCrossSection&) = delete;

signals:
	void sigSetImplantPosition(const int implant_id, const glm::vec3& pt_vol);
	void sigTranslateImplant(int implant_id, const glm::vec3& pt_vol);
	void sigRotateImplant(int implant_id, const glm::vec3& axis,
						  float delta_degree);
	void sigUpdateImplantImages(int implant_id,
								const glm::vec3& pt_vol); // translate 또는 rotate결과를 모든 뷰에 업데이트한다.
	void sigSelectImplant(int implant_id, int cross_section_id);
	void sigPlacedImplant();
	void sigDeleteImplant(int implant_id);

	void sigCreateDCMFiles_ushort(unsigned short* data, const QString& path, const int instance_number, const int rows, const int columns);
	void sigCreateDCMFiles_uchar(unsigned char* data, const QString& path, const int instance_number, const int rows, const int columns);

public:
	virtual void SetCommonToolOnOff(const common::CommonToolTypeOnOff& type) override;
	virtual void HideAllUI(bool is_hide) override;
	virtual void HideText(bool is_hide) override;

	void ChangeSelectedImplantSpec();
	virtual void UpdateSelectedImplantHandle(const std::map<int, std::unique_ptr<ImplantData>>& implant_datas, int selected_id) override;

	//리소스에서 선택된 임플란트 위치를 가져와서 핸들을 갱신 해줌
	void DeleteImplant(int implant_id);
	void DeleteAllImplants();

protected:
	virtual void enterEvent(QEvent* event) override;
	virtual void leaveEvent(QEvent* event) override;
	virtual void wheelEvent(QWheelEvent* event) override;
	virtual void keyPressEvent(QKeyEvent* event) override;
	virtual void keyReleaseEvent(QKeyEvent* event) override;
	virtual void mouseMoveEvent(QMouseEvent * event) override;
	virtual void mousePressEvent(QMouseEvent * event) override;
	virtual void mouseReleaseEvent(QMouseEvent * event) override;

private:
	virtual void TransformItems(const QTransform& transform) override;

	// -1일 경우 not hovered.
	int GetImplantHoveredID();

	QPointF GetImplantScenePosition(const int & implant_id);

	void TranslationDetailedControlImplant();
	void RotationDetailedControlImplant();
	void InputKeyClear();
	void ImplantHandleVisible();

private slots:
	void slotTranslateImplant();
	void slotRotateImplant(float degree_angle);
	void slotUpdateImplantPos();

private:
	std::unique_ptr<ImplantHandle> implant_handle_;

	std::map<int, bool> input_key_map_;
	bool is_update_implant_ = false;
};
