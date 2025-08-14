#pragma once
/*=========================================================================
File:			preferences_general_widget.h
Language:		C++11
Library:        Qt 5.9.9
Author:			Lim Tae Kyun
First date:		2021-08-31
Last modify:	2021-09-01

Copyright (c) 2018 HDXWILL. All rights reserved.
=========================================================================*/

#include "base_widget.h"

class QCheckBox;
class QListWidget;
class UIFRAME_EXPORT PreferencesGeneralWidget : public BaseWidget
{
	Q_OBJECT
public:
	explicit PreferencesGeneralWidget(QWidget *parent = nullptr);
	virtual ~PreferencesGeneralWidget();

	PreferencesGeneralWidget(const PreferencesGeneralWidget&) = delete;
	const PreferencesGeneralWidget& operator = (const PreferencesGeneralWidget&) = delete;

	void SetGlobalPreferences();	

private slots:
#ifndef WILL3D_VIEWER
	void slotSetEnableNetwork(int radio_id);
	void slotAddNewPath();
	void slotDeleteFavoriteOpenPaths();
#endif

private:
	virtual void Reset() override;

	QVBoxLayout* CreateGeneralLayout();
#ifndef WILL3D_VIEWER
	QVBoxLayout* CreateNetworkSettingLayout();
#endif
	QVBoxLayout* CreateFilesLayout();
	QVBoxLayout* CreateInterfaceLayout();
	QVBoxLayout* CreateDisplayLayout();

	QHBoxLayout* CreateFilesPathLayout(const QString& text, QLineEdit** input);
	QHBoxLayout* CreateInterfaceComboBoxLayout(const QString& text, QComboBox** input, const QStringList& items);

	void GetGeneralValues();
	QString GetFolderPath();

private:
	//Network
#ifndef WILL3D_VIEWER
	QCheckBox* network_setting_use_willmaster_check_ = nullptr;
	QRadioButton* network_setting_local_radio_ = nullptr;
	QRadioButton* network_setting_network_radio_ = nullptr;
	QLineEdit* network_setting_ip_address_input_ = nullptr;

	//File
	QListWidget* files_favorite_open_paths_list_ = nullptr;
#endif
	QLineEdit* files_capture_path_input_ = nullptr;
	QLineEdit* files_stl_export_path_input_ = nullptr;
	
	//Interface
	QComboBox* interface_language_combo_ = nullptr;
	QComboBox* interface_gui_size_combo_ = nullptr;
	QComboBox* interface_font_size_combo_ = nullptr;
	
	//Display
	QCheckBox* display_show_slice_numbers_check_ = nullptr;
	QCheckBox* display_show_rulers_check_ = nullptr;
	
	QRadioButton* display_grid_spacing_first_radio_ = nullptr;
	QRadioButton* display_grid_spacing_second_radio_ = nullptr;
	QRadioButton* display_grid_spacing_third_radio_ = nullptr;
};
