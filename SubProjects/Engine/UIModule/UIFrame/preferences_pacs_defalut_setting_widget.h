#pragma once
/*=========================================================================
File:			preferences_pacs_defalut_setting_widget.h
Language:		C++11
Library:        Qt 5.9.9
Author:			Lim Tae Kyun
First date:		2021-09-07
Last modify:	2021-09-29

Copyright (c) 2018 HDXWILL. All rights reserved.
=========================================================================*/

#include "base_widget.h"

class QCheckBox;
class PreferencesPACSDefaultSettingWidget : public BaseWidget
{
	Q_OBJECT
public:
	explicit PreferencesPACSDefaultSettingWidget(QWidget *parent = nullptr);
	virtual ~PreferencesPACSDefaultSettingWidget();

	PreferencesPACSDefaultSettingWidget(const PreferencesPACSDefaultSettingWidget&) = delete;
	const PreferencesPACSDefaultSettingWidget& operator = (const PreferencesPACSDefaultSettingWidget&) = delete;

	void SetGlobalPreferences();

private:
	virtual void Reset() override;

	QVBoxLayout* CreateMainContentsLayout();
	QVBoxLayout* CreatePACSDefaultSettingLayout();

	QVBoxLayout* CreateMPRDefaultSetting2D();
	QVBoxLayout* CreateMPRDefaultSetting3D();

	void GetPACSValues();

private:
	//MPR 2D
	QCheckBox* view_list_swap_check_ = nullptr;
	QRadioButton* slice_rotate_type_horizontal_radio_ = nullptr;
	QRadioButton* slice_rotate_type_vertical_radio_ = nullptr;
	QSpinBox* slice_interval_spin_ = nullptr;
	QSpinBox* slice_angel_spin_ = nullptr;
	QSpinBox* slice_thickness_spin_ = nullptr;
	QSpinBox* slice_filter_spin_ = nullptr;

	//MPR 3D
	QRadioButton* volume_rotate_dir_Anterior_radio_ = nullptr;
	QRadioButton* volume_rotate_dir_Posterior_radio_ = nullptr;
	QRadioButton* volume_rotate_type_horizontal_radio_ = nullptr;
	QRadioButton* volume_slice_rotate_type_vertical_radio_ = nullptr;
	QSpinBox* volume_angel_spin_ = nullptr;

	//Panorama
};
