#pragma once
/*=========================================================================
File:			class GlobalPreferencesDialog
Language:		C++11
Library:        Qt 5.9.9
Author:			Jung Dae Gun
First date:		2018-04-16
Last modify:	2021-09-07

Copyright (c) 2018 HDXWILL. All rights reserved.
=========================================================================*/

#include "../../Common/Common/W3Dialog.h"

#include "uiframe_global.h"

class QHBoxLayout;
class QTabWidget;
class CW3VREngine;
class CW3MPREngine;
class CW3ResourceContainer;

class PreferencesGeneralWidget;
class PreferencesObjectsWidget;
class PreferencesAdvancedWidget;
class PreferencesPACSDefaultSettingWidget;
class PreferencesPACSServerWidget;
class UIFRAME_EXPORT GlobalPreferencesDialog : public CW3Dialog
{
	Q_OBJECT
public:
	GlobalPreferencesDialog(CW3VREngine* vr_engine, CW3MPREngine* mpr_engine,
		CW3ResourceContainer* rcontainer, QWidget* parent = 0);
	~GlobalPreferencesDialog();
	GlobalPreferencesDialog(const GlobalPreferencesDialog&) = delete;
	const GlobalPreferencesDialog& operator = (const GlobalPreferencesDialog&) = delete;

private slots:
	void slotOK();
#ifndef WILL3D_VIEWER
	void slotImplantLibrary();
	void slotImplantPresets();
#endif

private:
	void SetLayout();
	QTabWidget* CreateTabWidget();
	QHBoxLayout* CreateButtonLayout();

private:
	CW3VREngine* m_pgVREngine = nullptr;
	CW3MPREngine* mpr_engine_ = nullptr;
	CW3ResourceContainer* rcontainer_ = nullptr;
	
	PreferencesGeneralWidget* preferences_general_widget_ = nullptr;
	PreferencesObjectsWidget* preferences_objects_widget_ = nullptr;
	PreferencesAdvancedWidget* preferences_Advanced_widget_ = nullptr;

#ifndef WILL3D_VIEWER
	PreferencesPACSDefaultSettingWidget* preferences_pacs_default_setting_widget_ = nullptr;
	PreferencesPACSServerWidget* preferences_pacs_server_widget_ = nullptr;
#endif
};
