#pragma once
/*=========================================================================
File:			preferences_objects_widget.h
Language:		C++11
Library:        Qt 5.9.9
Author:			Lim Tae Kyun
First date:		2021-09-02
Last modify:	2021-09-06

Copyright (c) 2018 HDXWILL. All rights reserved.
=========================================================================*/

#include "base_widget.h"

class QCheckBox;
class UIFRAME_EXPORT PreferencesObjectsWidget : public BaseWidget
{
	Q_OBJECT
public:
	explicit PreferencesObjectsWidget(QWidget *parent = nullptr);
	virtual ~PreferencesObjectsWidget();

	PreferencesObjectsWidget(const PreferencesObjectsWidget&) = delete;
	const PreferencesObjectsWidget& operator = (const PreferencesObjectsWidget&) = delete;

	void SetGlobalPreferences();

signals:
#ifndef WILL3D_VIEWER
	void sigImplantLibraryClicked();
	void sigImplantPresetsClicked();
#endif

private:
	virtual void Reset() override;

	QVBoxLayout* CreateObjectsLayout();
	QVBoxLayout* CreateMeasureLayout();
	QVBoxLayout* CreateNerveLayout();
#ifndef WILL3D_VIEWER
	QVBoxLayout* CreateImplantLayout();
#endif

	QHBoxLayout* CreateMeasureColorLayout(const QString& text, QToolButton* button);

	void GetObjectsValues();
	void ChangeBtnColor(QColor& out_color, QToolButton* dest_btn);

private:
	QComboBox* measure_text_size_combo_ = nullptr;
	QDoubleSpinBox* nerve_default_diameter_spin_ = nullptr;

	QToolButton* measure_line_color_button_ = nullptr;
	QToolButton* measure_text_color_button_ = nullptr;
	QToolButton* nerve_default_color_button_ = nullptr;

	QColor measure_line_color_;
	QColor measure_text_color_;
	QColor nerve_default_color_;

	QCheckBox* measure_tape_line_multi_check_ = nullptr;

#ifndef WILL3D_VIEWER
	QCheckBox* implant_margin_visible_on_2d_views_check_ = nullptr;
	QCheckBox* implant_margin_visible_on_3d_views_check_ = nullptr;
	QCheckBox* implant_always_show_implant_id_check_ = nullptr;

	QDoubleSpinBox* implant_alpha_spin_ = nullptr;
	QDoubleSpinBox* implant_collision_margin_spin_ = nullptr;

	QRadioButton* implant_rendering_type_volume_radio_ = nullptr;
	QRadioButton* implant_rendering_type_wire_radio_ = nullptr;

	QToolButton* implant_default_color_volume_button_ = nullptr;
	QToolButton* implant_default_color_wire_button_ = nullptr;
	QToolButton* implant_selected_color_volume_button_ = nullptr;
	QToolButton* implant_selected_color_wire_button_ = nullptr;
	QToolButton* implant_collided_color_volume_button_ = nullptr;
	QToolButton* implant_collided_color_wire_button_ = nullptr;

	QColor implant_default_color_volume_;
	QColor implant_default_color_wire_;
	QColor implant_selected_color_volume_;
	QColor implant_selected_color_wire_;
	QColor implant_collided_color_volume_;
	QColor implant_collided_color_wire_;
#endif	
};
