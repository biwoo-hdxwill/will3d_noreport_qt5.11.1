#pragma once

/**=================================================================================================

Project:		UIComponent
File:			view_tmj_axial.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-11-13
Last modify: 	2018-11-13

		Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/
#include <memory>

#if defined(__APPLE__)
#include <glm/detail/type_vec3.hpp>
#else
#include <GL/glm/detail/type_vec3.hpp>
#endif

#include "../../Common/Common/W3Enum.h"
#include "uicomponent_global.h"
#include "view.h"

class ViewControllerSlice;
class TMJItem;

class UICOMPONENT_EXPORT ViewTMJaxial : public View
{
	Q_OBJECT

public:
	explicit ViewTMJaxial(QWidget* parent = 0);
	virtual ~ViewTMJaxial();

	ViewTMJaxial(const ViewTMJaxial&) = delete;
	ViewTMJaxial& operator=(const ViewTMJaxial&) = delete;

signals:
	void sigWindowingDone();
	void sigUpdateTMJ(const TMJDirectionType& type);
	void sigTranslateZ(const int& z_value);
	void sigROIRectCreated(const TMJDirectionType& direction_type);
	void sigRequestInitialize();

public:
	void SetHighlightLateralLine(const TMJDirectionType& type, const int& index);
	void SetVisibleTMJitemLateralLine(const TMJDirectionType& direction_type, const bool& is_visible);
	void SetVisibleTMJitemFrontalLine(const TMJDirectionType& direction_type, const bool& is_visible);
	void SetLateralParam(const TMJLateralID& id, const float& value);
	void GetLateralParam(const TMJLateralID& id, float* value) const;
	bool TranslateLateralFromWheelEvent(const TMJDirectionType& direction_type, const int& wheel_step);
	bool TranslateFrontalFromWheelEvent(const TMJDirectionType& direction_type, const int& wheel_step);
	void TranslateLateral(const TMJDirectionType& direction_type, const float& delta_vol);
	void TranslateFrontal(const TMJDirectionType& direction_type, const float& delta_vol);
	void SetTMJlateralCount(const TMJDirectionType& type, const int& count);
	void GetTMJSizeInfo(const TMJDirectionType& type, float* width, float* height);
	bool GetTMJRectCenter(const TMJDirectionType& type, glm::vec3* pt_center_in_vol);
	bool GetLateralPositionInfo(const TMJDirectionType& type, std::map<int, glm::vec3>* pt_center_in_vol, glm::vec3* up_vector_in_vol) const;
	bool GetFrontalPositionInfo(const TMJDirectionType& type, glm::vec3* pt_center_in_vol, glm::vec3* up_vector_in_vol) const;

	virtual void SetCommonToolOnOff(const common::CommonToolTypeOnOff& type) override;
	virtual void HideAllUI(bool is_hide) override;

	void InitTMJitems();
	void InitSliceRange(const float& min, const float& max, const float& value);
	void UpdateSlice();
	void DeleteROIRectUI(const TMJDirectionType& direction_type);
	void ForceRotateMatrix(const glm::mat4& mat);
	void SetSliceInVol(const float& z_pos_vol);
	void SetSliceRange(const float& min, const float& max);

	glm::vec3 GetUpVector() const;
	glm::vec3 GetCenterPosition() const;
	int GetSliceInVol() const;
	void GetSliceRange(int* min, int* max);

	void SetFrontalLineIndex(const TMJDirectionType& direction, int index);
	void SetLateralLineIndex(const TMJDirectionType& direction, int index);

protected:
	virtual void TransformItems(const QTransform& transform) override;
	virtual void resizeEvent(QResizeEvent* pEvent) override;
	virtual void mousePressEvent(QMouseEvent* event) override;
	virtual void mouseReleaseEvent(QMouseEvent* event) override;
	virtual void mouseMoveEvent(QMouseEvent* event) override;
	virtual void keyPressEvent(QKeyEvent* event) override;
	virtual void keyReleaseEvent(QKeyEvent* event) override;
	virtual void leaveEvent(QEvent* event) override;
	virtual void enterEvent(QEvent* event) override;

	inline ViewControllerSlice* controller() const { return controller_.get(); }

	void RenderSlice();
	void UpdateMeasurePlaneInfo();

public slots:
	void slotROIRectDraw(const TMJDirectionType& direction_type, bool draw_on);
	void slotTMJRectSizeChanged(const TMJRectID& roi_id, double value);

private slots:
	void slotActiveSharpenItem(const int index);
	virtual void slotChangedValueSlider(int) override;
	virtual void slotGetProfileData(const QPointF& start_pt_scene, const QPointF& end_pt_scene, std::vector<short>& data) override;
	virtual void slotGetROIData(const QPointF& start_pt_scene, const QPointF& end_pt_scene, std::vector<short>& data) override;

private:
	virtual void drawBackground(QPainter* painter, const QRectF& rect) override;
	virtual void SetGraphicsItems() override;
	virtual void InitializeController() override;
	virtual bool IsReadyController() override;
	virtual void ClearGL() override;
	virtual void ActiveControllerViewEvent() override;

	void RequestDICOMInfo(const QPointF& pt_scene);
	void DisplayDICOMInfo(const glm::vec4& vol_info);

	void SetTMJPositionAndDegree(const TMJDirectionType& type, const glm::vec3& rect_center_position, const glm::vec3& up_vector);

private:
	std::unique_ptr<ViewControllerSlice> controller_;
	std::unique_ptr<TMJItem> tmj_item_;
};
