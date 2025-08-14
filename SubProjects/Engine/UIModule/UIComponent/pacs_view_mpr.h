#pragma once
/**=================================================================================================
Project:		UIComponent
File:			pacs_view_mpr.h
Language:		C++11
Library:		Qt 5.9.9
Author:			Lim Tae Kyun
First date:		2021-06-04
Last modify: 	2021-09-29

Copyright (c) 2018 HDXWILL. All rights reserved.
 *===============================================================================================**/

#include "view.h"
#include "../../Common/Common/W3Enum.h"

class PACSViewController;
class UICOMPONENT_EXPORT PACSViewMPR : public View
{
	Q_OBJECT
public:
	explicit PACSViewMPR(QWidget* parent = 0);
	virtual ~PACSViewMPR();

	PACSViewMPR(const PACSViewMPR&) = delete;
	PACSViewMPR& operator=(const PACSViewMPR&) = delete;

signals:
	void sigCreateDCMFiles_ushort(unsigned short* data, const QString& path, const int instance_number, const int rows, const int columns);
	void sigCreateDCMFiles_uchar(unsigned char* data, const QString& path, const int instance_number, const int rows, const int columns);

public:
	virtual void SetCommonToolOnOff(const common::CommonToolTypeOnOff& type) override;

	void UpdateSlice();
	void UpdateTranslationSliderValue(const int value);
	void UpdateRotationSliderValue(const int value, bool is_rot_horizontal);
	void UpdateThickness(const int thickness, bool is_trans);
	void UpdateFilter(const int filter);
	void UpdateRotation(const bool is_rot_horizontal);

	void SetVisibleNerve(bool is_visible);
	void SetVisibleImplant(bool is_visible);

	void EmitCreateDCMFile(const int range, const bool is_translation, const bool draw_surface);

	inline void SetPlaneInfo(const glm::vec3& plane_right, const glm::vec3& plane_back, const int available_depth)
	{
		plane_right_ = plane_right;
		plane_back_ = plane_back;
		available_depth_ = available_depth;
	}

	inline void set_mpr_type(MPRViewType mpr_type) { mpr_type_ = mpr_type; };
	inline void set_is_inverse_up_vec(bool is_inverse_up_vec) { is_inverse_up_vec_ = is_inverse_up_vec; }
	inline void set_spacing_ratio(const float spacing_ratio) { spacing_ratio_ = spacing_ratio; }
	inline void set_cross_pos(const glm::vec3& cross_pos) { cross_pos_ = cross_pos;	}
	inline void set_offset(const int offset) { offset_ = offset; }
	inline void set_rotation_angle(const float radian) { rotation_angle_ = radian; }

protected:
	virtual void resizeEvent(QResizeEvent* pEvent) override;

	inline PACSViewController* controller() const { return controller_; }

	void RenderSlice();

private:
	virtual void drawBackground(QPainter* painter, const QRectF& rect) override;
	virtual void InitializeController() override;
	virtual bool IsReadyController() override;
	virtual void ClearGL() override;
	virtual void ActiveControllerViewEvent() override;

	void UpdateController();
	void UpdateRotataionController();
	void ActiveSharpen(const int index);
	void RenewalWH(const glm::vec3& right, const glm::vec3& back);
	QString CreateDicomFileMiddlePath();

private:
	MPRViewType mpr_type_ = MPRViewType::MPR_END;
	PACSViewController* controller_ = nullptr;

	glm::vec3 plane_right_ = glm::vec3(0.f);
	glm::vec3 plane_back_ = glm::vec3(0.f);
	int available_depth_ = 0;
	
	int slider_value_ = -1;
	float spacing_ratio_ = 1.f;
	bool is_rot_horizontal_ = true;

	bool is_inverse_up_vec_ = false;
	glm::vec3 cross_pos_;
	int offset_ = 0;

	int thickness_ = 1;
	float rotation_angle_ = 0.f;

	int renewal_width_ = 0;
	int renewal_height_ = 0;
};
