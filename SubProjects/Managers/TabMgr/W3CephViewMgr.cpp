#include "W3CephViewMgr.h"

/*=========================================================================

File:			class CW3CephViewMgr
Language:		C++11
Library:        Qt 5.4.0
Author:			Tae Hoon yoo
First date:		2016-01-11
Last modify:	2016-01-11

=========================================================================*/
#include <QApplication>
#include <QComboBox>

#include <Engine/Common/Common/W3Memory.h>
#include <Engine/Common/Common/W3MessageBox.h>
#include <Engine/Common/Common/language_pack.h>
#include <Engine/Resource/ResContainer/resource_container.h>
#include <Engine/Resource/Resource/W3Image3D.h>
#include <Engine/Resource/Resource/W3ImageHeader.h>
#ifndef WILL3D_VIEWER
#include <Engine/Core/W3ProjectIO/project_io_ceph.h>
#endif
#include <Engine/UIModule/UIPrimitive/W3SpanSlider.h>
#include <Engine/UIModule/UITools/ceph_surgery_tool.h>
#include <Engine/UIModule/UITools/ceph_task_tool.h>
#include <Engine/UIModule/UITools/datatypes.h>

#include <Engine/Module/VREngine/W3VREngine.h>

#include <Engine/UIModule/UIComponent/W3View3DCeph.h>
#include <Engine/UIModule/UIFrame/W3CephIndicatorBar.h>
#include <Engine/UIModule/UIFrame/W3CephTracingBar.h>

#include "../DBManager/W3CephDM.h"

CW3CephViewMgr::CW3CephViewMgr(CW3VREngine* VREngine, CW3MPREngine* MPRengine,
	CW3VTOSTO* vtosto,
	CW3ResourceContainer* Rcontainer,
	QWidget* parent)
	: m_pgVRengine(VREngine), QWidget(parent) {
	m_pDataManager = new CW3CephDM(this);

	m_pTracingBar = new CW3CephTracingBar(this);
	m_pIndicatorBar = new CW3CephIndicatorBar(m_pDataManager, this);
	surgery_bar_.reset(new CephSurgeryTool(this));

	m_pView3DCeph = new CW3View3DCeph(m_pgVRengine, MPRengine, vtosto,
		m_pDataManager, common::ViewTypeID::CEPH);

	connections();
}

CW3CephViewMgr::~CW3CephViewMgr() {
	SAFE_DELETE_OBJECT(m_pView3DCeph);
	SAFE_DELETE_OBJECT(m_pDataManager);
	SAFE_DELETE_OBJECT(m_pTracingBar);
	SAFE_DELETE_OBJECT(m_pIndicatorBar);
}

void CW3CephViewMgr::set_task_tool(const std::shared_ptr<CephTaskTool>& tool) {
	task_tool_ = tool;

	bool is_visible_face = task_tool_->IsVisible(VisibleID::FACEPHOTO);
	m_pView3DCeph->FacePhotoEnable(is_visible_face);
	connect(m_pView3DCeph, SIGNAL(sigClipLower(int)), this,
		SLOT(slotViewClipLower(int)));
	connect(m_pView3DCeph, SIGNAL(sigClipUpper(int)), this,
		SLOT(slotViewClipUpper(int)));
}
#ifndef WILL3D_VIEWER
void CW3CephViewMgr::exportProject(ProjectIOCeph& out) {
	out.InitCephTab();
	m_pView3DCeph->exportProject(out);
	surgery_bar_->exportProject(out);
}

void CW3CephViewMgr::importProject(ProjectIOCeph& in, bool& is_data_turn_on) {
	m_pView3DCeph->importProject(in);
	surgery_bar_->importProject(in);

	if (m_pDataManager->isTurnOn()) {
		m_pTracingBar->checkAll();
		is_data_turn_on = true;
	}
	else {
		is_data_turn_on = false;
	}
}
#endif
void CW3CephViewMgr::UpdateVRview(bool is_high_quality) {
	m_pView3DCeph->UpdateVR(is_high_quality);
}
void CW3CephViewMgr::reset() {
	m_pView3DCeph->reset();

	ClipStatus clip_status = task_tool_.get()->GetClipStatus();
	m_pView3DCeph->initClipValues(clip_status.plane, clip_status.enable,
		clip_status.lower, clip_status.upper);
}

bool CW3CephViewMgr::IsTracingFinished() const {
	return !m_pTracingBar->isVisible() && m_pView3DCeph->isFinishedTracing();
}

void CW3CephViewMgr::TaskTracing() { m_pTracingBar->awake(); }

void CW3CephViewMgr::SetCommonToolOnce(const common::CommonToolTypeOnce& type,
	bool on) {
	m_pView3DCeph->SetCommonToolOnce(type, on);
}

void CW3CephViewMgr::SetCommonToolOnOff(
	const common::CommonToolTypeOnOff& type) {
	m_pView3DCeph->SetCommonToolOnOff(type);
}

void CW3CephViewMgr::setVisibleViews(bool isVisible) {
	m_pTracingBar->setVisible(isVisible);
	m_pIndicatorBar->setVisible(isVisible);
	surgery_bar_->setVisible(isVisible);
	m_pView3DCeph->setVisible(isVisible);
}

void CW3CephViewMgr::setVolume(bool& isCephDataTurnOn) {
	if (!&ResourceContainer::GetInstance()->GetMainVolume()) return;

	QString strStudyID = ResourceContainer::GetInstance()
		->GetMainVolume()
		.getHeader()
		->getStudyID();
	m_pDataManager->InitFromDatabase(strStudyID);

	if (m_pDataManager->isTurnOn()) {
		m_pTracingBar->checkAll();
		isCephDataTurnOn = true;
	}
	else {
		isCephDataTurnOn = false;
	}
}

void CW3CephViewMgr::connections(void)
{
	const auto& surgery_bar = surgery_bar_.get();
	connect(surgery_bar, &CephSurgeryTool::sigSurgeryModeEnable, this, &CW3CephViewMgr::slotSurgeryModeEnable);
	connect(surgery_bar, &CephSurgeryTool::sigSurgeryEnableOn, m_pView3DCeph, &CW3View3DCeph::slotSurgeryEnableOn);
	connect(surgery_bar, &CephSurgeryTool::sigSurgeryAdjustOn, m_pView3DCeph, &CW3View3DCeph::slotSurgeryAdjustOn);
	connect(surgery_bar, &CephSurgeryTool::sigSurgeryMoveOn, m_pView3DCeph, &CW3View3DCeph::slotSurgeryMoveOn);
	connect(surgery_bar, &CephSurgeryTool::sigSurgeryCutTranslate, m_pView3DCeph, &CW3View3DCeph::slotSurgeryCutTranslate);
	connect(surgery_bar, &CephSurgeryTool::sigSurgeryCutRotate, m_pView3DCeph, &CW3View3DCeph::slotSurgeryCutRotate);
	connect(surgery_bar, &CephSurgeryTool::sigSurgeryAxisTranslate, m_pView3DCeph, &CW3View3DCeph::slotSurgeryAxisTranslate);
	connect(surgery_bar, &CephSurgeryTool::sigSurgeryAxisRotate, m_pView3DCeph, &CW3View3DCeph::slotSurgeryAxisRotate);

	connect(m_pView3DCeph, &CW3View3DCeph::sigSurgeryTrans, surgery_bar, &CephSurgeryTool::slotTranslateFromView);
	connect(m_pView3DCeph, &CW3View3DCeph::sigSurgeryRotate, surgery_bar, &CephSurgeryTool::slotRotateFromView);

	connect(m_pView3DCeph, &CW3View3DCeph::sigSetZeroValuesSurgeryBar, surgery_bar, &CephSurgeryTool::slotSurgeryParamsClear);
	connect(m_pView3DCeph, &CW3View3DCeph::sigDisableAllSurgeryBar, surgery_bar, &CephSurgeryTool::slotSurgeryParamsDisable);

	connect(m_pTracingBar, &CW3CephTracingBar::sigActiveTracingTask, m_pView3DCeph, &CW3View3DCeph::slotActiveTracingTask);
	connect(m_pTracingBar, &CW3CephTracingBar::sigClearTracingTasks, this, &CW3CephViewMgr::slotTracingTaskClear);
	connect(m_pTracingBar, &CW3CephTracingBar::sigFinishedTracingTasks, this, &CW3CephViewMgr::slotTracingTaskFinished);
	connect(m_pTracingBar, &CW3CephTracingBar::sigCancelTracingTask, this, &CW3CephViewMgr::slotTracingTaskCancel);
	connect(m_pTracingBar, &CW3CephTracingBar::sigSetCoordSystem, m_pView3DCeph, &CW3View3DCeph::slotSetCoordSystem);

	connect(m_pIndicatorBar->getComboBoxSelectAnalysis(), &QComboBox::currentTextChanged, this, &CW3CephViewMgr::slotChangeSelectAnalysis);
	connect(m_pIndicatorBar, &CW3CephIndicatorBar::sigLandmarkChangeContentSwitch, m_pView3DCeph, &CW3View3DCeph::slotLandmarkChangeContentSwitch);
	connect(m_pIndicatorBar, &CW3CephIndicatorBar::sigMeasurementChangeContentSwitch, m_pView3DCeph, &CW3View3DCeph::slotMeasurementChangeContentSwitch);
	connect(m_pIndicatorBar, &CW3CephIndicatorBar::sigReferenceChangeContentSwitch, m_pView3DCeph, &CW3View3DCeph::slotReferenceChangeContentSwitch);

	connect(m_pView3DCeph, &CW3View3DCeph::sigSetTracingGuideImage, m_pTracingBar, &CW3CephTracingBar::slotSetGuideImage);
	connect(m_pView3DCeph, &CW3View3DCeph::sigDoneTracingTask, m_pTracingBar, &CW3CephTracingBar::slotNextTracingTask);
	connect(m_pView3DCeph, &CW3View3DCeph::sigSyncCephIndicatorBar, m_pIndicatorBar, &CW3CephIndicatorBar::slotSyncCephView);
	connect(m_pView3DCeph, &CW3View3DCeph::sigSetClipToolValues, this, &CW3CephViewMgr::slotViewClipParamsSet);

	connect(m_pgVRengine, &CW3VREngine::sigTFupdated, m_pView3DCeph, &CW3View3DCeph::slotTFupdated);
	connect(m_pgVRengine, &CW3VREngine::sigTFupdateCompleted, m_pView3DCeph, &CW3View3DCeph::slotRenderCompleted);
	connect(m_pgVRengine, &CW3VREngine::sigShadeOn, m_pView3DCeph, &CW3View3DCeph::slotShadeOnFromOTF);

#ifdef WILL3D_EUROPE
	connect(m_pView3DCeph, &CW3View3DCeph::sigShowButtonListDialog, this, &CW3CephViewMgr::sigShowButtonListDialog);
#endif // WILL3D_EUROPE
}

void CW3CephViewMgr::TaskSelectAnalysis() {
	if (!m_pIndicatorBar->isVisible()) {
		CW3MessageBox msgBox("Will3D", lang::LanguagePack::msg_25(),
			CW3MessageBox::Information);
		msgBox.exec();
		return;
	}

	if (m_pIndicatorBar->getCurrentTab() != CW3CephIndicatorBar::TAB_ANALYSIS) {
		m_pIndicatorBar->setCurrentTab(CW3CephIndicatorBar::TAB_ANALYSIS);
		QApplication::processEvents();
	}
	m_pIndicatorBar->getComboBoxSelectAnalysis()->showPopup();
}
void CW3CephViewMgr::TaskSurgery() { surgery_bar_->slotChangeEnable(); }

void CW3CephViewMgr::DeleteMeasureUI(const common::ViewTypeID& view_type,
	const unsigned int& measure_id) {
	if (view_type == common::ViewTypeID::CEPH)
		m_pView3DCeph->DeleteMeasureUI(measure_id);
}

void CW3CephViewMgr::slotChangeSelectAnalysis(const QString&) {
	task_tool_->SyncTaskEventLeave(CephTaskID::SELECT_METHOD);
}

void CW3CephViewMgr::setCephNoEventMode(bool isEnable) {
	m_pView3DCeph->setViewType(isEnable ? common::ViewTypeID::FACE_AFTER
		: common::ViewTypeID::CEPH);
	m_pView3DCeph->setNoEventMode(isEnable);
}
void CW3CephViewMgr::setMIP(bool isMIP) { m_pView3DCeph->setMIP(isMIP); }
void CW3CephViewMgr::disableSurgery() {
	surgery_bar_->slotSurgeryParamsDisable();
}

void CW3CephViewMgr::slotViewClipLower(int value) {
	task_tool_->SetClipLower(value);
}

void CW3CephViewMgr::slotViewClipUpper(int value) {
	task_tool_->SetClipUpper(value);
}

void CW3CephViewMgr::slotViewClipParamsSet(const bool isEnable,
	const bool isFlip,
	const MPRClipID plane,
	const int lower, const int upper) {
	task_tool_->SetClipParams(isEnable, isFlip, plane, lower, upper);
}
void CW3CephViewMgr::slotTracingTaskCancel() {
	// tracing bar에서 cancel 버튼을 누르면 호출된다.
	emit sigSetIndicatorLayoutMode();
	m_pView3DCeph->TracingTaskCancel();
}

void CW3CephViewMgr::slotSurgeryModeEnable(const bool isEnable) {
	m_pView3DCeph->SurgeryEnable(isEnable);

	emit sigEnableSurgeryBarLayout(isEnable);
	if (!isEnable) task_tool_->SyncTaskUI(CephTaskID::SURGERY, false);
}

void CW3CephViewMgr::slotTracingTaskClear() {
	m_pView3DCeph->TracingTasksClear();
	m_pIndicatorBar->TracingTasksClear();
}

void CW3CephViewMgr::slotTracingTaskFinished() {
	m_pView3DCeph->TracingTasksFinished();
	emit sigSetIndicatorLayoutMode();
}

void CW3CephViewMgr::slotCephClipParamsChanged() {
	ClipStatus clip_status = task_tool_->GetClipStatus();
	m_pView3DCeph->SetClipValues(clip_status.plane, clip_status.enable,
		clip_status.lower, clip_status.upper);
	;
}

void CW3CephViewMgr::slotCephClipSet() { m_pView3DCeph->slotRenderCompleted(); }

void CW3CephViewMgr::slotCephVisible(const VisibleID& id, int status) {
	if (id == VisibleID::FACEPHOTO) m_pView3DCeph->FacePhotoEnable(status);
}

void CW3CephViewMgr::slotCephChangeFaceTransparency(int value) {
	m_pView3DCeph->FacePhotoTransparencyChange(value);
}

void CW3CephViewMgr::ApplyPreferences()
{
	m_pView3DCeph->ApplyPreferences();
}

#ifdef WILL3D_EUROPE
void CW3CephViewMgr::SetSyncControlButtonOut()
{
	m_pView3DCeph->SetSyncControlButton(false);
}
#endif // WILL3D_EUROPE
