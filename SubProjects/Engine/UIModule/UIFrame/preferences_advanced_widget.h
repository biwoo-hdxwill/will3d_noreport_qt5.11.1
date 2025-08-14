#pragma once
/*=========================================================================
File:			preferences_advanced_widget.h
Language:		C++11
Library:        Qt 5.9.9
Author:			Lim Tae Kyun
First date:		2021-09-06
Last modify:	2021-09-06

Copyright (c) 2018 HDXWILL. All rights reserved.
=========================================================================*/

#include "base_widget.h"

class QCheckBox;
class UIFRAME_EXPORT PreferencesAdvancedWidget : public BaseWidget
{
	Q_OBJECT
public:
	explicit PreferencesAdvancedWidget(QWidget *parent = nullptr);
	virtual ~PreferencesAdvancedWidget();

	PreferencesAdvancedWidget(const PreferencesAdvancedWidget&) = delete;
	const PreferencesAdvancedWidget& operator = (const PreferencesAdvancedWidget&) = delete;

	void SetGlobalPreferences();	

private slots:

private:
	virtual void Reset() override;

	QVBoxLayout* CreateAdvancedLayout();
	QVBoxLayout* CreateMPRLayout();
	QVBoxLayout* CreateCrossSectionLayout();
	QVBoxLayout* CreatePanoramaLayout();
	QVBoxLayout* CreateImplantLayout();
	QVBoxLayout* CreateVolumeRenderingLayout();
	QVBoxLayout* CreateWindowingLayout();

	void GetAdvancedValues();

private:
	QRadioButton* mpr_reorientation_auto_radio_ = nullptr;
	QRadioButton* mpr_reorientation_manual_radio_ = nullptr;
	QDoubleSpinBox* mpr_default_interval_spin_ = nullptr;
	QCheckBox* mpr_hide_mpr_views_on_maximized_vr_layout_check_ = nullptr;
	
	QDoubleSpinBox* cross_section_view_thickness_increments_spin_ = nullptr;
	QDoubleSpinBox* cross_section_view_interval_increments_spin_ = nullptr;
	QDoubleSpinBox* cross_section_view_default_interval_spin_ = nullptr;
	QCheckBox* cross_section_view_flip_slices_across_the_arch_centerline_check_ = nullptr;
	
	QDoubleSpinBox* panorama_default_thickness_spin_ = nullptr;
	QDoubleSpinBox* panorama_axial_view_arch_thickness_increments_spin_ = nullptr;
	QDoubleSpinBox* panorama_default_range_spin_ = nullptr;

	QSpinBox* implant_translation_increments_spin_ = nullptr;
	QSpinBox* implant_rotation_increments_spin_ = nullptr;

	QComboBox* volume_rendering_quality_combo_ = nullptr;

	QCheckBox* windowing_use_fixed_value_ = nullptr;
	QSpinBox* windowing_fixed_level_spin_ = nullptr;
	QSpinBox* windowing_fixed_width_spin_ = nullptr;
};
