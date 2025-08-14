#include "global_preferences_dialog.h"

#include <QHBoxLayout>
#include <QToolButton>
#include <QTabWidget>

#include "../../Common/Common/global_preferences.h"
#include "../../Common/Common/W3Define.h"
#include "../../Common/Common/W3Memory.h"
#include "../../Common/Common/W3Theme.h"
#include "../../Common/Common/W3LayoutFunctions.h"

#include "../../Module/VREngine/W3VREngine.h"

#ifndef WILL3D_VIEWER
#include "W3ImplantDBDlg.h"
#include "implant_preference_dialog.h"

#include "preferences_pacs_defalut_setting_widget.h"
#include "preferences_pacs_server_widget.h"

#include "pacs_add_server_dialog.h"
#endif

#include "preferences_general_widget.h"
#include "preferences_objects_widget.h"
#include "preferences_advanced_widget.h"

GlobalPreferencesDialog::GlobalPreferencesDialog(CW3VREngine* vr_engine,
	CW3MPREngine* mpr_engine,
	CW3ResourceContainer* rcontainer, QWidget* parent)
	: CW3Dialog(lang::LanguagePack::txt_preferences(), parent),
	m_pgVREngine(vr_engine), mpr_engine_(mpr_engine), rcontainer_(rcontainer)
{
	SetLayout();
}

GlobalPreferencesDialog::~GlobalPreferencesDialog()
{
	SAFE_DELETE_LATER(preferences_general_widget_);
#ifndef WILL3D_EUROPE
	SAFE_DELETE_LATER(preferences_objects_widget_);
	SAFE_DELETE_LATER(preferences_Advanced_widget_);
#endif // !WILL3D_EUROPE
#ifndef WILL3D_VIEWER
	SAFE_DELETE_LATER(preferences_pacs_default_setting_widget_);
	SAFE_DELETE_LATER(preferences_pacs_server_widget_);
#endif
}

void GlobalPreferencesDialog::SetLayout()
{
	m_contentLayout->setContentsMargins(20, 10, 20, 10);
	m_contentLayout->setSpacing(10);
	m_contentLayout->addWidget(CreateTabWidget());
	m_contentLayout->addLayout(CreateButtonLayout());
}

QTabWidget* GlobalPreferencesDialog::CreateTabWidget()
{
	QTabWidget* tab_widget = new QTabWidget(this);

	tab_widget->setStyleSheet(CW3Theme::getInstance()->GlobalPreferencesDialogTabStyleSheet());
	
	preferences_general_widget_ = new PreferencesGeneralWidget(this);
#ifndef WILL3D_EUROPE
	preferences_objects_widget_ = new PreferencesObjectsWidget(this);
	preferences_Advanced_widget_ = new PreferencesAdvancedWidget(this);
#endif // !WILL3D_EUROPE

#ifndef WILL3D_VIEWER
	connect(preferences_objects_widget_, &PreferencesObjectsWidget::sigImplantLibraryClicked, this, &GlobalPreferencesDialog::slotImplantLibrary);
	connect(preferences_objects_widget_, &PreferencesObjectsWidget::sigImplantPresetsClicked, this, &GlobalPreferencesDialog::slotImplantPresets);
#endif

	tab_widget->addTab(preferences_general_widget_, lang::LanguagePack::txt_general());
#ifndef WILL3D_EUROPE
	tab_widget->addTab(preferences_objects_widget_, lang::LanguagePack::txt_objects());
	tab_widget->addTab(preferences_Advanced_widget_, lang::LanguagePack::txt_advanced());
#endif // !WILL3D_EUROPE

#ifndef WILL3D_VIEWER
	if (GlobalPreferences::GetInstance()->preferences_.pacs.pacs_on)
	{
		preferences_pacs_default_setting_widget_ = new PreferencesPACSDefaultSettingWidget(this);
		preferences_pacs_server_widget_ = new PreferencesPACSServerWidget(this);

		tab_widget->addTab(preferences_pacs_default_setting_widget_, lang::LanguagePack::txt_pacs());
		tab_widget->addTab(preferences_pacs_server_widget_, lang::LanguagePack::txt_pacs());
	}
#endif
	
	tab_widget->setCurrentIndex(0);
	   
	return tab_widget;
}

QHBoxLayout* GlobalPreferencesDialog::CreateButtonLayout()
{
	QHBoxLayout* layout = new QHBoxLayout();
	layout->setSpacing(10);
	layout->setAlignment(Qt::AlignCenter);

	QToolButton* ok_button = new QToolButton(this);
	QToolButton* cancel_button = new QToolButton(this);

	connect(ok_button, SIGNAL(clicked()), this, SLOT(slotOK()));
	connect(cancel_button, SIGNAL(clicked()), this, SLOT(reject()));

	ok_button->setText(lang::LanguagePack::txt_ok());
	cancel_button->setText(lang::LanguagePack::txt_cancel());

	layout->addWidget(ok_button);
	layout->addWidget(cancel_button);

	return layout;
}

void GlobalPreferencesDialog::slotOK()
{
	preferences_general_widget_->SetGlobalPreferences();
#ifndef WILL3D_EUROPE
	preferences_objects_widget_->SetGlobalPreferences();
	preferences_Advanced_widget_->SetGlobalPreferences();
#endif // !WILL3D_EUROPE
	
#ifndef WILL3D_VIEWER
	if (GlobalPreferences::GetInstance()->preferences_.pacs.pacs_on)
	{
		preferences_pacs_default_setting_widget_->SetGlobalPreferences();
	}
#endif

	GlobalPreferences::GetInstance()->SavePreferences();
	
	accept();
}

#ifndef WILL3D_VIEWER
void GlobalPreferencesDialog::slotImplantLibrary()
{
	if (!m_pgVREngine || !mpr_engine_ || !rcontainer_ || !m_pgVREngine->getVol(0))
	{
		return;
	}

	CW3ImplantDBDlg implant_db_dialog(m_pgVREngine, mpr_engine_, rcontainer_, this);
	implant_db_dialog.exec();
}

void GlobalPreferencesDialog::slotImplantPresets()
{
	if (!m_pgVREngine || !mpr_engine_ || !rcontainer_ || !m_pgVREngine->getVol(0))
	{
		return;
	}

	ImplantPreferenceDlg implant_preference_dialog(m_pgVREngine, mpr_engine_, rcontainer_, this);
	implant_preference_dialog.exec();
}
#endif
