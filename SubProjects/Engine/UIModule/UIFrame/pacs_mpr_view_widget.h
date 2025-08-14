#pragma once
/*=========================================================================
File:			pacs_mpr_view_widget.h
Language:		C++11
Library:        Qt 5.9.9
Author:			Lim Tae Kyun
First date:		2021-07-16
Last modify:	2021-09-29

Copyright (c) 2018 HDXWILL. All rights reserved.
=========================================================================*/

#include <QWidget>
#include <GL/glm/vec3.hpp>
#include "../../Common/Common/W3Enum.h"

#include "uiframe_global.h"

class QVBoxLayout;
class QLabel;

class PACSViewMPR;
class CW3DicomLoaderScrollbar;

namespace
{
	//const int kViewCnt = 3;
}

class UIFRAME_EXPORT PacsMPRViewWidget : public QWidget
{
	Q_OBJECT
public:
	explicit PacsMPRViewWidget(QWidget* parent = 0);
	virtual ~PacsMPRViewWidget();

	PacsMPRViewWidget(const PacsMPRViewWidget&) = delete;
	const PacsMPRViewWidget& operator=(const PacsMPRViewWidget&) = delete;

	void SetMPRPlaneInfo(MPRViewType mpr_type, const glm::vec3& plane_right, const glm::vec3& plane_back, const int available_depth);
	void SetCrossPos(const glm::vec3& cross_pos);

	void EmitCreateDCMFile();

	inline void SetSurfaceVisible(bool nerve, bool implant) { draw_nerve_ = nerve; draw_implant_ = implant; }

public slots:
	void slotSliderTypeChange();					//default translation
	void slotRotationTypeChange();					//default horizontal
	void slotNerveVisibility(bool is_visible);
	void slotImplantVisibility(bool is_visible);
	void slotUpdateInterval(const int interval);
	void slotUpdateAngle(const int angle);			//degree
	void slotUpdateThickness(const int thickness);
	void slotUpdateFilter(const int filter);

signals:
	void sigCreateDCMFiles_uchar(unsigned char* data, const QString& path, const int instance_number, const int rows, const int columns);
	void sigCreateDCMFiles_ushort(unsigned short* data, const QString& path, const int instance_number, const int rows, const int columns);

private slots:
	void slotSliderValueChange(int index, float value);

private:
	void Initialize();
	void SetLayout();
	void Connections();

	QVBoxLayout* CreateMPRContentsLayout();

	void SetTranslationSliderRange();
	void SetRotationSliderRange();
	void SetTextImageCntLabel();

private:
	QVBoxLayout* main_layout_ = nullptr;
	PACSViewMPR* pacs_view_mpr_[3] = { nullptr, };
	CW3DicomLoaderScrollbar* slider_ = nullptr;
	QLabel* label_image_cnt_ = nullptr;

	MPRViewType mpr_view_type_ = MPRViewType::MPR_UNKNOWN;
	bool is_translation_ = true;
	bool is_rot_horizontal_ = true;

	glm::vec3 plane_right_ = glm::vec3(0.f);
	glm::vec3 plane_back_ = glm::vec3(0.f);
	int available_depth_ = 0;

	glm::vec3 cross_pos_ = glm::vec3(-1.f);

	int slider_maxinum_ = 0;
	int interval_ = 1;
	int angle_ = 1;
	
	bool draw_nerve_ = false;
	bool draw_implant_ = false;
};
