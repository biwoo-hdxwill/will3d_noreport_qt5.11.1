#include "W3ENDOViewMgr.h"
/*=========================================================================

File:			class CW3ENDOViewMgr
Language:		C++11
Library:		Qt 5.2.0
Author:			Jung Dae Gun, Hong Jung
First date:		2016-01-13
Last modify:	2016-04-07

=========================================================================*/
#include "../../Engine/Common/Common/W3Memory.h"
#ifndef WILL3D_VIEWER
#include "../../Engine/Core/W3ProjectIO/project_io_endo.h"
#endif
#include "../../Engine/UIModule/UIComponent/W3View3DEndoSagittal.h"
#include "../../Engine/UIModule/UIComponent/W3View3DEndoSlice.h"
#include "../../Engine/UIModule/UIComponent/W3View3DEndo.h"
#include "../../Engine/UIModule/UIComponent/W3View3DEndoModify.h"

#include <Engine/UIModule/UITools/endo_task_tool.h>

#include "../../Engine/Module/VREngine/W3VREngine.h"

CW3ENDOViewMgr::CW3ENDOViewMgr(CW3VREngine *VREngine, CW3MPREngine *MPRengine,
							   CW3JobMgr *JobMgr, CW3ResourceContainer *Rcontainer,
							   QWidget *parent) : m_pgVRengine(VREngine) {
	m_pView3DEndoSagital = new CW3View3DEndoSagittal(
		m_pgVRengine, MPRengine, JobMgr, Rcontainer,
		common::ViewTypeID::ENDO_SAGITTAL, this);

	m_pView3DEndoSlice = new CW3View3DEndoSlice(
		m_pgVRengine, MPRengine, Rcontainer,
		common::ViewTypeID::ENDO_SLICE, this);

	m_pView3DENDO = new CW3View3DEndo(
		m_pgVRengine, MPRengine, Rcontainer,
		common::ViewTypeID::ENDO, this);

	m_pView3DEndoModify = new CW3View3DEndoModify(
		m_pgVRengine, MPRengine, Rcontainer,
		common::ViewTypeID::ENDO_MODIFY, this);

	connections();
}

CW3ENDOViewMgr::~CW3ENDOViewMgr(void) {
	SAFE_DELETE_OBJECT(m_pView3DEndoSagital);
	SAFE_DELETE_OBJECT(m_pView3DEndoSlice);
	SAFE_DELETE_OBJECT(m_pView3DENDO);
	SAFE_DELETE_OBJECT(m_pView3DEndoModify);
}

void CW3ENDOViewMgr::SetCommonToolOnce(const common::CommonToolTypeOnce& type, bool on)
{
	//m_pView3DEndoSagital->SetCommonToolOnce(type, on);
	//m_pView3DEndoSlice->SetCommonToolOnce(type, on);
	m_pView3DENDO->SetCommonToolOnce(type, on);
	m_pView3DEndoModify->SetCommonToolOnce(type, on);
}

void CW3ENDOViewMgr::SetCommonToolOnOff(const common::CommonToolTypeOnOff& type)
{
	//m_pView3DEndoSagital->SetCommonToolOnOff(type);
	//m_pView3DEndoSlice->SetCommonToolOnOff(type);
	m_pView3DENDO->SetCommonToolOnOff(type);
	m_pView3DEndoModify->SetCommonToolOnOff(type);
}

#ifdef WILL3D_EUROPE
void CW3ENDOViewMgr::SetSyncControlButtonOut()
{
	m_pView3DEndoSagital->SetSyncControlButton(false);
	m_pView3DEndoSlice->SetSyncControlButton(false);
	m_pView3DENDO->SetSyncControlButton(false);
	m_pView3DEndoModify->SetSyncControlButton(false);
}
#endif // WILL3D_EUROPE

#ifndef WILL3D_VIEWER
void CW3ENDOViewMgr::exportProject(ProjectIOEndo & out) {
	out.InitEndoTab();

	out.InitializeView(ProjectIOEndo::ViewType::SAGITTAL);
	m_pView3DEndoSagital->exportProject(out);
	out.InitializeView(ProjectIOEndo::ViewType::MODIFY);
	m_pView3DEndoModify->CW3View3D::exportProject(out.GetViewIO());
	out.InitializeView(ProjectIOEndo::ViewType::ENDO);
	m_pView3DENDO->CW3View3D::exportProject(out.GetViewIO());
	out.InitializeView(ProjectIOEndo::ViewType::SLICE);
	m_pView3DEndoSlice->CW3View3D::exportProject(out.GetViewIO());
}

void CW3ENDOViewMgr::importProject(ProjectIOEndo & in) {
	in.InitializeView(ProjectIOEndo::ViewType::SAGITTAL);
	m_pView3DEndoSagital->importProject(in);
	in.InitializeView(ProjectIOEndo::ViewType::MODIFY);
	m_pView3DEndoModify->CW3View3D::importProject(in.GetViewIO());
	in.InitializeView(ProjectIOEndo::ViewType::ENDO);
	m_pView3DENDO->CW3View3D::importProject(in.GetViewIO());
	in.InitializeView(ProjectIOEndo::ViewType::SLICE);
	m_pView3DEndoSlice->CW3View3D::importProject(in.GetViewIO());
}
#endif
void CW3ENDOViewMgr::UpdateVRview(bool is_high_quality) {
  m_pView3DENDO->UpdateVR(is_high_quality);
  m_pView3DEndoModify->UpdateVR(is_high_quality);
}

void CW3ENDOViewMgr::setVisibleViews(bool bVisible) {
	m_pView3DEndoSagital->setVisible(bVisible);
	m_pView3DEndoSlice->setVisible(bVisible);
	m_pView3DENDO->setVisible(bVisible);
	m_pView3DEndoModify->setVisible(bVisible);
}

void CW3ENDOViewMgr::set_task_tool(const std::shared_ptr<EndoTaskTool>& task_tool) {
	task_tool_ = task_tool;

	connect(task_tool_.get(), &EndoTaskTool::sigEndoPlayerAction, m_pView3DENDO, &CW3View3DEndo::slotExploreAction);
	connect(task_tool_.get(), &EndoTaskTool::sigEndoPlayerParam, m_pView3DENDO, &CW3View3DEndo::slotExploreChangeParam);
	connect(task_tool_.get(), &EndoTaskTool::sigEndoFreeOnOff, m_pView3DENDO, &CW3View3DEndo::slotExploreFreeOnOff);
	connect(task_tool_.get(), &EndoTaskTool::sigEndoCamDir, m_pView3DENDO, &CW3View3DEndo::slotSetCamPos);
	connect(task_tool_.get(), &EndoTaskTool::sigEndoSelectPath, m_pView3DEndoSagital, &CW3View3DEndoSagittal::slotSelectPath);
	connect(task_tool_.get(), &EndoTaskTool::sigEndoRemovePath, m_pView3DEndoSagital, &CW3View3DEndoSagittal::slotDeletePath);
	connect(task_tool_.get(), &EndoTaskTool::sigEndoShowPath, this, &CW3ENDOViewMgr::slotVisiblePath);
	connect(task_tool_.get(), &EndoTaskTool::sigEndoVisible, m_pView3DEndoModify, &CW3View3DEndoModify::slotDrawAirway);

	connect(m_pView3DEndoSagital, &CW3View3DEndoSagittal::sigSetEnableEndoPath, task_tool_.get(), &EndoTaskTool::slotEnableRemovePath);
	connect(m_pView3DEndoSagital, &CW3View3DEndoSagittal::sigSetPathNum, this, &CW3ENDOViewMgr::slotSetPathNum);
	connect(m_pView3DENDO, &CW3View3DEndo::sigSetFreeExplorerBtnState, task_tool_.get(), &EndoTaskTool::slotToggleFreeExplorer);

	connect(m_pView3DEndoSagital, &CW3View3DEndoSagittal::sigAirwayDisabled, this, &CW3ENDOViewMgr::slotAirwayDisabled);
	connect(m_pView3DEndoModify, &CW3View3DEndoModify::sigAirwayDisabled, this, &CW3ENDOViewMgr::slotAirwayDisabled);
}

void CW3ENDOViewMgr::reset() {
	m_pView3DEndoSagital->reset();
	m_pView3DEndoSlice->reset();
	m_pView3DENDO->reset();
	m_pView3DEndoModify->reset();
}

void CW3ENDOViewMgr::connections() {
	// from VREngine
	connect(m_pgVRengine, SIGNAL(sigReoriupdate(glm::mat4*)), this, SLOT(slotReoriUpdate(glm::mat4*)));
	connect(m_pgVRengine, SIGNAL(sigTFupdated(bool)), this, SLOT(slotTFupdated(bool)));
	connect(m_pgVRengine, SIGNAL(sigTFupdateCompleted()), this, SLOT(slotTFupdateCompleted()));

	// from view3DEndo
	connect(m_pView3DENDO, SIGNAL(sigSetCameraPos(int, bool)), this, SLOT(slotSetCameraPos(int, bool)));
	connect(m_pView3DENDO, SIGNAL(sigSliceUpdate(mat4, mat4, int, int)), m_pView3DEndoSlice, SLOT(slotSliceUpdate(mat4, mat4, int, int)));
	connect(m_pView3DENDO, SIGNAL(sigSliceReset()), m_pView3DEndoSlice, SLOT(slotSliceReset()));
	connect(m_pView3DENDO, SIGNAL(sigUpdate()), this, SLOT(slotUpdate_3DENDO()));

	// from view3DEndoSagital
	connect(m_pView3DEndoSagital, SIGNAL(sigUpdateEndoPath(CW3Spline3DItem*, bool)), this, SLOT(slotUpdateEndoPath(CW3Spline3DItem*, bool)));
	connect(m_pView3DEndoSagital, SIGNAL(sigSegAirway(std::vector<tri_STL>&)), this, SLOT(slotSegAirway(std::vector<tri_STL>&)));
	connect(m_pView3DEndoSagital, SIGNAL(sigUpdatePoint(int)), this, SLOT(slotUpdatePoint(int)));
	connect(m_pView3DEndoSagital, SIGNAL(sigSetAirwaySize(double)), this, SLOT(slotSetAirwaySize(double)));
	connect(m_pView3DEndoSagital, SIGNAL(sigUpdate()), m_pView3DENDO, SLOT(slotUpdate()));
	connect(m_pView3DEndoSagital, SIGNAL(sigUpdatePick()), m_pView3DEndoModify, SLOT(slotUpdate()));	

	// from view3DEndoModify
	connect(m_pView3DEndoModify, SIGNAL(sigUpdatePoint(int)), this, SLOT(slotUpdatePoint(int)));
	connect(m_pView3DEndoModify, SIGNAL(sigUpdatePick()), m_pView3DEndoSagital, SLOT(slotUpdate()));

	// from view3DEndoSilce
	connect(m_pView3DEndoSlice, SIGNAL(sigWheelEvent(QWheelEvent*)), m_pView3DENDO, SLOT(slotWheelEvent(QWheelEvent*)));

#ifdef WILL3D_EUROPE
	connect(m_pView3DEndoSagital, &CW3View3DEndoSagittal::sigShowButtonListDialog, this, &CW3ENDOViewMgr::sigShowButtonListDialog);
	connect(m_pView3DEndoSlice, &CW3View3DEndoSlice::sigShowButtonListDialog, this, &CW3ENDOViewMgr::sigShowButtonListDialog);
	connect(m_pView3DENDO, &CW3View3DEndo::sigShowButtonListDialog, this, &CW3ENDOViewMgr::sigShowButtonListDialog);
	connect(m_pView3DEndoModify, &CW3View3DEndoModify::sigShowButtonListDialog, this, &CW3ENDOViewMgr::sigShowButtonListDialog);
	
	connect(m_pView3DEndoSagital, &CW3View3DEndoSagittal::sigSyncControlButton, [=](bool is_on)
	{
		m_pView3DEndoSlice->SetSyncControlButton(is_on);
		m_pView3DENDO->SetSyncControlButton(is_on);
		m_pView3DEndoModify->SetSyncControlButton(is_on);
	});
	connect(m_pView3DEndoSlice, &CW3View3DEndoSagittal::sigSyncControlButton, [=](bool is_on)
	{
		m_pView3DEndoSagital->SetSyncControlButton(is_on);
		m_pView3DENDO->SetSyncControlButton(is_on);
		m_pView3DEndoModify->SetSyncControlButton(is_on);
	});
	connect(m_pView3DENDO, &CW3View3DEndoSagittal::sigSyncControlButton, [=](bool is_on)
	{
		m_pView3DEndoSagital->SetSyncControlButton(is_on);
		m_pView3DEndoSlice->SetSyncControlButton(is_on);
		m_pView3DEndoModify->SetSyncControlButton(is_on);
	});
	connect(m_pView3DEndoModify, &CW3View3DEndoSagittal::sigSyncControlButton, [=](bool is_on)
	{
		m_pView3DEndoSagital->SetSyncControlButton(is_on);
		m_pView3DEndoSlice->SetSyncControlButton(is_on);
		m_pView3DENDO->SetSyncControlButton(is_on);
	});

	connect(m_pView3DEndoSagital, &CW3View3DEndoSagittal::sigSpline3DEnd, this, &CW3ENDOViewMgr::sigSpline3DEnd);
#endif // WILL3D_EUROPE
}

void CW3ENDOViewMgr::slotSetPathNum(int path) {
	task_tool_->blockSignals(true);
	task_tool_->SyncSetPath(static_cast<EndoPathID>(path), true);
	task_tool_->blockSignals(false);
}

void CW3ENDOViewMgr::slotAirwayDisabled(bool is_draw) {
	task_tool_->blockSignals(true);
	bool is_enable = !is_draw;
	task_tool_->SetAirwayEnable(is_enable);
	task_tool_->blockSignals(false);

	emit sigAirwayEnable(is_enable);
}

void CW3ENDOViewMgr::slotSetAirwaySize(double size) {
	task_tool_->SetAirwaySize(size);
	emit sigSetAirwaySize(size);
}

void CW3ENDOViewMgr::slotReoriUpdate(glm::mat4* m) {
	m_pView3DEndoSagital->reoriUpdate(m);
	m_pView3DEndoModify->slotReoriupdate(m);
	m_pView3DENDO->reoriUpdate(m);
	m_pView3DEndoSlice->reoriUpdate(m);
}

void CW3ENDOViewMgr::slotTFupdated(bool bUpdate) {
	m_pView3DENDO->slotTFupdated(bUpdate);
	m_pView3DEndoModify->slotTFupdated(bUpdate);
}

void CW3ENDOViewMgr::slotTFupdateCompleted(void) {
	m_pView3DENDO->slotTFupdateCompleted();
	m_pView3DEndoModify->slotTFupdateCompleted();
}

void CW3ENDOViewMgr::slotSetCameraPos(const int nPos, const bool bFixed) {
	m_pView3DEndoModify->slotSetCameraPos(nPos, bFixed);
	m_pView3DEndoSagital->slotSetCameraPos(nPos, bFixed);
}

void CW3ENDOViewMgr::slotUpdate_3DENDO(void) {
	m_pView3DEndoModify->slotUpdate();
	m_pView3DEndoSagital->slotUpdate();
}

void CW3ENDOViewMgr::slotUpdateEndoPath(CW3Spline3DItem* path, const bool bReset) {
	m_pView3DEndoModify->slotUpdateEndoPath(path, bReset);
	m_pView3DENDO->slotUpdateEndoPath(path, bReset);
}

void CW3ENDOViewMgr::slotSegAirway(std::vector<tri_STL>& vSTL) {
	m_pView3DEndoModify->slotSegAirway(vSTL);
	emit sigSetAirway(vSTL);
}

void CW3ENDOViewMgr::slotVisiblePath(int state) {
	m_pView3DEndoSagital->setVisiblePath(state);
	m_pView3DEndoModify->setVisiblePath(state);
}

void CW3ENDOViewMgr::slotUpdatePoint(int index) {
	QObject* pSender = QObject::sender();
	if (pSender == m_pView3DEndoModify)
		m_pView3DEndoSagital->slotUpdatePoint(index);
	else if (pSender == m_pView3DEndoSagital)
		m_pView3DEndoModify->slotUpdatePoint(index);

	m_pView3DENDO->slotUpdatePoint(index);
}

void CW3ENDOViewMgr::ApplyPreferences() {
	m_pView3DEndoSagital->ApplyPreferences();
	m_pView3DEndoSlice->ApplyPreferences();
	m_pView3DENDO->ApplyPreferences();
	m_pView3DEndoModify->ApplyPreferences();
}

void CW3ENDOViewMgr::DeleteMeasureUI(const common::ViewTypeID & view_type,
									 const unsigned int & measure_id) {
	switch (view_type) {
	case common::ViewTypeID::ENDO_SAGITTAL:
		m_pView3DEndoSagital->DeleteMeasureUI(measure_id);
		break;
	case common::ViewTypeID::ENDO_SLICE:
		m_pView3DEndoSlice->DeleteMeasureUI(measure_id);
		break;
	case common::ViewTypeID::ENDO:
		m_pView3DENDO->DeleteMeasureUI(measure_id);
		break;
	case common::ViewTypeID::ENDO_MODIFY:
		m_pView3DEndoModify->DeleteMeasureUI(measure_id);
		break;
	}
}
