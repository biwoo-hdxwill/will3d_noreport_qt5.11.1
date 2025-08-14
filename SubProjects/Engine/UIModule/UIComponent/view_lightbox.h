#pragma once

/**=================================================================================================

Project:		UIComponent
File:			view_lightbox.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Seo Seok man
First date:		2018-10-09
Last modify: 	2018-10-09

	Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/
#include <vector>
#include <Engine/Common/Common/W3Enum.h>
#include <Engine/Common/GLfunctions/W3GLTypes.h>

#include "view.h"
#include "uicomponent_global.h"

class ViewControllerSlice;

class UICOMPONENT_EXPORT ViewLightbox : public View {
	Q_OBJECT
public:
	ViewLightbox(const LightboxViewType& type, int light_box_id, QWidget* parent = 0);
	virtual ~ViewLightbox();

	ViewLightbox(const ViewLightbox&) = delete;
	ViewLightbox& operator=(const ViewLightbox&) = delete;

signals:
	void sigTranslate(const int& lightbox_id, const int& slider_value);
	void sigSetSharpen(const SharpenLevel& level);
	void sigMaximize(const int& lightbox_id);
	/*
		void sigProcessedZoomEvent(float scene_scale);
		void sigProcessedPanEvent(const QPointF& gl_translate);
		void sigProcessedLightEvent();
		가 mouse move마다 날아가게 구현되어 있어서
		마우스를 놓았을 때 sync 하는 형식으로 구현하기 위해 signal을 별도로 선언함
	*/
	void sigLightboxWindowingDone();
	void sigLightboxZoomDone(const float& scene_scale);
	void sigLightboxPanDone(const QPointF& trans);

	void sigDisplayDICOMInfo(const int& lightbox_id,
							 const QPointF& pt_lightbox_plane,
							 glm::vec4& vol_info);
	void sigGetProfileData(const int& lightbox_id,
						   const QPointF& start_pt_plane,
						   const QPointF& end_pt_plane,
						   std::vector<short>& data);
	void sigGetROIData(const int& lightbox_id,
					   const QPointF& start_pt_plane,
					   const QPointF& end_pt_plane,
					   std::vector<short>& data);

	void sigMeasureCreated(const int& lightbox_id,
						   const unsigned int& measure_id);
	void sigMeasureDeleted(const int& lightbox_id,
						   const unsigned int& measure_id);
	void sigMeasureModified(const int& lightbox_id,
							const unsigned int& measure_id);

	void sigCreateDCMFiles_ushort(unsigned short* data, const QString& path, const int instance_number, const int rows, const int columns);
	void sigCreateDCMFiles_uchar(unsigned char* data, const QString& path, const int instance_number, const int rows, const int columns);

public:
	void UpdateLightbox();
	void RenderAndUpdate();
	void InitSlider(int min, int max, int value);
	void SetSliderValue(int value);
	void SetEnabledSharpenUI(const bool& is_enabled);
	void SetSharpenLevel(const SharpenLevel& sharpen_level);

	glm::vec3 GetUpVector() const;
	glm::vec3 GetCenterPosition() const;

	void SetVisibleNerve(bool is_visible);
	void SetVisibleImplant(bool is_visible);

#ifndef WILL3D_VIEWER
	void RequestedCreateDCMFiles(const QString& time, const bool nerve_visible, const bool implant_visible, const int filter, const int thickness);
	const int GetFilterValue() const;
#endif

private slots:
	virtual void slotActiveSharpenItem(const int index) override;
	virtual void slotChangedValueSlider(int) override;
	virtual void slotGetProfileData(const QPointF& start_pt_scene,
									const QPointF& end_pt_scene,
									std::vector<short>& data) override;
	virtual void slotGetROIData(const QPointF& start_pt_scene,
								const QPointF& end_pt_scene,
								std::vector<short>& data) override;

private:
	virtual void InitializeController() override;
	virtual bool IsReadyController() override;
	virtual void ClearGL() override;
	virtual void ActiveControllerViewEvent() override;
	virtual void drawBackground(QPainter *painter, const QRectF &rect) override;

	virtual void leaveEvent(QEvent * event) override;
	virtual void enterEvent(QEvent * event) override;
	virtual void resizeEvent(QResizeEvent *pEvent) override;
	virtual void mouseMoveEvent(QMouseEvent * event) override;
	virtual void mouseReleaseEvent(QMouseEvent *event) override;
	virtual void mouseDoubleClickEvent(QMouseEvent* event) override;

	void RenderSlice();
	void RenderScreen();
	void SetLightboxPlane();
	void UpdateMeasurePlaneInfo();

	void RequestDICOMInfo(const QPointF& pt_scene);
	void DisplayDICOMInfo(const glm::vec4& vol_info);
	void RenewalWH(int& out_width, int& out_height);

private:
	std::unique_ptr<ViewControllerSlice> controller_slice_;
	int lightbox_id_ = -1;
	LightboxViewType view_type_ = LightboxViewType::VIEW_TYPE_UNKNOWN;
};
