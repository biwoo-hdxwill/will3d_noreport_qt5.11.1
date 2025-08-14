#pragma once
/*=========================================================================
File:			pacs_mpr_setting_widget.h
Language:		C++11
Library:        Qt 5.9.9
Author:			Lim Tae Kyun
First date:		2021-07-16
Last modify:	2021-09-29

Copyright (c) 2018 HDXWILL. All rights reserved.
=========================================================================*/

#include <QWidget>

#include "uiframe_global.h"

namespace
{
	enum class PACS_MPR_SliderID
	{
		TRANS_INTERVAL,
		TRANS_THICKNESS,
		TRANS_FILTER,
		ROT_ANGLE,
		ROT_THICKNESS,
		ROT_FILTER,
		END
	};

	enum class PACS_Rotation_Type
	{
		HORIZONTAL,
		VERTICAL,
		END
	};

	const int kMPRSliderSize = static_cast<int>(PACS_MPR_SliderID::END);
}

class QVBoxLayout;
class QTabWidget;
class QSlider;
class UIFRAME_EXPORT PacsMPRSettingWidget : public QWidget
{
	Q_OBJECT

public:
	explicit PacsMPRSettingWidget(QWidget* parent = 0);
	virtual ~PacsMPRSettingWidget();

	PacsMPRSettingWidget(const PacsMPRSettingWidget&) = delete;
	const PacsMPRSettingWidget& operator=(const PacsMPRSettingWidget&) = delete;

signals:
	void sigSliderTypeChange();		//default translation
	void sigRotationTypeChange();	//default horizontal
	void sigUpdateInterval(const int interval);
	void sigUpdateAngle(const int angle);
	void sigUpdateThickness(const int thickness);
	void sigUpdateFilter(const int filter);

private slots:
	void slotRotationTypeChange(int id);

private:
	void Initialize();
	void SetLayout();

	QTabWidget* CreateMPRSettingTabWidget();

	void EmitUpdateMPRSlider(const int id);
	void CreateMPRSliderLayouts();

private:
	QVBoxLayout* main_layout_ = nullptr;

	QVBoxLayout* mpr_slider_layouts_[kMPRSliderSize] = { nullptr, };
	QSlider* mpr_sliders_[kMPRSliderSize] = { nullptr, };
	int prev_slider_value_[kMPRSliderSize] = { 0, };

	int radio_check_id_ = 0;
};
