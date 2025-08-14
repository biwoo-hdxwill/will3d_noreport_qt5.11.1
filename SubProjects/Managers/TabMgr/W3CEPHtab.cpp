#include "W3CEPHtab.h"
/*=========================================================================

File:			class CW3CEPHtab
Language:		C++11
Library:		Qt 5.2.0
Author:			Hong Jung
First date:		2016-01-12
Last date:		2016-01-14

=========================================================================*/
#include <qboxlayout.h>

#include <Engine/Common/Common/W3Memory.h>
#include <Engine/Common/Common/W3Theme.h>
#include <Engine/Common/Common/W3Logger.h>
#include <Engine/Common/Common/W3MessageBox.h>
#include <Engine/Common/Common/language_pack.h>
#include <Engine/Common/Common/W3LayoutFunctions.h>
#include <Engine/Resource/Resource/W3Image3D.h>
#ifndef WILL3D_VIEWER
#include <Engine/Core/W3ProjectIO/project_io_ceph.h>
#endif

#include <Engine/UIModule/UITools/ceph_task_tool.h>
#include <Engine/UIModule/UITools/tool_mgr.h>

#include <Engine/UIModule/UIComponent/W3VTOSTO.h>
#include <Engine/UIModule/UIComponent/W3View3DCeph.h>

#include <Engine/UIModule/UIFrame/window_plane.h>
#include <Engine/UIModule/UIFrame/W3CephCoordSysDlg.h>

#include "W3CephViewMgr.h"

CW3CEPHtab::CW3CEPHtab(CW3VREngine* VREngine, CW3MPREngine *MPRengine, CW3VTOSTO* vtosto,
					   CW3ResourceContainer *Rcontainer)
	: m_pgVREngine(VREngine), m_pgVTOSTO(vtosto),
	m_pgMPRengine(MPRengine),
	m_pgRcontainer(Rcontainer) {

	task_tool_.reset(new CephTaskTool(this));
	ToolMgr::instance()->SetCephTaskTool(task_tool_);
}

CW3CEPHtab::~CW3CEPHtab(void) {
	SAFE_DELETE_OBJECT(m_pCephViewMgr);
}
#ifndef WILL3D_VIEWER
void CW3CEPHtab::exportProject(ProjectIOCeph & out) {
	if (m_pCephViewMgr)
		m_pCephViewMgr->exportProject(out);
}

void CW3CEPHtab::importProject(ProjectIOCeph & in) {
	if (in.IsInit()) {
		if (!initialized()) {
			Initialize();
		}
		bool isCephDataTurnOn;
		m_pCephViewMgr->importProject(in, isCephDataTurnOn);

		if (isCephDataTurnOn) {
			if (!m_bCephLayoutFlag && !m_bCephSurgeryLayoutFlag) {
				m_bCephLayoutFlag = true;
				SetLayout();
			}
		} else {
			if (m_bCephLayoutFlag && !m_bCephSurgeryLayoutFlag) {
				m_bCephLayoutFlag = false;
				SetLayout();
			}
		}
	}
}
#endif
void CW3CEPHtab::UpdateVRview(bool is_high_quality) {
  m_pCephViewMgr->UpdateVRview(is_high_quality);
}

void CW3CEPHtab::connections() 
{
	connect(m_pCephViewMgr, SIGNAL(sigSetIndicatorLayoutMode()), this, SLOT(slotSetIndicatorLayoutMode()));
	connect(m_pCephViewMgr, SIGNAL(sigSetNoCephLayoutMode()), this, SLOT(slotSetNoCephLayoutMode()));
	connect(m_pCephViewMgr, SIGNAL(sigEnableSurgeryBarLayout(bool)), this, SLOT(slotEnableSurgeryBar(bool)));

	connect(task_tool_.get(), SIGNAL(sigCephTask(CephTaskID)), this, SLOT(slotCephTask(CephTaskID)));
	connect(task_tool_.get(), SIGNAL(sigCephClipParamsChanged()), m_pCephViewMgr, SLOT(slotCephClipParamsChanged()));
	connect(task_tool_.get(), SIGNAL(sigCephClipSet()), m_pCephViewMgr, SLOT(slotCephClipSet()));
	connect(task_tool_.get(), SIGNAL(sigCephVisible(VisibleID,int)), m_pCephViewMgr, SLOT(slotCephVisible(VisibleID,int)));
	connect(task_tool_.get(), SIGNAL(sigCephChangeFaceTransparency(int)), m_pCephViewMgr, SLOT(slotCephChangeFaceTransparency(int)));

#ifdef WILL3D_EUROPE
	connect(m_pCephViewMgr, &CW3CephViewMgr::sigShowButtonListDialog, this, &CW3CEPHtab::sigShowButtonListDialog);
#endif // WILL3D_EUROPE
}

void CW3CEPHtab::TaskCoordSysSelect() {
	CW3CephCoordSysDlg coord_sys_dlg;
	coord_sys_dlg.show();
}

void CW3CEPHtab::TaskTracing() {
	if (!m_pCephViewMgr->getSurgeryBar()->isVisible()) {
		SetTracingLayoutMode();
		m_pCephViewMgr->TaskTracing();
	}
}

void CW3CEPHtab::TaskSelectAnalysis() {
	m_pCephViewMgr->TaskSelectAnalysis();
}

void CW3CEPHtab::TaskSurgery() {
	if (m_pCephViewMgr->IsTracingFinished()) {
		m_pCephViewMgr->TaskSurgery();
	} else {
		CW3MessageBox msgBox("Will3D", lang::LanguagePack::msg_25(), CW3MessageBox::Information);
		msgBox.exec();

		task_tool_->SyncTaskUI(CephTaskID::SURGERY, false);
	}
}

void CW3CEPHtab::TaskShowSkin() 
{
	if (m_pgVTOSTO->isAvailableFace()) 
	{
		task_tool_->SyncVisibilityEnable(VisibleID::FACEPHOTO, true);
		return;
	}

	CW3MessageBox msgBox("Will3D", lang::LanguagePack::msg_26(), CW3MessageBox::Question);
	msgBox.setDetailedText(lang::LanguagePack::msg_47());

	if (msgBox.exec())
	{
		emit sigChangeTab(TabType::TAB_FACESIM);
	}
}

void CW3CEPHtab::SetLayout() {
  if (!BaseTab::initialized())
	return;

  window_->SetView(m_pCephViewMgr->getView3DCeph());

	if (m_bCephLayoutFlag) {
		if (m_bCephSurgeryLayoutFlag) {
			if (CW3Theme::getInstance()->isLowResolution()) {
				setCephLayout(
					m_pCephViewMgr->getSurgeryBar(),
					window_.get());
			} else {
				setCephLayout(
					m_pCephViewMgr->getSurgeryBar(),
					window_.get(),
					m_pCephViewMgr->getIndicatorBar());
			}
		} else {
			setCephLayout(
				window_.get(),
				m_pCephViewMgr->getIndicatorBar());
		}
	} else {
		setCephLayout(
			m_pCephViewMgr->getTracingBar(),
			window_.get());
	}

	m_pCephViewMgr->setCephNoEventMode(false);
}


void CW3CEPHtab::ReleaseSurgeryViewFromLayout() {
  if (tab_layout()) {
	tab_layout()->removeWidget(m_pCephViewMgr->getView3DCeph());
  }
}


QLayout* CW3CEPHtab::GetTabLayout() {
	if(initialized())
		this->SetLayout(); 
	return BaseTab::GetTabLayout();
}

void CW3CEPHtab::SetVisibleWindows(bool isVisible) {
	if (!initialized())
		return;

	window_->setVisible(isVisible);
	m_pCephViewMgr->getView3DCeph()->setVisible(isVisible);
	if (isVisible) {
	task_tool_->SyncVisibilityResources();
	CW3View2D_thyoo* view = reinterpret_cast<CW3View2D_thyoo*>(m_pCephViewMgr->getView3DCeph());

		if (view != nullptr) {
			//view->changeTextViewName(lang::LanguagePack::txt_3d_ceph());
			m_pCephViewMgr->setCephNoEventMode(false);
		}
		CW3LayoutFunctions::setVisibleWidgets(tab_layout_, isVisible);
	} else
		this->setVisibleViews(isVisible);

}

void CW3CEPHtab::SetCommonToolOnce(const common::CommonToolTypeOnce & type, bool on) {
	m_pCephViewMgr->SetCommonToolOnce(type, on);
}

void CW3CEPHtab::SetCommonToolOnOff(const common::CommonToolTypeOnOff & type) {
	m_pCephViewMgr->SetCommonToolOnOff(type);
}

void CW3CEPHtab::Initialize() {
	if (BaseTab::initialized()) {
		common::Logger::instance()->Print(common::LogType::ERR, "already initialized.");
		assert(false);
	}

	m_pCephViewMgr = new CW3CephViewMgr(m_pgVREngine, m_pgMPRengine,
										m_pgVTOSTO, m_pgRcontainer);
	m_pCephViewMgr->set_task_tool(task_tool_);
	bool isCephDataTurnOn;
	m_pCephViewMgr->setVolume(isCephDataTurnOn);

	if (isCephDataTurnOn) {
		if (!m_bCephLayoutFlag && !m_bCephSurgeryLayoutFlag) {
			m_bCephLayoutFlag = true;
		}
	} else {
		if (m_bCephLayoutFlag && !m_bCephSurgeryLayoutFlag) {
			m_bCephLayoutFlag = false;
		}
	}
	BaseTab::set_initialized(true);

	m_pCephViewMgr->getView3DCeph()->setVisible(false);
	window_.reset(new WindowPlane(lang::LanguagePack::txt_3d_ceph()));
	window_->SetView(m_pCephViewMgr->getView3DCeph());
	SetLayout();

	connections();
}

void CW3CEPHtab::setVisibleViews(bool isVisible) {
	if (!initialized())
		return;

	m_pCephViewMgr->setVisibleViews(isVisible);

	if (m_bCephLayoutFlag) {
		if (m_bCephSurgeryLayoutFlag) {
			m_pCephViewMgr->getSurgeryBar()->setVisible(isVisible);

			if (!CW3Theme::getInstance()->isLowResolution())
				m_pCephViewMgr->getIndicatorBar()->setVisible(isVisible);
		} else {
			m_pCephViewMgr->getIndicatorBar()->setVisible(isVisible);
		}
	} else {
		m_pCephViewMgr->getTracingBar()->setVisible(isVisible);
	}

	m_pCephViewMgr->getView3DCeph()->setVisible(isVisible);
}

QWidget* CW3CEPHtab::getView3DCeph() {
	if (!initialized()) {
		Initialize();
	}

	return m_pCephViewMgr->getView3DCeph();
}

void CW3CEPHtab::disableSurgery() {
	m_pCephViewMgr->setCephNoEventMode(false);
	m_pCephViewMgr->disableSurgery();
}

void CW3CEPHtab::SetTracingLayoutMode() {
	if (m_bCephLayoutFlag && !m_bCephSurgeryLayoutFlag) {
		setVisibleViews(false);
		m_bCephLayoutFlag = false;
		emit sigChangeTab(TAB_3DCEPH);
	}
}

void CW3CEPHtab::slotSetIndicatorLayoutMode() {
	if (!m_bCephLayoutFlag && !m_bCephSurgeryLayoutFlag) {
		setVisibleViews(false);
		m_bCephLayoutFlag = true;
		emit sigChangeTab(TAB_3DCEPH);
	}
}

void CW3CEPHtab::slotSetNoCephLayoutMode() {
	if (!m_bCephSurgeryLayoutFlag) {
		m_bCephLayoutFlag = false;
		emit sigChangeTab(TAB_3DCEPH);
	}
}

void CW3CEPHtab::slotEnableSurgeryBar(const bool isEnable) {
	if (m_bCephLayoutFlag && m_bCephSurgeryLayoutFlag != isEnable) {
		setVisibleViews(false);
		m_bCephSurgeryLayoutFlag = isEnable;
		emit sigChangeTab(TAB_3DCEPH);
	}
}

void CW3CEPHtab::slotCephTask(const CephTaskID &task_id) {
	switch (task_id) {
	case CephTaskID::COORDSYS:
		TaskCoordSysSelect();
		break;
	case CephTaskID::TRACING:
		TaskTracing();
		break;
	case CephTaskID::SELECT_METHOD:
		TaskSelectAnalysis();
		break;
	case CephTaskID::SURGERY:
		TaskSurgery();
		break;
	case CephTaskID::SHOW_SKIN:
		TaskShowSkin();
		break;
	default:
		break;
	}
}

void CW3CEPHtab::setMIP(bool isMIP) {
	m_pCephViewMgr->setMIP(isMIP);
}

bool CW3CEPHtab::setCephLayout(QWidget* v1, QWidget* v2) {
	if (v1 == nullptr || v2 == nullptr) return false;

	if (tab_layout_) {
	  CW3LayoutFunctions::RemoveWidgetsAll(tab_layout_);
	  SAFE_DELETE_OBJECT(tab_layout_);
	}

	QHBoxLayout *LRLayout = new QHBoxLayout;
	LRLayout->setContentsMargins(0, 0, 0, 0);
	LRLayout->setMargin(0);
	LRLayout->setSpacing(0);
	LRLayout->addWidget(v1);
	LRLayout->addWidget(v2);

	tab_layout_ = LRLayout;

	return true;
}

bool CW3CEPHtab::setCephLayout(QWidget* v1, QWidget* v2, QWidget* v3) {
	if (v1 == nullptr || v2 == nullptr || v3 == nullptr) return false;

	if (tab_layout_) {
	  CW3LayoutFunctions::RemoveWidgetsAll(tab_layout_);
	  SAFE_DELETE_OBJECT(tab_layout_);
	}

	QHBoxLayout *LRLayout = new QHBoxLayout;
	LRLayout->setContentsMargins(0, 0, 0, 0);
	LRLayout->setMargin(0);
	LRLayout->setSpacing(0);
	LRLayout->addWidget(v1);
	LRLayout->addWidget(v2);
	LRLayout->addWidget(v3);
	LRLayout->setStretch(2, 1);

	tab_layout_ = LRLayout;

	return true;
}

void CW3CEPHtab::slotEnableCephNoEventMode() {
	m_pCephViewMgr->setCephNoEventMode(true);
}

QStringList CW3CEPHtab::GetViewList() {
	return QStringList{ window_.get()->window_title() };
}

QImage CW3CEPHtab::GetScreenshot(int view_type) {
	QWidget* source = GetScreenshotSource(view_type);

	return BaseTab::GetScreenshot(source);
}

QWidget* CW3CEPHtab::GetScreenshotSource(int view_type)
{
	QWidget* source = nullptr;

	switch (view_type)
	{
	case 1:
		source = window_.get();
		break;
	}

	return source;
}

#ifdef WILL3D_EUROPE
void CW3CEPHtab::SyncControlButtonOut()
{
	m_pCephViewMgr->SetSyncControlButtonOut();
}
#endif // WILL3D_EUROPE

void CW3CEPHtab::DeleteMeasureUI(const common::ViewTypeID & view_type,
								 const unsigned int & measure_id) {
	m_pCephViewMgr->DeleteMeasureUI(view_type, measure_id);
}

void CW3CEPHtab::ApplyPreferences() {
	if (m_pCephViewMgr)
		m_pCephViewMgr->ApplyPreferences();
}
