#include "W3SIViewMgr.h"
/*=========================================================================

File:			class CW3SIViewMgr
Language:		C++11
Library:		Qt 5.4.0
Author:			Hong Jung
First date:		2015-11-26
Last modify:	2016-04-22

=========================================================================*/
#include "../../Engine/Common/Common/W3Memory.h"
#ifndef WILL3D_VIEWER
#include "../../Engine/Core/W3ProjectIO/project_io_si.h"
#endif
#include "../../Engine/UIModule/UIComponent/W3ViewMPR.h"
#include "../../Engine/UIModule/UIComponent/W3View3DSI.h"
#include "../../Engine/Module/VREngine/W3VREngine.h"
#include "../../Engine/Module/MPREngine/W3MPREngine.h"

CW3SIViewMgr::CW3SIViewMgr(CW3VREngine *VREngine, CW3MPREngine *MPRengine,
	CW3ResourceContainer *Rcontainer, QWidget *parent)
	: m_pgVRengine(VREngine), m_pgMPRengine(MPRengine) {
	m_pViewMPR[MPRViewType::AXIAL] = new CW3ViewMPR(m_pgVRengine, m_pgMPRengine, Rcontainer,
		common::ViewTypeID::MPR_AXIAL, MPRViewType::AXIAL, this);
	m_pViewMPR[MPRViewType::SAGITTAL] = new CW3ViewMPR(m_pgVRengine, m_pgMPRengine, Rcontainer,
		common::ViewTypeID::MPR_SAGITTAL, MPRViewType::SAGITTAL, this);
	m_pViewMPR[MPRViewType::CORONAL] = new CW3ViewMPR(m_pgVRengine, m_pgMPRengine, Rcontainer,
		common::ViewTypeID::MPR_CORONAL, MPRViewType::CORONAL, this);
	for (int i = 0; i < MPRViewType::MPR_END; ++i) {
		m_pViewMPR[i]->SIModeOn(true);
	}
	m_pViewSI = new CW3View3DSI(m_pgVRengine, m_pgMPRengine, Rcontainer,
		common::ViewTypeID::SUPERIMPOSITION, this);
	connections();
}

CW3SIViewMgr::~CW3SIViewMgr() {
	for (int i = 0; i < MPRViewType::MPR_END; ++i)
		SAFE_DELETE_OBJECT(m_pViewMPR[i]);
	SAFE_DELETE_OBJECT(m_pViewSI);
}
#ifndef WILL3D_VIEWER
void CW3SIViewMgr::exportProject(ProjectIOSI & out) {
	out.InitializeView(ProjectIOSI::ViewType::AXIAL);
	m_pViewMPR[MPRViewType::AXIAL]->exportProject(out);

	out.InitializeView(ProjectIOSI::ViewType::SAGITTAL);
	m_pViewMPR[MPRViewType::SAGITTAL]->exportProject(out);

	out.InitializeView(ProjectIOSI::ViewType::CORONAL);
	m_pViewMPR[MPRViewType::CORONAL]->exportProject(out);

	out.InitializeView(ProjectIOSI::ViewType::VR);
	m_pViewSI->exportProject(out);
}

void CW3SIViewMgr::importProject(ProjectIOSI & in) {
	in.InitializeView(ProjectIOSI::ViewType::AXIAL);
	m_pViewMPR[MPRViewType::AXIAL]->importProject(in);

	in.InitializeView(ProjectIOSI::ViewType::SAGITTAL);
	m_pViewMPR[MPRViewType::SAGITTAL]->importProject(in);

	in.InitializeView(ProjectIOSI::ViewType::CORONAL);
	m_pViewMPR[MPRViewType::CORONAL]->importProject(in);

	in.InitializeView(ProjectIOSI::ViewType::VR);
	m_pViewSI->importProject(in);
}
#endif
void CW3SIViewMgr::UpdateVRview(bool is_high_quality) {
	m_pViewSI->UpdateVR(is_high_quality);
}
void CW3SIViewMgr::SetCommonToolOnce(const common::CommonToolTypeOnce & type, bool on) {
	for (int i = 0; i < MPRViewType::MPR_END; ++i)
		m_pViewMPR[i]->SetCommonToolOnce(type, on);
	m_pViewSI->SetCommonToolOnce(type, on);
}

void CW3SIViewMgr::SetCommonToolOnOff(const common::CommonToolTypeOnOff & type) {
	for (int i = 0; i < MPRViewType::MPR_END; ++i)
		m_pViewMPR[i]->SetCommonToolOnOff(type);
	m_pViewSI->SetCommonToolOnOff(type);
}

void CW3SIViewMgr::reset() {
	for (int i = 0; i < MPRViewType::MPR_END; ++i)
		m_pViewMPR[i]->reset();
	m_pViewSI->reset();

	m_isInitialized = false;
}

void CW3SIViewMgr::activate() {
	if (!m_isInitialized) {
		for (int i = 0; i < MPRViewType::MPR_END; ++i) {
			m_pViewMPR[i]->initViewPlane(0);
			m_pViewMPR[i]->InitialDraw();
		}

		m_isInitialized = true;
	}
	else {
		for (int i = 0; i < MPRViewType::MPR_END; ++i) {
			m_pViewMPR[i]->drawImageOnViewPlane(false, 0, false);
			m_pViewMPR[i]->scene()->update();
		}
	}

	for (int i = 0; i < MPRViewType::MPR_END; ++i) {
		m_pViewMPR[i]->SetVisibleMPRUIs(false);
		m_pViewMPR[i]->SetVisibleRects(true);
		m_pViewMPR[i]->setDirectionText();
	}
}

void CW3SIViewMgr::drawSecond(const glm::mat4& mat) {
	m_pViewSI->SecondVolumeLoaded(mat);
	for (int i = 0; i < MPRViewType::MPR_END; ++i)
		m_pViewMPR[i]->VisibleSecond(Qt::CheckState::Checked);
}

void CW3SIViewMgr::setVisible(bool bVisible) {
	m_pViewSI->setVisible(bVisible);
	for (int i = 0; i < MPRViewType::MPR_END; ++i)
		m_pViewMPR[i]->setVisible(bVisible);
}

void CW3SIViewMgr::VisibleMain(bool b) {
	m_pViewSI->VisibleMain(b);
	for (int i = 0; i < MPRViewType::MPR_END; ++i)
		m_pViewMPR[i]->VisibleVolMain(b);
}

void CW3SIViewMgr::VisibleSecond(bool b) {
	m_pViewSI->VisibleSecond(b);
	for (int i = 0; i < MPRViewType::MPR_END; ++i)
		m_pViewMPR[i]->VisibleVolSecond(b);
}

void CW3SIViewMgr::VisibleBoth(bool b) {
	m_pViewSI->VisibleBoth(b);
	for (int i = 0; i < MPRViewType::MPR_END; ++i)
		m_pViewMPR[i]->VisibleVolBoth(b);
}

#ifdef WILL3D_EUROPE
void CW3SIViewMgr::SetSyncControlButtonOut()
{
	for (int i = 0; i < MPRViewType::MPR_END; ++i)
	{
		m_pViewMPR[i]->SetSyncControlButton(false);
	}

	m_pViewSI->SetSyncControlButton(false);
}
#endif // WILL3D_EUROPE

void CW3SIViewMgr::slotReoriUpdate(glm::mat4* mat) {
	m_pViewSI->slotReoriupdate(mat);
	for (int i = 0; i < MPRViewType::MPR_END; ++i)
		m_pViewMPR[i]->slotReoriupdate(mat);
}

void CW3SIViewMgr::slotSecondUpdate(glm::mat4* mat) {
	for (int i = 0; i < MPRViewType::MPR_END; ++i)
		m_pViewMPR[i]->SecondUpdate(mat);
}

void CW3SIViewMgr::slotWheel(MPRViewType eType, float fDelta) {
	QObject* pSender = QObject::sender();
	for (int i = 0; i < MPRViewType::MPR_END; ++i) {
		if (pSender != m_pViewMPR[i])
			m_pViewMPR[i]->WheelView(eType, fDelta);
	}
}

void CW3SIViewMgr::connections()
{
	connect(m_pgVRengine, &CW3VREngine::sigReoriupdate, this, &CW3SIViewMgr::slotReoriUpdate);
	connect(m_pgVRengine, &CW3VREngine::sigTFupdated, m_pViewSI, &CW3View3DSI::slotTFupdated);
	connect(m_pgVRengine, &CW3VREngine::sigTFupdateCompleted, m_pViewSI, &CW3View3DSI::slotTFupdateCompleted);
	connect(m_pgMPRengine, &CW3MPREngine::sigSecondUpdate, this, &CW3SIViewMgr::slotSecondUpdate);

	for (int i = 0; i < MPRViewType::MPR_END; ++i)
	{
		connect(m_pViewMPR[i], &CW3ViewMPR::sigWheel, this, &CW3SIViewMgr::slotWheel);
	}

	connect(m_pViewSI, &CW3View3DSI::sigSetTranslateMatSecondVolume, this, &CW3SIViewMgr::sigSetTranslateMatSecondVolume);
	connect(m_pViewSI, &CW3View3DSI::sigSetRotateMatSecondVolume, this, &CW3SIViewMgr::sigSetRotateMatSecondVolume);

#ifdef WILL3D_EUROPE
	connect(m_pViewSI, &CW3View3DSI::sigShowButtonListDialog, this, &CW3SIViewMgr::sigShowButtonListDialog);
	for (int i = 0; i < MPRViewType::MPR_END; ++i)
	{
		connect(m_pViewMPR[i], &CW3ViewMPR::sigShowButtonListDialog, this, &CW3SIViewMgr::sigShowButtonListDialog);
		connect(m_pViewMPR[i], &CW3ViewMPR::sigSyncControlButton, [=](bool is_on)
		{
			m_pViewSI->SetSyncControlButton(is_on);
			for (int j = 0; j < MPRViewType::MPR_END; ++j)
			{
				if (i == j)
				{
					continue;
				}

				m_pViewMPR[j]->SetSyncControlButton(is_on);
			}
		});
	}

	connect(m_pViewSI, &CW3View3DSI::sigSyncControlButton, [=](bool is_on)
	{
		for (int i = 0; i < MPRViewType::MPR_END; ++i)
		{
			m_pViewMPR[i]->SetSyncControlButton(is_on);
		}
	});
#endif // WILL3D_EUROPE
}

void CW3SIViewMgr::ApplyPreferences() {
	for (int i = 0; i < MPRViewType::MPR_END; ++i)
		m_pViewMPR[i]->ApplyPreferences();
	m_pViewSI->ApplyPreferences();
}

void CW3SIViewMgr::ResetMatrixToAuto() {
	m_pViewSI->ResetMatrixToAuto();
}

void CW3SIViewMgr::DeleteMeasureUI(const common::ViewTypeID & view_type,
	const unsigned int & measure_id) {
	switch (view_type) {
	case common::ViewTypeID::MPR_AXIAL:
		m_pViewMPR[MPRViewType::AXIAL]->DeleteMeasureUI(measure_id);
		break;
	case common::ViewTypeID::MPR_SAGITTAL:
		m_pViewMPR[MPRViewType::SAGITTAL]->DeleteMeasureUI(measure_id);
		break;
	case common::ViewTypeID::MPR_CORONAL:
		m_pViewMPR[MPRViewType::CORONAL]->DeleteMeasureUI(measure_id);
		break;
	case common::ViewTypeID::SUPERIMPOSITION:
		m_pViewSI->DeleteMeasureUI(measure_id);
		break;
	}
}
