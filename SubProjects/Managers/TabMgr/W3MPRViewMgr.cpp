#include "W3MPRViewMgr.h"
/*=========================================================================

File:			class CW3MPRViewMgr
Language:		C++11
Library:		Qt 5.2.0
Author:			Hong Jung
First date:		2015-11-26
Last modify:	2016-04-14

=========================================================================*/
#include <ctime>

#if defined(__APPLE__)
#include </usr/local/Cellar/llvm/5.0.0/lib/clang/5.0.0/include/omp.h>
#else
#include <omp.h>
#endif
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc.hpp"

#include <QDebug>
#include <QKeyEvent>
#include <QtConcurrent/QtConcurrent>

#include <Engine/Common/Common/W3Define.h>
#include <Engine/Common/Common/W3Logger.h>
#include <Engine/Common/Common/W3Memory.h>
#include <Engine/Common/Common/W3MessageBox.h>
#include <Engine/Common/Common/W3ProgressDialog.h>
#include <Engine/Common/Common/define_measure.h>
#include <Engine/Common/Common/event_handle_common.h>
#include <Engine/Common/Common/event_handler.h>
#include <Engine/Common/Common/global_preferences.h>

#include <Engine/Resource/ResContainer/resource_container.h>
#include <Engine/Resource/Resource/W3Image3D.h>
#include <Engine/Resource/Resource/W3TF.h>
#include <Engine/Resource/Resource/lightbox_resource.h>

#include <Engine/Core/Surfacing/MarchingCube.h>
#include <Engine/Core/Surfacing/W3MeshSimplifier.h>
#ifndef WILL3D_VIEWER
#include <Engine/Core/W3ProjectIO/project_io_mpr.h>
#endif
#include <Engine/UIModule/UITools/datatypes.h>
#include <Engine/UIModule/UITools/mpr_task_tool.h>

#include <Engine/UIModule/UIComponent/W3View3DMPR.h>
#include <Engine/UIModule/UIComponent/W3ViewMPR.h>
#include <Engine/UIModule/UIComponent/view_lightbox.h>
#include <Engine/UIModule/UIComponent/view_mpr_orientation.h>
#include <Engine/UIModule/UIFrame/generate_face_dialog.h>
#include <Engine/UIModule/UIFrame/stl_export_dialog.h>

#include <Engine/Module/MPREngine/W3MPREngine.h>
#include <Engine/Module/VREngine/W3VREngine.h>
#include <Engine/Module/Panorama/pano_engine.h>

CW3MPRViewMgr::CW3MPRViewMgr(CW3VREngine* VREngine, CW3MPREngine* MPRengine,
	CW3ResourceContainer* Rcontainer, QWidget* parent)
	: m_pgVRengine(VREngine), m_pgMPRengine(MPRengine)
{
	m_pViewMPR[MPRViewType::AXIAL].reset(
		new CW3ViewMPR(m_pgVRengine, m_pgMPRengine, Rcontainer,
			common::ViewTypeID::MPR_AXIAL, MPRViewType::AXIAL, this));
	m_pViewMPR[MPRViewType::SAGITTAL].reset(new CW3ViewMPR(
		m_pgVRengine, m_pgMPRengine, Rcontainer, common::ViewTypeID::MPR_SAGITTAL,
		MPRViewType::SAGITTAL, this));
	m_pViewMPR[MPRViewType::CORONAL].reset(new CW3ViewMPR(
		m_pgVRengine, m_pgMPRengine, Rcontainer, common::ViewTypeID::MPR_CORONAL,
		MPRViewType::CORONAL, this));

	m_pView3DMPR.reset(new CW3View3DMPR(m_pgVRengine, m_pgMPRengine, Rcontainer,
		common::ViewTypeID::MPR_3D, this));

	m_pViewZoom3D.reset(new CW3View3DMPR(m_pgVRengine, m_pgMPRengine, Rcontainer,
		common::ViewTypeID::MPR_ZOOM3D, this));

	for (int id = 0; id < ReorientViewID::REORI_VIEW_END; ++id)
	{
		ReorientViewID view_type = static_cast<ReorientViewID>(id);
		view_orientation_[id].reset(new ViewMPROrientation(view_type));
		view_orientation_[id]->setVisible(false);
	}

	for (int i = 0; i < kRegistrationParamCnt; ++i)
	{
		m_RegiPointsSol[i] = 0.0f;
	}

	connections();
}

CW3MPRViewMgr::~CW3MPRViewMgr() {}

void CW3MPRViewMgr::set_task_tool(const std::weak_ptr<MPRTaskTool>& tool)
{
	task_tool_ = tool;
	task_tool_.lock()->Connections();
	slotSetMPRClipStatus();
}

#ifndef WILL3D_VIEWER
void CW3MPRViewMgr::exportProject(ProjectIOMPR& out)
{
	out.InitMPRTab();
	out.SaveRegistrationParams(m_RegiPointsSol, kRegistrationParamCnt);

	out.InitializeView(ProjectIOMPR::ViewType::AXIAL);
	m_pViewMPR[MPRViewType::AXIAL]->exportProject(out);

	out.InitializeView(ProjectIOMPR::ViewType::SAGITTAL);
	m_pViewMPR[MPRViewType::SAGITTAL]->exportProject(out);

	out.InitializeView(ProjectIOMPR::ViewType::CORONAL);
	m_pViewMPR[MPRViewType::CORONAL]->exportProject(out);

	out.InitializeView(ProjectIOMPR::ViewType::VR);
	m_pView3DMPR->exportProject(out);
	out.InitializeView(ProjectIOMPR::ViewType::ZOOM3D);
	m_pViewZoom3D->exportProject(out);

	if (mpr_transform_measure_.size() > 0)
	{
		out.SaveTransformStatusCount(mpr_transform_measure_.size());
		for (const auto& transform_status : mpr_transform_measure_)
		{
			const auto& status = transform_status.second;
			out.SaveTransformStatusForMeasure(
				transform_status.first, status.axial, status.sagittal, status.coronal,
				status.rotate_center, status.axial_angle_degree,
				status.sagittal_angle_degree, status.coronal_angle_degree);
		}
	}
}

void CW3MPRViewMgr::importProject(ProjectIOMPR& in)
{
	in.LoadRegistrationParams(m_RegiPointsSol);

	in.InitializeView(ProjectIOMPR::ViewType::AXIAL);
	m_pViewMPR[MPRViewType::AXIAL]->importProject(in);

	in.InitializeView(ProjectIOMPR::ViewType::SAGITTAL);
	m_pViewMPR[MPRViewType::SAGITTAL]->importProject(in);

	in.InitializeView(ProjectIOMPR::ViewType::CORONAL);
	m_pViewMPR[MPRViewType::CORONAL]->importProject(in);

	in.InitializeView(ProjectIOMPR::ViewType::VR);
	m_pView3DMPR->importProject(in);
	in.InitializeView(ProjectIOMPR::ViewType::ZOOM3D);
	m_pViewZoom3D->importProject(in);

	int mpr_transform_measure_cnt;
	in.LoadTransformStatusCount(mpr_transform_measure_cnt);
	for (int i = 0; i < mpr_transform_measure_cnt; ++i)
	{
		MPRTansfromStatusForMeasure status;
		unsigned int measure_id;
		in.LoadTransformStatusForMeasure(
			measure_id, status.axial, status.sagittal, status.coronal,
			status.rotate_center, status.axial_angle_degree,
			status.sagittal_angle_degree, status.coronal_angle_degree);

		mpr_transform_measure_[measure_id] = status;
	}
}

void CW3MPRViewMgr::RequestedCreateLightBoxDCMFiles(const bool nerve_visible, const bool implant_visible, const int filter, const int thickness)
{
	if (view_lightbox_.empty())
	{
		return;
	}

	QTime time = QTime::currentTime();
	QString str = QString::number(time.msecsSinceStartOfDay()) + "_";
	for (int i = 0; i < lightbox_count_; ++i)
	{
		view_lightbox_[i]->RequestedCreateDCMFiles(str, nerve_visible, implant_visible, filter, thickness);
	}
}

const int CW3MPRViewMgr::GetLightBoxFilterValue() const
{
	return view_lightbox_.front()->GetFilterValue();
}
#endif

void CW3MPRViewMgr::reset()
{
	for (int i = 0; i < MPRViewType::MPR_END; ++i) m_pViewMPR[i]->reset();

	m_pView3DMPR->reset();
	m_pViewZoom3D->reset();

	m_isInitialized_ = false;
	is_visible_canal_ = false;

	slotSetMPRClipStatus();

	mpr_transform_measure_.clear();
}

void CW3MPRViewMgr::activate()
{
	if (!m_isInitialized_)
	{
		for (int i = 0; i < MPRViewType::MPR_END; ++i)
		{
			m_pViewMPR[i]->initViewPlane(0);
			m_pViewMPR[i]->InitialDraw();

			double default_interval = GlobalPreferences::GetInstance()->preferences_.advanced.mpr.default_interval;
			if (static_cast<int>(default_interval) == 0)
			{
				m_pViewMPR[i]->SetThicknessMaximumRange(common::kMPRThicknessMax / m_pgVRengine->getVol(0)->pixelSpacing());
			}
			else
			{
				m_pViewMPR[i]->SetThicknessMaximumRange(common::kMPRThicknessMax / default_interval);
			}
		}

		m_isInitialized_ = true;
	}
	else
	{
		for (int i = 0; i < MPRViewType::MPR_END; ++i)
		{
			m_pViewMPR[i]->drawImageOnViewPlane(false, 0, is_visible_canal_);
			m_pViewMPR[i]->scene()->update();
		}
	}
}

void CW3MPRViewMgr::SetCommonToolOnce(const common::CommonToolTypeOnce& type, bool on)
{
	m_pView3DMPR->SetCommonToolOnce(type, on);
	if (m_pView3DMPR->only_trd_mode())
	{
		return;
	}

	for (int i = 0; i < MPRViewType::MPR_END; ++i)
		m_pViewMPR[i]->SetCommonToolOnce(type, on);
	if (m_pViewZoom3D) m_pViewZoom3D->SetCommonToolOnce(type, on);

	if (!view_lightbox_.empty())
	{
		for (auto& view : view_lightbox_) view->SetCommonToolOnce(type, on);
	}
}

void CW3MPRViewMgr::SetCommonToolOnOff(const common::CommonToolTypeOnOff& type)
{
	m_pView3DMPR->SetCommonToolOnOff(type);
	if (m_pView3DMPR->only_trd_mode())
	{
		return;
	}

	for (int i = 0; i < MPRViewType::MPR_END; ++i)
		m_pViewMPR[i]->SetCommonToolOnOff(type);
	if (m_pViewZoom3D) m_pViewZoom3D->SetCommonToolOnOff(type);

	if (!view_lightbox_.empty())
	{
		for (auto& view : view_lightbox_) view->SetCommonToolOnOff(type);
	}
}

void CW3MPRViewMgr::UpdateVRview(bool is_high_quality)
{
	m_pView3DMPR->UpdateVR(is_high_quality);
	m_pViewZoom3D->UpdateVR(is_high_quality);
}

void CW3MPRViewMgr::connections()
{
	connect(m_pgVRengine, &CW3VREngine::sigReoriupdate, this, &CW3MPRViewMgr::slotReoriUpdateVR);
	connect(m_pgVRengine, &CW3VREngine::sigTFupdated, m_pView3DMPR.get(), &CW3View3DMPR::slotTFupdated);
	connect(m_pgVRengine, &CW3VREngine::sigTFupdateCompleted, this, &CW3MPRViewMgr::slotTFUpdateCompleted);

	connect(m_pgMPRengine, &CW3MPREngine::sigReoriupdate, this, &CW3MPRViewMgr::slotReoriUpdateMPR);
	connect(m_pgMPRengine, &CW3MPREngine::sigSecondUpdate, this, &CW3MPRViewMgr::slotSecondUpdate);

	connect(m_pViewMPR[MPRViewType::AXIAL].get(),
		SIGNAL(sigUpdateArch(ArchTypeID, std::vector<glm::vec3>, glm::mat4, int)),
		this,
		SIGNAL(sigUpdateArch(ArchTypeID, std::vector<glm::vec3>, glm::mat4, int)));

	for (int i = 0; i < MPRViewType::MPR_END; ++i)
	{
		const auto view_mpr = m_pViewMPR[i].get();
		connect(view_mpr, &CW3ViewMPR::sigRotate, this, &CW3MPRViewMgr::slotRotate);
		connect(view_mpr, &CW3ViewMPR::sigTranslate, this, &CW3MPRViewMgr::slotTranslate);
		connect(view_mpr, &CW3ViewMPR::sigTranslateCross, this, &CW3MPRViewMgr::slotTranslateCross);
		connect(view_mpr, &CW3ViewMPR::sigThickChange, this, &CW3MPRViewMgr::slotThickChange);
		connect(view_mpr, &CW3ViewMPR::sigSelectedMPR, this, &CW3MPRViewMgr::slotSelectedMPR);
		connect(view_mpr, &CW3ViewMPR::sigWheel, this, &CW3MPRViewMgr::slotWheel);

		connect(view_mpr, &CW3ViewMPR::sigSetZoom3DMPR, this, &CW3MPRViewMgr::slotSetZoom3DMPR);
		connect(view_mpr, &CW3ViewMPR::sigSetZoom3DVR, this, &CW3MPRViewMgr::slotSetZoom3DVR);

		for (int j = 0; j < MPRViewType::MPR_END; ++j)
		{
			if (i != j)
			{
				const auto view_mpr_target = m_pViewMPR[j].get();
				connect(view_mpr, &CW3ViewMPR::sigSyncScale, view_mpr_target, &CW3ViewMPR::slotSyncScale);
				connect(view_mpr, &CW3ViewMPR::sigSyncWindowing, view_mpr_target, &CW3ViewMPR::slotSyncWindowing);
			}
		}

		connect(view_mpr, &CW3ViewMPR::sigMeasureCreated, this, &CW3MPRViewMgr::slotMPRMeasureCreated);
		connect(view_mpr, &CW3ViewMPR::sigMeasureDeleted, this, &CW3MPRViewMgr::slotMPRMeasureDeleted);

		connect(view_mpr, &CW3ViewMPR::sigSelectImplant, this, &CW3MPRViewMgr::slotSelectImplant);
		connect(view_mpr, &CW3ViewMPR::sigTranslateImplant, this, &CW3MPRViewMgr::slotTranslateImplant);
		connect(view_mpr, &CW3ViewMPR::sigRotateImplant, this, &CW3MPRViewMgr::slotRotateImplant);
		connect(view_mpr, &CW3ViewMPR::sigDoneAdjustImplant, this, &CW3MPRViewMgr::slotDoneAdjustImplant);

		connect(view_mpr, &CW3ViewMPR::sigMouseRelease, this, &CW3MPRViewMgr::slotUpdateMPROverlay);

#ifdef WILL3D_EUROPE
		connect(view_mpr, &CW3ViewMPR::sigShowButtonListDialog, this, [=](const QPoint& global_pos)
		{
			emit sigShowButtonListDialog(global_pos, view_mpr->mouse_orver() ? i : -1);
		});
		connect(view_mpr, &CW3ViewMPR::sigSyncControlButton, [=](bool is_on)
		{
			for (int j = 0; j < MPRViewType::MPR_END; ++j)
			{
				if (i != j)
				{
					auto view_mpr_target = m_pViewMPR[j].get();
					view_mpr_target->SetSyncControlButton(is_on);
				}
			}

			m_pView3DMPR.get()->SetSyncControlButton(is_on);
			m_pViewZoom3D.get()->SetSyncControlButton(is_on);
		});
#endif // WILL3D_EUROPE
	}

#ifdef WILL3D_EUROPE
	connect(m_pView3DMPR.get(), &CW3View3DMPR::sigShowButtonListDialog, this, &CW3MPRViewMgr::sigShowButtonListDialog);
	connect(m_pView3DMPR.get(), &CW3View3DMPR::sigSyncControlButton, [=](bool is_on)
	{
		for (int i = 0; i < MPRViewType::MPR_END; ++i)
		{
			auto view_mpr = m_pViewMPR[i].get();
			view_mpr->SetSyncControlButton(is_on);
		}
		m_pViewZoom3D.get()->SetSyncControlButton(is_on);
	});

	connect(m_pViewZoom3D.get(), &CW3View3DMPR::sigShowButtonListDialog, this, &CW3MPRViewMgr::sigShowButtonListDialog);
	connect(m_pViewZoom3D.get(), &CW3View3DMPR::sigSyncControlButton, [=](bool is_on)
	{
		for (int i = 0; i < MPRViewType::MPR_END; ++i)
		{
			auto view_mpr = m_pViewMPR[i].get();
			view_mpr->SetSyncControlButton(is_on);
		}
		m_pViewZoom3D.get()->SetSyncControlButton(is_on);
	});
#endif // WILL3D_EUROPE

	connect(m_pView3DMPR.get(), &CW3View3DMPR::sigRequestClipStatus, this, &CW3MPRViewMgr::slotSetMPRClipStatus);
	connect(m_pView3DMPR.get(), &CW3View3DMPR::sigMPROverlayOn, this, &CW3MPRViewMgr::sigMPROverlayOn);
	connect(m_pView3DMPR.get(), SIGNAL(sigSave3DFaceToPLYFile()), this, SIGNAL(sigSave3DFaceToPLYFile()));
	connect(m_pView3DMPR.get(), SIGNAL(sigSave3DFaceToOBJFile()), this, SIGNAL(sigSave3DFaceToOBJFile()));
	connect(m_pView3DMPR.get(), &CW3View3DMPR::sigVolumeClicked, this, &CW3MPRViewMgr::slotVolumeClicked);

	for (int id = 0; id < ReorientViewID::REORI_VIEW_END; ++id)
	{
		const auto& view = view_orientation_[id].get();
		connect(view, SIGNAL(sigRotateMat(ReorientViewID, glm::mat4)), this, SLOT(slotRotateOrientationView(ReorientViewID, glm::mat4)));
		connect(view, &BaseViewOrientation::sigRenderQuality, this, &CW3MPRViewMgr::slotSetOrientationViewRenderQuality);
	}
}

void CW3MPRViewMgr::slotSetZoom3DMPR(const MPRViewType eViewType,
	const QPointF center, const float radius,
	const bool updateVR)
{
	switch (eViewType)
	{
	case MPRViewType::AXIAL:
		m_pViewMPR[MPRViewType::SAGITTAL]->setZoom3DMPR(radius, false);
		break;
	case MPRViewType::SAGITTAL:
		m_pViewMPR[MPRViewType::AXIAL]->setZoom3DMPR(radius, updateVR);
		break;
	case MPRViewType::CORONAL:
		m_pViewMPR[MPRViewType::SAGITTAL]->setZoom3DMPR(radius, false);
		m_pViewMPR[MPRViewType::AXIAL]->setZoom3DMPR(radius, updateVR);
		break;
	}
}

void CW3MPRViewMgr::slotRotateMatrix(MPRViewType eType, glm::mat4* T)
{
	glm::mat4 T2;
	m_pViewMPR[eType]->rotate3DDraw(&T2, T);

	for (int view = 0; view < MPRViewType::MPR_END; view++)
	{
		if (view != eType) m_pViewMPR[view]->rotate3DDrawPassive(T2);
	}
}

void CW3MPRViewMgr::slotRotate(UILineType eLineType, MPRViewType eType,
	const glm::vec3& UpVec, float radian)
{
	for (int view = 0; view < MPRViewType::MPR_END; view++)
	{
		if (view != eType)
		{
			m_pViewMPR[view]->rotateDraw(UpVec, radian);
			glm::mat4 rotMat = glm::rotate((float)(radian), UpVec);
			m_pViewMPR[view]->rotateZoom3DCube(rotMat);
		}
		else
		{
			m_pViewMPR[eType]->rotateItems(radian);
		}
	}

	if (m_pView3DMPR->isDrawMPROverlay())
	{
		m_pView3DMPR->setMPROverlay(
			m_pViewMPR[Line2View[eType][eLineType]]->getPlaneEquation(),
			m_pViewMPR[Line2View[eType][eLineType]]->getPlaneRightVector()
		);
	}
}

void CW3MPRViewMgr::slotTranslate(MPRViewType eType, MPRViewType eLineType, float fDist)
{
	bool inverse_sagittal = GlobalPreferences::GetInstance()->preferences_.advanced.mpr.sagittal_direction == GlobalPreferences::Direction::Inverse;

	float fittedDist = 0.0f;
	switch (eType)
	{
	case MPRViewType::AXIAL:
		if (eLineType == MPRViewType::CORONAL)
		{
			fittedDist = m_pViewMPR[eLineType]->translateDraw(fDist);
		}
		else
		{
			fittedDist = m_pViewMPR[eLineType]->translateDraw(inverse_sagittal ? fDist : -fDist);
			if (!inverse_sagittal)
			{
				fittedDist *= -1.0f;
			}
		}
		break;
	case MPRViewType::SAGITTAL:
		if (eLineType == MPRViewType::AXIAL)
		{
			fittedDist = -m_pViewMPR[eLineType]->translateDraw(-fDist);
		}
		else
		{
			fittedDist = m_pViewMPR[eLineType]->translateDraw(inverse_sagittal ? -fDist : fDist);
			if (inverse_sagittal)
			{
				fittedDist *= -1.0f;
			}
		}
		break;
	case MPRViewType::CORONAL:
		if (eLineType == MPRViewType::AXIAL)
		{
			fittedDist = -m_pViewMPR[eLineType]->translateDraw(-fDist);
		}
		else
		{
			fittedDist = m_pViewMPR[eLineType]->translateDraw(inverse_sagittal ? fDist : -fDist);
			if (!inverse_sagittal)
			{
				fittedDist *= -1.0f;
			}
		}
		break;
	}

	switch (eType)
	{
	case MPRViewType::AXIAL:
		if (eLineType == MPRViewType::CORONAL)
		{
			m_pViewMPR[eType]->translateCrossPt(UILineType::HORIZONTAL, fittedDist);
			m_pViewMPR[MPRViewType::SAGITTAL]->translateCrossPt(UILineType::VERTICAL, inverse_sagittal ? -fittedDist : fittedDist);
		}
		else
		{
			m_pViewMPR[eType]->translateCrossPt(UILineType::VERTICAL, fittedDist);
			m_pViewMPR[MPRViewType::CORONAL]->translateCrossPt(UILineType::VERTICAL, fittedDist);
		}
		break;
	case MPRViewType::SAGITTAL:
		if (eLineType == MPRViewType::AXIAL)
		{
			m_pViewMPR[eType]->translateCrossPt(UILineType::HORIZONTAL, fittedDist);
			m_pViewMPR[MPRViewType::CORONAL]->translateCrossPt(UILineType::HORIZONTAL, fittedDist);
		}
		else
		{
			m_pViewMPR[eType]->translateCrossPt(UILineType::VERTICAL, fittedDist);
			m_pViewMPR[MPRViewType::AXIAL]->translateCrossPt(UILineType::HORIZONTAL, inverse_sagittal ? -fittedDist : fittedDist);
		}
		break;
	case MPRViewType::CORONAL:
		if (eLineType == MPRViewType::AXIAL)
		{
			m_pViewMPR[eType]->translateCrossPt(UILineType::HORIZONTAL, fittedDist);
			m_pViewMPR[MPRViewType::SAGITTAL]->translateCrossPt(UILineType::HORIZONTAL, fittedDist);
		}
		else
		{
			m_pViewMPR[eType]->translateCrossPt(UILineType::VERTICAL, fittedDist);
			m_pViewMPR[MPRViewType::AXIAL]->translateCrossPt(UILineType::VERTICAL, fittedDist);
		}
		break;
	}

	if (m_pView3DMPR->isDrawMPROverlay())
	{
		m_pView3DMPR->setMPROverlay(
			m_pViewMPR[eLineType]->getPlaneEquation(),
			m_pViewMPR[eLineType]->getPlaneRightVector()
		);
	}
}

void CW3MPRViewMgr::slotTranslateCross(MPRViewType eType, QVector2D& vD)
{
	bool inverse_sagittal = GlobalPreferences::GetInstance()->preferences_.advanced.mpr.sagittal_direction == GlobalPreferences::Direction::Inverse;

	float fittedDisty = 0.0f;
	float fittedDistx = 0.0f;
	switch (eType)
	{
	case MPRViewType::AXIAL:
		fittedDisty = m_pViewMPR[MPRViewType::CORONAL]->translateDraw(vD.y());
		fittedDistx = m_pViewMPR[MPRViewType::SAGITTAL]->translateDraw(inverse_sagittal ? vD.x() : -vD.x());
		if (!inverse_sagittal)
		{
			fittedDistx *= -1.0f;
		}
		break;
	case MPRViewType::SAGITTAL:
		fittedDisty = -m_pViewMPR[MPRViewType::AXIAL]->translateDraw(-vD.y());
		fittedDistx = m_pViewMPR[MPRViewType::CORONAL]->translateDraw(inverse_sagittal ? -vD.x() : vD.x());
		if (inverse_sagittal)
		{
			fittedDistx *= -1.0f;
		}
		break;
	case MPRViewType::CORONAL:
		fittedDisty = -m_pViewMPR[MPRViewType::AXIAL]->translateDraw(-vD.y());
		fittedDistx = m_pViewMPR[MPRViewType::SAGITTAL]->translateDraw(inverse_sagittal ? vD.x() : -vD.x());
		if (!inverse_sagittal)
		{
			fittedDistx *= -1.0f;
		}
		break;
	}

	m_pViewMPR[eType]->translateCrossPt(fittedDistx, fittedDisty);

	switch (eType)
	{
	case MPRViewType::AXIAL:
		m_pViewMPR[MPRViewType::SAGITTAL]->translateCrossPt(UILineType::VERTICAL, inverse_sagittal ? -fittedDisty : fittedDisty);
		m_pViewMPR[MPRViewType::CORONAL]->translateCrossPt(UILineType::VERTICAL, fittedDistx);
		break;
	case MPRViewType::SAGITTAL:
		m_pViewMPR[MPRViewType::CORONAL]->translateCrossPt(UILineType::HORIZONTAL, fittedDisty);
		m_pViewMPR[MPRViewType::AXIAL]->translateCrossPt(UILineType::HORIZONTAL, inverse_sagittal ? -fittedDistx : fittedDistx);
		break;
	case MPRViewType::CORONAL:
		m_pViewMPR[MPRViewType::SAGITTAL]->translateCrossPt(UILineType::HORIZONTAL, fittedDisty);
		m_pViewMPR[MPRViewType::AXIAL]->translateCrossPt(UILineType::VERTICAL, fittedDistx);
		break;
	}

	if (m_pView3DMPR->isDrawMPROverlay())
	{
		m_pView3DMPR->setMPROverlay(
			m_pViewMPR[eType]->getPlaneEquation(),
			m_pViewMPR[eType]->getPlaneRightVector()
		);
	}
}

void CW3MPRViewMgr::slotThickChange(MPRViewType target_view,
	MPRViewType ui_sync_view,
	float thickness_in_vol)
{
	m_pViewMPR[target_view]->thickDraw(thickness_in_vol);
	m_pViewMPR[ui_sync_view]->thickLineChange(target_view, thickness_in_vol);
	float real_thickness =
		thickness_in_vol * m_pgVRengine->getVol(0)->pixelSpacing();
	emit sigThicknessChanged(target_view, real_thickness);
}

void CW3MPRViewMgr::slotReorientReset()
{
	task_tool_.lock()->ResetOrientationDegreesUI();

	for (int id = 0; id < ReorientViewID::REORI_VIEW_END; ++id)
	{
		view_orientation_[id]->SetOrientationAngle(0);
	}
#if 1
	for (int id = 0; id < ReorientViewID::REORI_VIEW_END; ++id)
	{
		view_orientation_[id]->SetRotateMatrix(glm::mat4(1.0f));
	}
#endif
	for (int id = 0; id < ReorientViewID::REORI_VIEW_END; ++id)
	{
		view_orientation_[id]->SetRenderQuality();
	}

	glm::mat4 model(1.0f);
	m_pgVRengine->applyReorientation(model);
	m_pgMPRengine->applyReorientation(model);
}

void CW3MPRViewMgr::slotGridOnOffOrientation(bool on)
{
	for (int id = 0; id < ReorientViewID::REORI_VIEW_END; ++id)
	{
		view_orientation_[id]->SetGridOnOff(on);
	}
}

void CW3MPRViewMgr::slotMPRVisible(const VisibleID& visible_id, int state)
{
	m_pView3DMPR->VisibleObject(visible_id, state);

	if (VisibleID::NERVE == visible_id)
	{
		is_draw_nerve_ = state;
	}
	else if (VisibleID::IMPLANT == visible_id)
	{
		is_draw_implant_ = state;
	}

#ifndef WILL3D_LIGHT
	if (visible_id != VisibleID::AIRWAY)
#endif
	{
		for (int i = 0; i < MPRViewType::MPR_END; ++i)
		{
			m_pViewMPR[i]->VisibleObject(visible_id, state);
		}
	}
}

void CW3MPRViewMgr::slotMPRChangeFaceTransparency(int opacity)
{
	m_pView3DMPR->ChangeFaceTransparency(opacity);
}

void CW3MPRViewMgr::slotMPRClipEnable(int state)
{
	m_pView3DMPR->ClipEnable(state);
	slotSetMPRClipStatus();
}

void CW3MPRViewMgr::slotMPRClipRangeMove(int lower, int upper)
{
	m_pView3DMPR->ClipRangeMove(lower, upper);
}

void CW3MPRViewMgr::slotMPRClipRangeSet()
{
	m_pView3DMPR->ClipRangeSet();
}

void CW3MPRViewMgr::slotMPRClipPlaneChanged(const MPRClipID& clip_plane)
{
	m_pView3DMPR->ClipPlaneChanged(clip_plane);

	slotSelectedMPR(MPRViewType::AXIAL);
}

void CW3MPRViewMgr::slotFlipClipping(int state)
{
	m_pView3DMPR->SetFlipClipping(state);
}

void CW3MPRViewMgr::slotReoriUpdateVR(glm::mat4* mat)
{
	m_pView3DMPR->slotReoriupdate(mat);
}

void CW3MPRViewMgr::slotReoriUpdateMPR(glm::mat4* mat)
{
	for (int i = 0; i < MPRViewType::MPR_END; ++i)
		m_pViewMPR[i]->slotReoriupdate(mat);
}

void CW3MPRViewMgr::slotSecondUpdate(glm::mat4* mat)
{
	for (int i = 0; i < MPRViewType::MPR_END; ++i)
		m_pViewMPR[i]->SecondUpdate(mat);
}

std::vector<QWidget*> CW3MPRViewMgr::GetViewLightbox() const
{
	std::vector<QWidget*> widgets;
	widgets.reserve(view_lightbox_.size());
	for (const auto& view : view_lightbox_)
		widgets.push_back((QWidget*)(view));

	return widgets;
}

QWidget* CW3MPRViewMgr::GetViewLightboxMaximize(const int& lightbox_id) const
{
	return view_lightbox_[lightbox_id];
}

void CW3MPRViewMgr::secondDisabled(bool state, float* param)
{
	if (param)
	{
		for (int i = 0; i < kRegistrationParamCnt; ++i)
			m_RegiPointsSol[i] = param[i];
	}

	if (!state) m_pgMPRengine->SecondRegistered(m_RegiPointsSol);
}

void CW3MPRViewMgr::setTranslateMatSecondVolume(glm::mat4* mat)
{
	m_pView3DMPR->setTranslateMatSecondVolume(mat);
}

void CW3MPRViewMgr::setRotateMatSecondVolume(glm::mat4* mat)
{
	m_pView3DMPR->setRotateMatSecondVolume(mat);
}

void CW3MPRViewMgr::SetAirway(std::vector<tri_STL>& tries)
{
	m_pView3DMPR->SetAirway(tries);
}

void CW3MPRViewMgr::pointModel(glm::mat4* mat)
{
	m_pView3DMPR->slotTransformedPhotoPoints(mat);
}

void CW3MPRViewMgr::resetZoomView() { m_pViewZoom3D->reset(); }

void CW3MPRViewMgr::setMaximizeVRView(bool bMaximize)
{
	m_pView3DMPR->setMaximize(bMaximize);
}

void CW3MPRViewMgr::setVisibleMPRView(MPRViewType eType, bool bVisible)
{
  m_pViewMPR[eType]->setVisible(bVisible);
}

void CW3MPRViewMgr::InitMPRItems(MPRViewType eType)
{
  m_pViewMPR[eType]->initItems();
}

void CW3MPRViewMgr::setVisibleLightboxViews(bool visible)
{
	for (int id = 0; id < lightbox_count_; ++id)
	{
		view_lightbox_[id]->setVisible(visible);
	}
}

void CW3MPRViewMgr::setVisibleZoomView(bool bVisible)
{
	m_pViewZoom3D->setVisible(bVisible);
}

void CW3MPRViewMgr::setMIP(bool bMIP) { m_pView3DMPR->setMIP(bMIP); }

void CW3MPRViewMgr::setOnlyTRDMode() { m_pView3DMPR->setOnlyTRDMode(); }

void CW3MPRViewMgr::updateImageOnViewPlane(MPRViewType eType)
{
	m_pViewMPR[eType]->updateImageOnViewPlane();
}

void CW3MPRViewMgr::ChangeThicknessDirect(MPRViewType view_type,
	float real_thickness)
{
	float pixel_spacing = m_pgVRengine->getVol(0)->pixelSpacing();
	float slice_spacing = m_pgVRengine->getVol(0)->sliceSpacing();
	glm::vec3 up_vector =
		glm::normalize(glm::vec3(m_pViewMPR[view_type]->getPlaneEquation()));
	glm::vec3 spacing(pixel_spacing, pixel_spacing, slice_spacing);
	float plane_up_spacing = glm::length(up_vector * spacing);
	float ui_thickness = real_thickness / plane_up_spacing;

	for (int i = 0; i < MPRViewType::MPR_END; ++i)
	{
		if (static_cast<MPRViewType>(i) == view_type)
		{
			m_pViewMPR[i]->thickDraw(ui_thickness);
		}
		else
		{
			m_pViewMPR[i]->thickLineChange(view_type, ui_thickness);
		}
	}
}

void CW3MPRViewMgr::ChangeIntervalDirect(MPRViewType view_type,
	float real_interval)
{
	float pixel_spacing = m_pgVRengine->getVol(0)->pixelSpacing();
	float slice_spacing = m_pgVRengine->getVol(0)->sliceSpacing();
	glm::vec3 up_vector =
		glm::normalize(glm::vec3(m_pViewMPR[view_type]->getPlaneEquation()));

	glm::vec3 spacing(pixel_spacing, pixel_spacing, slice_spacing);
	float plane_up_spacing = glm::length(up_vector * spacing);
	float interval = real_interval / plane_up_spacing;

	m_pViewMPR[view_type]->SetIntervalOfSlider(interval);
}

void CW3MPRViewMgr::slotUpdateMPRPhoto(void)
{
	for (int i = 0; i < MPRViewType::MPR_END; ++i)
	{
		m_pViewMPR[i]->updateMPRPhoto();
		m_pViewMPR[i]->InitFacePhoto3D();
	}
	m_pView3DMPR->InitFacePhoto3D();
}

void CW3MPRViewMgr::TaskZoom3D(bool b)
{
	for (int i = 0; i < MPRViewType::MPR_END; ++i) m_pViewMPR[i]->zoom3D(b);

	m_pView3DMPR->zoom3D(b);
	m_pViewZoom3D->zoom3D(b);
}

void CW3MPRViewMgr::CreateLightboxViews(const LightboxViewType& view_type)
{
	view_lightbox_.resize(common::kMaxLightbox);
	for (int id = 0; id < common::kMaxLightbox; ++id)
	{
		view_lightbox_[id] = new ViewLightbox(view_type, id);
		view_lightbox_[id]->setVisible(false);
		view_lightbox_[id]->SetVisibleNerve(is_draw_nerve_);
		view_lightbox_[id]->SetVisibleImplant(is_draw_implant_);
	}

	ConnectLightbox();
}

void CW3MPRViewMgr::DeleteLightboxViews()
{
	for (int id = 0; id < view_lightbox_.size(); ++id)
	{
		SAFE_DELETE_OBJECT(view_lightbox_[id]);
	}
	view_lightbox_.clear();
}

void CW3MPRViewMgr::GetLightboxPlaneParams(
	const MPRViewType& view_type,
	lightbox_resource::PlaneParams& plane_params)
{
	m_pViewMPR[view_type]->GetViewPlaneParams(plane_params);
}

glm::vec3 CW3MPRViewMgr::GetLightboxPlaneCenter(const MPRViewType& view_type)
{
	return m_pViewMPR[view_type]->GetViewPlaneCenter();
}

void CW3MPRViewMgr::UpdateLightboxViews()
{
	const auto& res_lightbox =
		ResourceContainer::GetInstance()->GetLightboxResource();
	int maximized_lightbox_id = -1;
	if (res_lightbox.IsMaximzeMode(maximized_lightbox_id))
	{
		view_lightbox_[maximized_lightbox_id]->UpdateLightbox();
	}
	else
	{
		for (int id = 0; id < lightbox_count_; ++id)
		{
			view_lightbox_[id]->UpdateLightbox();
		}
	}
}

void CW3MPRViewMgr::InitLightboxSlider(const int& row, const int& col,
	const std::vector<int>& slider_positions,
	const int& available_depth)
{
	lightbox_count_ = slider_positions.size();
	for (int id = 0; id < lightbox_count_; ++id)
	{
		view_lightbox_[id]->SetEnabledSharpenUI(false);
		view_lightbox_[id]->blockSignals(true);
		view_lightbox_[id]->InitSlider(0, available_depth - 1,
			slider_positions[id]);
		view_lightbox_[id]->blockSignals(false);
	}

	view_lightbox_[col - 1]->SetEnabledSharpenUI(true);
}

void CW3MPRViewMgr::SyncLightboxSliderPositions(
	const std::vector<int>& slider_positions)
{
	for (int id = 0; id < lightbox_count_; ++id)
	{
		auto& view = view_lightbox_[id];
		view->blockSignals(true);
		view->SetSliderValue(slider_positions[id]);
		view->blockSignals(false);
	}
}

void CW3MPRViewMgr::SyncLightboxMeasureResource()
{
	for (int id = 0; id < lightbox_count_; ++id)
	{
		view_lightbox_[id]->SyncMeasureResourceSiblings(false);
	}
}

void CW3MPRViewMgr::DeleteMeasureUI(const common::ViewTypeID& view_type,
	const unsigned int& measure_id)
{
	switch (view_type)
	{
	case common::ViewTypeID::MPR_AXIAL:
		m_pViewMPR[MPRViewType::AXIAL]->DeleteMeasureUI(measure_id);
		break;
	case common::ViewTypeID::MPR_SAGITTAL:
		m_pViewMPR[MPRViewType::SAGITTAL]->DeleteMeasureUI(measure_id);
		break;
	case common::ViewTypeID::MPR_CORONAL:
		m_pViewMPR[MPRViewType::CORONAL]->DeleteMeasureUI(measure_id);
		break;
	case common::ViewTypeID::MPR_3D:
		m_pView3DMPR->DeleteMeasureUI(measure_id);
		break;
	case common::ViewTypeID::LIGHTBOX:
		for (int id = 0; id < lightbox_count_; ++id)
		{
			view_lightbox_[id]->SyncDeleteMeasureUI(measure_id);
		}
		break;
	}
}

void CW3MPRViewMgr::MoveViewsToSelectedMeasure(
	const common::ViewTypeID& view_type, const unsigned int& measure_id)
{
	// Get Visibility params from view
	common::measure::VisibilityParams visibility_params;
	GetViewsVisibilityParams(view_type, measure_id, &visibility_params);

	// Move view
	switch (view_type)
	{
	case common::ViewTypeID::MPR_AXIAL:
		MoveMPRViewToSelectedMeasure(MPRViewType::AXIAL, measure_id,
			visibility_params);
		break;
	case common::ViewTypeID::MPR_SAGITTAL:
		MoveMPRViewToSelectedMeasure(MPRViewType::SAGITTAL, measure_id,
			visibility_params);
		break;
	case common::ViewTypeID::MPR_CORONAL:
		MoveMPRViewToSelectedMeasure(MPRViewType::CORONAL, measure_id,
			visibility_params);
		break;
		// case common::ViewTypeID::MPR_3D: // 占쏙옙占쏙옙 3D Measure 占쏙옙占쏙옙
	case common::ViewTypeID::LIGHTBOX:
		MoveLightboxViewToSelectedMeasure(visibility_params);
		break;
	}
}

void CW3MPRViewMgr::ConnectLightbox()
{
	for (int id = 0; id < view_lightbox_.size(); ++id)
	{
		const auto& view = view_lightbox_[id];

		connect(view, &ViewLightbox::sigTranslate, this,
			&CW3MPRViewMgr::slotLightboxTranslate);
		connect(view, &ViewLightbox::sigDisplayDICOMInfo, this,
			&CW3MPRViewMgr::sigGetLightboxDICOMInfo);
		connect(view, &ViewLightbox::sigLightboxWindowingDone, this,
			&CW3MPRViewMgr::slotLightboxWindowing);
		connect(view, &ViewLightbox::sigLightboxZoomDone, this,
			&CW3MPRViewMgr::slotLightboxZoom);
		connect(view, &ViewLightbox::sigLightboxPanDone, this,
			&CW3MPRViewMgr::slotLightboxPan);
		connect(view, &ViewLightbox::sigSetSharpen, this,
			&CW3MPRViewMgr::slotLightboxViewSharpen);
		connect(view, &ViewLightbox::sigGetProfileData, this,
			&CW3MPRViewMgr::sigGetLightboxProfileData);
		connect(view, &ViewLightbox::sigGetROIData, this,
			&CW3MPRViewMgr::sigGetLightboxROIData);
		connect(view, &ViewLightbox::sigMeasureCreated, this,
			&CW3MPRViewMgr::slotLightboxMeasureCreated);
		connect(view, &ViewLightbox::sigMeasureDeleted, this,
			&CW3MPRViewMgr::slotLightboxMeasureDeleted);
		connect(view, &ViewLightbox::sigMeasureModified, this,
			&CW3MPRViewMgr::slotLightboxMeasureModified);

		connect(view, &ViewLightbox::sigMaximize, this,
			&CW3MPRViewMgr::slotLightboxMaximize, Qt::QueuedConnection);

		connect(view, &ViewLightbox::sigRequestSyncViewStatus, this,
			&CW3MPRViewMgr::slotSetLightboxViewStatus);

		connect(view, &ViewLightbox::sigCreateDCMFiles_ushort, this, &CW3MPRViewMgr::sigCreateDCMFiles_ushort);
		connect(view, &ViewLightbox::sigCreateDCMFiles_uchar, this, &CW3MPRViewMgr::sigCreateDCMFiles_uchar);
	}
}

void CW3MPRViewMgr::GetViewsVisibilityParams(
	const common::ViewTypeID& view_type, const unsigned int& measure_id,
	common::measure::VisibilityParams* visibility_params)
{
	switch (view_type)
	{
	case common::ViewTypeID::MPR_AXIAL:
		m_pViewMPR[MPRViewType::AXIAL]->GetMeasureParams(view_type, measure_id,
			visibility_params);
		break;
	case common::ViewTypeID::MPR_SAGITTAL:
		m_pViewMPR[MPRViewType::SAGITTAL]->GetMeasureParams(view_type, measure_id,
			visibility_params);
		break;
	case common::ViewTypeID::MPR_CORONAL:
		m_pViewMPR[MPRViewType::CORONAL]->GetMeasureParams(view_type, measure_id,
			visibility_params);
		break;
		// case common::ViewTypeID::MPR_3D: // 占쏙옙占쏙옙 3D Measure 占쏙옙占쏙옙
	case common::ViewTypeID::LIGHTBOX:
		view_lightbox_[0]->GetMeasureParams(view_type, measure_id,
			visibility_params);
		break;
	}
}

void CW3MPRViewMgr::MoveMPRViewToSelectedMeasure(
	const MPRViewType& view_type, const unsigned int& measure_id,
	const common::measure::VisibilityParams& visibility_param)
{
	auto transform_measure_iter = mpr_transform_measure_.find(measure_id);
	if (transform_measure_iter == mpr_transform_measure_.end())
	{
		common::Logger::instance()->PrintAndAssert(
			common::LogType::ERR,
			"CW3MPRViewMgr::MoveMPRViewToSelectedMeasure : measure does not "
			"exists. ");
	}

	SetCommonToolOnce(common::CommonToolTypeOnce::V_RESET, true);

	// translate planes
	MPRTansfromStatusForMeasure transform_status = transform_measure_iter->second;
	glm::vec3 center = *m_pgMPRengine->getMPRrotCenterInVol(0);
	glm::vec3 center_displacement = transform_status.rotate_center - center;
	glm::vec3 fit_dist;
	switch (view_type)
	{
	case MPRViewType::AXIAL:
		fit_dist.x = -m_pViewMPR[MPRViewType::SAGITTAL]->translateDraw(
			-center_displacement.x);
		fit_dist.y = m_pViewMPR[MPRViewType::CORONAL]->translateDraw(
			center_displacement.y);
		fit_dist.z = -m_pViewMPR[MPRViewType::AXIAL]->translateDraw(
			-center_displacement.z);
		m_pViewMPR[MPRViewType::AXIAL]->translateCrossPt(fit_dist.x, fit_dist.y);
		m_pViewMPR[MPRViewType::SAGITTAL]->translateCrossPt(UILineType::VERTICAL,
			fit_dist.y);
		m_pViewMPR[MPRViewType::CORONAL]->translateCrossPt(UILineType::VERTICAL,
			fit_dist.x);
		break;
	case MPRViewType::SAGITTAL:
		fit_dist.x = -m_pViewMPR[MPRViewType::SAGITTAL]->translateDraw(
			-center_displacement.x);
		fit_dist.y = m_pViewMPR[MPRViewType::CORONAL]->translateDraw(
			center_displacement.y);
		fit_dist.z = -m_pViewMPR[MPRViewType::AXIAL]->translateDraw(
			-center_displacement.z);
		m_pViewMPR[MPRViewType::SAGITTAL]->translateCrossPt(fit_dist.y,
			fit_dist.z);
		m_pViewMPR[MPRViewType::CORONAL]->translateCrossPt(UILineType::HORIZONTAL,
			fit_dist.z);
		m_pViewMPR[MPRViewType::AXIAL]->translateCrossPt(UILineType::HORIZONTAL,
			fit_dist.y);
		break;
	case MPRViewType::CORONAL:
		fit_dist.x = -m_pViewMPR[MPRViewType::SAGITTAL]->translateDraw(
			-center_displacement.x);
		fit_dist.y = m_pViewMPR[MPRViewType::CORONAL]->translateDraw(
			center_displacement.y);
		fit_dist.z = -m_pViewMPR[MPRViewType::AXIAL]->translateDraw(
			-center_displacement.z);
		m_pViewMPR[MPRViewType::CORONAL]->translateCrossPt(fit_dist.x,
			fit_dist.z);
		m_pViewMPR[MPRViewType::SAGITTAL]->translateCrossPt(
			UILineType::HORIZONTAL, fit_dist.z);
		m_pViewMPR[MPRViewType::AXIAL]->translateCrossPt(UILineType::VERTICAL,
			fit_dist.x);
		break;
	}

	// rotate planes
	glm::mat4 dummy;

	auto func_radian = [](const glm::vec3& v1, const glm::vec3& v2) -> float
	{
		glm::vec3 cross = glm::cross(v1, v2);
		float sign = (cross.x + cross.y + cross.z) > 0.0f ? 1.0f : -1.0f;
		return sign * glm::acos(glm::dot(glm::normalize(v1), glm::normalize(v2)));
	};
	m_pViewMPR[MPRViewType::AXIAL]->rotateItems(
		transform_status.axial_angle_degree / 180.0f * M_PI);
	m_pViewMPR[MPRViewType::AXIAL]->rotate3DDraw(&dummy, &transform_status.axial);

	m_pViewMPR[MPRViewType::SAGITTAL]->rotateItems(
		transform_status.sagittal_angle_degree / 180.0f * M_PI);
	m_pViewMPR[MPRViewType::SAGITTAL]->rotate3DDraw(&dummy,
		&transform_status.sagittal);

	m_pViewMPR[MPRViewType::CORONAL]->rotateItems(
		transform_status.coronal_angle_degree / 180.0f * M_PI);
	m_pViewMPR[MPRViewType::CORONAL]->rotate3DDraw(&dummy,
		&transform_status.coronal);

	if (m_pView3DMPR->isDrawMPROverlay())
	{
		m_pView3DMPR->setMPROverlay(
			m_pViewMPR[view_type]->getPlaneEquation(),
			m_pViewMPR[view_type]->getPlaneRightVector()
		);

		m_pView3DMPR->UpdateVR();
	}
}

void CW3MPRViewMgr::MoveLightboxViewToSelectedMeasure(
	const common::measure::VisibilityParams& visibility_param)
{
	glm::vec3 curr_center = view_lightbox_[0]->GetCenterPosition();
	glm::vec3 curr_up_vector = view_lightbox_[0]->GetUpVector();

	glm::vec3 curr_center_to_moved_center = visibility_param.center - curr_center;
	float displacement = glm::dot(curr_center_to_moved_center, curr_up_vector);

	emit sigLightboxTranslate(displacement);
	UpdateLightboxViews();
}

void CW3MPRViewMgr::slotSetZoom3DVR(const MPRViewType eType, const glm::vec3 v,
	const float f)
{
	m_pView3DMPR->SetZoom3DVR(eType, v, f);
	m_pViewZoom3D->SetZoom3DVR(eType, v, f);
}

void CW3MPRViewMgr::slotTFUpdateCompleted()
{
	m_pView3DMPR->slotTFupdateCompleted();
	m_pViewZoom3D->slotTFupdateCompleted();
}

void CW3MPRViewMgr::slotWheel(MPRViewType eType, float fDelta, const bool update_3d_view)
{
	QObject* pSender = QObject::sender();
	for (int idx = 0; idx < MPRViewType::MPR_END; ++idx)
	{
		if (pSender != m_pViewMPR[idx].get())
			m_pViewMPR[idx]->WheelView(eType, fDelta);
	}

	slotSelectedMPR(eType, update_3d_view);
}

void CW3MPRViewMgr::TaskSTLExport(bool clip_on)
{
	if (clip_on)
	{
		m_pView3DMPR->ClipEnable(false);
	}

	float min_tf = ResourceContainer::GetInstance()->GetTfResource().min_value();
	GenerateFaceDlg threshold_dialog(this);

	/*QPoint view_pos = m_pViewMPR[0]->pos();
	QPoint starting_point = m_pViewMPR[0]->mapToGlobal(view_pos);
	QRect geometry = m_pViewMPR[0]->geometry();

	QPoint center_pos = starting_point + QPoint(geometry.width(), geometry.height());

	QSize dlg_size = threshold_dialog.sizeHint();
	QRect dialog_geometry = threshold_dialog.geometry();

	QPoint move_pos = center_pos - QPoint(dlg_size.width(), dlg_size.height() + 20) * 0.5;

	threshold_dialog.move(move_pos);*/

	threshold_dialog.SetThreshold(min_tf);
	connect(&threshold_dialog, SIGNAL(sigChangeValue(double)), this, SLOT(slotChangeSTLExportThreshold(double)));
	if (threshold_dialog.exec())
	{
		STLExportDialog stl_export_dialog(this);
		if (stl_export_dialog.exec())
		{
			//stl_export_dialog.move(move_pos);

			CW3ProgressDialog* progress = CW3ProgressDialog::getInstance(CW3ProgressDialog::PERCENT);

			QFutureWatcher<bool> watcher;
			connect(&watcher, SIGNAL(finished()), progress, SLOT(hide()));

			auto future = QtConcurrent::run(this, &CW3MPRViewMgr::STLExport, stl_export_dialog.GetSavePath(), threshold_dialog.threshold());
			watcher.setFuture(future);
			progress->exec();
			watcher.waitForFinished();

			if (future.result())
			{
				CW3MessageBox msgBox("Will3D", lang::LanguagePack::msg_62(), CW3MessageBox::Information);
				msgBox.exec();
			}
			else
			{
				CW3MessageBox msgBox("Will3D", lang::LanguagePack::msg_63(), CW3MessageBox::Critical);
				msgBox.exec();
			}
		}
	}

	if (clip_on)
	{
		m_pView3DMPR->ClipEnable(true);
	}
}

void CW3MPRViewMgr::TaskDrawArch()
{
	ArchTypeID arch_type = task_tool_.lock()->GetArchType();
	m_pViewMPR[MPRViewType::AXIAL]->DrawArch(arch_type);
}

void CW3MPRViewMgr::slotChangeSTLExportThreshold(double threshold)
{
	EventHandler::GetInstance()->GetCommonEventHandle().EmitSigMoveTFpolygon(
		threshold);
}

void CW3MPRViewMgr::slotSetMPRClipStatus()
{
	ClipStatus clip_status = task_tool_.lock().get()->GetClipStatus();
	m_pView3DMPR->initClipValues(clip_status.plane, clip_status.enable,
		clip_status.flip, clip_status.lower,
		clip_status.upper);
}

void CW3MPRViewMgr::slotMPRMeasureCreated(const unsigned int& measure_id)
{
	MPRTansfromStatusForMeasure status;
	status.axial = m_pViewMPR[MPRViewType::AXIAL]->GetPlaneRotateMatrix();
	status.axial_angle_degree = m_pViewMPR[MPRViewType::AXIAL]->GetAngleDegree();
	status.sagittal = m_pViewMPR[MPRViewType::SAGITTAL]->GetPlaneRotateMatrix();
	status.sagittal_angle_degree =
		m_pViewMPR[MPRViewType::SAGITTAL]->GetAngleDegree();
	status.coronal = m_pViewMPR[MPRViewType::CORONAL]->GetPlaneRotateMatrix();
	status.coronal_angle_degree =
		m_pViewMPR[MPRViewType::CORONAL]->GetAngleDegree();
	status.rotate_center = *m_pgMPRengine->getMPRrotCenterInVol(0);
	mpr_transform_measure_[measure_id] = status;
}

void CW3MPRViewMgr::slotMPRMeasureDeleted(const unsigned int& measure_id)
{
	if (mpr_transform_measure_.find(measure_id) != mpr_transform_measure_.end())
		mpr_transform_measure_.erase(measure_id);
}

void CW3MPRViewMgr::slotLightboxTranslate(const int& lightbox_id,
	const int& slider_value)
{
	emit sigLightboxTranslate(lightbox_id, slider_value);

	UpdateLightboxViews();
}

void CW3MPRViewMgr::slotLightboxWindowing()
{
	for (auto& view : view_lightbox_)
	{
		view->RenderAndUpdate();
	}
}

void CW3MPRViewMgr::slotLightboxZoom(float scene_scale)
{
	QObject* sender = QObject::sender();
	for (int id = 0; id < view_lightbox_.size(); ++id)
	{
		if (view_lightbox_[id] == sender) continue;

		view_lightbox_[id]->SetZoomScale(scene_scale);
	}
}

void CW3MPRViewMgr::slotLightboxPan(const QPointF& trans)
{
	QObject* sender = QObject::sender();
	for (int id = 0; id < view_lightbox_.size(); ++id)
	{
		if (view_lightbox_[id] == sender) continue;

		view_lightbox_[id]->SetPanTranslate(trans);
	}
}

void CW3MPRViewMgr::slotLightboxViewSharpen(const SharpenLevel& sharpen_level)
{
	QObject* sender = QObject::sender();
	for (int id = 0; id < view_lightbox_.size(); ++id)
	{
		if (view_lightbox_[id] == sender)
		{
			continue;
		}

		view_lightbox_[id]->SetSharpenLevel(sharpen_level);
	}
}

void CW3MPRViewMgr::slotLightboxMaximize(const int& lightbox_id)
{
	emit sigLightboxMaximize(lightbox_id);
}

void CW3MPRViewMgr::slotSetLightboxViewStatus(float* scene_scale,
	QPointF* gl_translate)
{
	*scene_scale = view_lightbox_[0]->GetSceneScale();
	*gl_translate = view_lightbox_[0]->GetGLTranslate();
}

void CW3MPRViewMgr::slotLightboxMeasureCreated(const int& lightbox_id,
	const unsigned int& measure_id)
{
	for (int i = 0; i < lightbox_count_; i++)
	{
		if (i == lightbox_id) continue;

		view_lightbox_[i]->SyncCreateMeasureUI(measure_id);
	}
}

void CW3MPRViewMgr::slotLightboxMeasureDeleted(const int& lightbox_id,
	const unsigned int& measure_id)
{
	for (int i = 0; i < lightbox_count_; i++)
	{
		if (i == lightbox_id) continue;

		view_lightbox_[i]->SyncDeleteMeasureUI(measure_id);
	}
}

void CW3MPRViewMgr::slotLightboxMeasureModified(
	const int& lightbox_id, const unsigned int& measure_id)
{
	for (int i = 0; i < lightbox_count_; i++)
	{
		if (i == lightbox_id) continue;

		view_lightbox_[i]->SyncModifyMeasureUI(measure_id);
	}
}

void CW3MPRViewMgr::slotSelectedMPR(MPRViewType eViewType, const bool update_3d_view)
{
	current_mpr_overlay_plane_ = eViewType;
	m_pView3DMPR->setMPROverlay(
		m_pViewMPR[eViewType]->getPlaneEquation(),
		m_pViewMPR[eViewType]->getPlaneRightVector()
	);

	if (m_pView3DMPR->isDrawMPROverlay() && update_3d_view)
	{
		m_pView3DMPR->UpdateVR();
	}
}

void CW3MPRViewMgr::TaskOblique(bool bToggled)
{
	m_pView3DMPR->SetOblique(bToggled);
}

void CW3MPRViewMgr::Task3DCut(bool on, VRCutTool cut_tool)
{
	m_pView3DMPR->Cut3D(on, cut_tool);
}

void CW3MPRViewMgr::slot3DCutReset()
{
	m_pView3DMPR->Reset3DCut();
}

void CW3MPRViewMgr::slot3DCutUndo()
{
	m_pView3DMPR->Undo3DCut();
}

void CW3MPRViewMgr::slot3DCutRedo()
{
	m_pView3DMPR->Redo3DCut();
}

bool CW3MPRViewMgr::STLExport(const QString& path, int threshold)
{
	GlobalPreferences* global_preferences = GlobalPreferences::GetInstance();

	bool need_normal = false;

	CW3Image3D* volume = m_pgVRengine->getVol(0);
	int width = volume->width();
	int height = volume->height();
	int depth = volume->depth();

	float max_size = 0.0f;
	float factor = 1.0f;
	switch (global_preferences->preferences_.stl_export.quality)
	{
	case GlobalPreferences::Quality3::High:
		max_size = 266.6f;
		factor = 1.0f;
		break;
	case GlobalPreferences::Quality3::Medium:
		max_size = 200.0f;
		factor = 2.0f;
		break;
	case GlobalPreferences::Quality3::Low:
		max_size = 160.0f;
		factor = 3.0f;
		break;
	}

	if (max_size == 0.0f) return false;

#if 0
	float dsf = std::max(float(std::max(width, height)) / max_size, 1.0f);
#else
	float dsf = factor;
#endif

	qDebug() << "STLExport cut & smooth start";
	clock_t start_time = clock();

	// cut
	int cur_vr_cut_history_step = m_pView3DMPR->cur_vr_cut_history_step();
	unsigned short** vr_cut_mask = nullptr;
	if (m_pView3DMPR->vr_cut_mask_vol())
		vr_cut_mask = m_pView3DMPR->vr_cut_mask_vol()->getData();
	unsigned short** remainder_volume = nullptr;

	remainder_volume = SAFE_ALLOC_VOLUME(unsigned short, depth, width* height);

	int count = 0;

	CW3ProgressDialog* progress = CW3ProgressDialog::getInstance(CW3ProgressDialog::PERCENT);
	progress->setRange(0, 100);
	progress->setValue(0);

	const int num_thread = omp_get_max_threads();
	qDebug() << "omp_get_max_threads = " << num_thread;
	omp_set_num_threads(1);  // TODO: check using multi thread
#pragma omp parallel for shared(count)
	for (int i = 0; i < depth; i++)
	{
		// smooth
		unsigned short* slice = volume->getData()[i];
		cv::Mat cv_smooth;
		if (global_preferences->preferences_.stl_export.smooth_on)
		{
			cv::Mat cv_slice = cv::Mat(height, width, CV_16U);
			memcpy(cv_slice.ptr<unsigned short>(0), slice,
				sizeof(unsigned short) * width * height);
			cv::GaussianBlur(cv_slice, cv_smooth, cv::Size(9, 9), 4.0f);
			slice = cv_smooth.ptr<unsigned short>(0);

			cv_slice.release();
		}
		// smooth

		for (int j = 0; j < width * height; j++)
		{
			if (vr_cut_mask)
			{
				if (vr_cut_mask[i][j] & 0x0001 << cur_vr_cut_history_step)
					remainder_volume[i][j] = 0.0f;
				else
					remainder_volume[i][j] = slice[j];
			}
			else
			{
				remainder_volume[i][j] = slice[j];
			}
		}

		cv_smooth.release();

#pragma omp atomic
		++count;
		progress->setValue(static_cast<float>(count) / depth * 100.0f);
	}
	// cut

	float elapsed_time = static_cast<float>(clock() - start_time);
	qDebug() << "STLExport cut & smooth done :" << elapsed_time << "ms";

	progress->setFormat(CW3ProgressDialog::WAITTING);

	std::vector<glm::vec3> points;
	std::vector<glm::vec3> normals;
	std::vector<std::vector<int>> triangles;
	MarchingCube::execute(points, triangles, threshold, remainder_volume, width,
		height, depth, dsf, need_normal ? &normals : nullptr);

	if (remainder_volume) SAFE_DELETE_VOLUME(remainder_volume, depth);

#if 0
	qDebug() << "CW3MPRViewMgr::STLExport() points :" << points.size();
	qDebug() << "CW3MPRViewMgr::STLExport() normals :" << normals.size();
	qDebug() << "CW3MPRViewMgr::STLExport() triangles :" << triangles.size();
#endif

	bool simplification = false;
	if (simplification)
	{
		CW3MeshSimplifier::execute_CGAL(points, triangles, 0.8f);
#if 0
		qDebug() << "CW3MPRViewMgr::STLExport() points :" << points.size();
		qDebug() << "CW3MPRViewMgr::STLExport() normals :" << normals.size();
		qDebug() << "CW3MPRViewMgr::STLExport() triangles :" << triangles.size();
#endif
	}

	QFile file(path);
	if (!file.open(QIODevice::WriteOnly))
	{
		return false;
	}

	try
	{
		qDebug() << "STLExport save start";
		clock_t start_time = clock();

		progress->setFormat(CW3ProgressDialog::PERCENT);
		progress->setRange(0, 100);
		progress->setValue(0);

		float width_factor = 0.5f * width * volume->pixelSpacing();
		float height_factor = 0.5f * height * volume->pixelSpacing();
		float depth_factor = 0.5f * depth * volume->sliceSpacing();

		quint32 triangles_size = triangles.size();
		quint16 control_bytes = 0;

		QDataStream data_stream(&file);

		char header[80];
		data_stream.writeRawData(header, sizeof(header));
		data_stream.writeRawData((char*)&triangles_size, sizeof(triangles_size));

		const std::streamsize float_size = sizeof(float);
		const std::streamsize control_bytes_size = sizeof(control_bytes);
		const float progress_percent_value = 100.0f / triangles_size;
		for (int i = 0; i < triangles_size; ++i)
		{
			float nx, ny, nz;

			const int& idx_1 = triangles.at(i).at(0);
			const int& idx_2 = triangles.at(i).at(1);
			const int& idx_3 = triangles.at(i).at(2);

			if (need_normal)
			{
				nx = -(normals[idx_1].x + normals[idx_2].x + normals[idx_3].x) / 3.f;
				ny = (normals[idx_1].y + normals[idx_2].y + normals[idx_3].y) / 3.f;
				nz = -(normals[idx_1].z + normals[idx_2].z + normals[idx_3].z) / 3.f;
			}
			else
			{
				nx = 0.f;
				ny = 0.f;
				nz = 0.f;
			}

			float x[] = { width_factor * (-points[idx_1].x + 1.f),
						 width_factor * (-points[idx_2].x + 1.f),
						 width_factor * (-points[idx_3].x + 1.f) };

			float y[] = { height_factor * (points[idx_1].y + 1.f),
						 height_factor * (points[idx_2].y + 1.f),
						 height_factor * (points[idx_3].y + 1.f) };

			float z[] = { depth_factor * (-points[idx_1].z + 1.f),
						 depth_factor * (-points[idx_2].z + 1.f),
						 depth_factor * (-points[idx_3].z + 1.f) };

			// nx, ny, nz: normal for face
			data_stream.writeRawData((char*)&nx, float_size);
			data_stream.writeRawData((char*)&ny, float_size);
			data_stream.writeRawData((char*)&nz, float_size);

			// x[0], y[0], z[0]: x, y, z coordinate for each triangle
			data_stream.writeRawData((char*)&x[0], float_size);
			data_stream.writeRawData((char*)&y[0], float_size);
			data_stream.writeRawData((char*)&z[0], float_size);

			// x[1], y[1], z[1]: x, y, z coordinate for each triangle
			data_stream.writeRawData((char*)&x[1], float_size);
			data_stream.writeRawData((char*)&y[1], float_size);
			data_stream.writeRawData((char*)&z[1], float_size);

			// x[2], y[2], z[2]: x, y, z coordinate for each triangle
			data_stream.writeRawData((char*)&x[2], float_size);
			data_stream.writeRawData((char*)&y[2], float_size);
			data_stream.writeRawData((char*)&z[2], float_size);

			data_stream.writeRawData((char*)&control_bytes, control_bytes_size);

			progress->setValue(static_cast<float>(i) * progress_percent_value);
		}
		progress->setFormat(CW3ProgressDialog::WAITTING);

		file.close();

		float elapsed_time = static_cast<float>(clock() - start_time);
		qDebug() << "STLExport save done :" << elapsed_time << "ms";

		return true;
	}
	catch (std::runtime_error e)
	{
		return false;
	}
}

void CW3MPRViewMgr::ApplyPreferences()
{
	for (int i = 0; i < MPRViewType::MPR_END; ++i)
		m_pViewMPR[i]->ApplyPreferences();
	m_pView3DMPR->ApplyPreferences();
	m_pViewZoom3D->ApplyPreferences();
}

void CW3MPRViewMgr::slotRotateOrientationView(const ReorientViewID& view_type, const glm::mat4& orien)
{
	for (int i = 0; i < ReorientViewID::REORI_VIEW_END; ++i)
	{
		if (i == view_type)
		{
			int degree_view = view_orientation_[view_type]->GetAngleDegree();
			task_tool_.lock()->SyncOrientationDegreeUIOnly(view_type, degree_view);
		}
		else
		{
			view_orientation_[i]->SetRotateMatrix(orien);
		}
	}

	glm::mat4 reorientation_matrix = orien;
	m_pgVRengine->applyReorientation(reorientation_matrix);
	m_pgMPRengine->applyReorientation(reorientation_matrix);
}

void CW3MPRViewMgr::slotRotateOrientationFromTaskTool(const ReorientViewID& view_type, int angle)
{
	view_orientation_[view_type]->SetOrientationAngle(angle);
}

void CW3MPRViewMgr::slotSetOrientationViewRenderQuality(const ReorientViewID& view_type)
{
	for (int id = 0; id < ReorientViewID::REORI_VIEW_END; ++id)
	{
		if (id == view_type)
		{
			continue;
		}
		view_orientation_[id]->SetRenderQuality();
	}
}

void CW3MPRViewMgr::slotSelectImplant(const int id)
{
	pano_engine_->SetImplantSelected(id, true);
}

void CW3MPRViewMgr::slotTranslateImplant(const int& implant_id, const glm::vec3& delta_trans)
{
#if 0
	emit sigTranslateImplant(implant_id, delta_trans);
#else

	pano_engine_->SetImplantPositionVolAndPano3D(implant_id, delta_trans);
	pano_engine_->SetImplantAxisPointVolAndPano3D(implant_id);
	if (pano_engine_->IsValidPanoramaImage()) // for import project
	{
		pano_engine_->SetImplantPositionInPanoPlane(implant_id);
	}
	pano_engine_->SetImplantAxisPointPanoPlane(implant_id);
	pano_engine_->CheckCollideImplant(m_pView3DMPR->GetProjectionViewMatrix());
#endif
}

void CW3MPRViewMgr::slotRotateImplant(const int& implant_id, const glm::vec3& rotate_axes, const float& delta_degree)
{
#if 0
	emit sigRotateImplant(implant_id, rotate_axes, delta_degree);
#else
	pano_engine_->RotateImplantIn3D(implant_id, rotate_axes, delta_degree);
	pano_engine_->SetImplantAxisPointVolAndPano3D(implant_id);
	if (pano_engine_->IsValidPanoramaImage()) // for import project
	{
		pano_engine_->SetImplantPositionInPanoPlane(implant_id);
	}
	pano_engine_->SetImplantAxisPointPanoPlane(implant_id);
	pano_engine_->CheckCollideImplant(m_pView3DMPR->GetProjectionViewMatrix());
#endif
}

void CW3MPRViewMgr::slotDoneAdjustImplant()
{
	QObject* sender = QObject::sender();
	for (int i = 0; i < MPRViewType::MPR_END; ++i)
	{
		if (sender != m_pViewMPR[i].get())
		{
			m_pViewMPR[i]->updateImageOnViewPlane();
		}
	}
	m_pView3DMPR->UpdateVR(true);
}

void CW3MPRViewMgr::slotAdjustImplantButtonToggled(bool checked)
{
	for (int i = 0; i < MPRViewType::MPR_END; ++i)
	{
		m_pViewMPR[i]->AdjustImplant(checked);
	}
}

void CW3MPRViewMgr::slotVolumeClicked(const glm::vec3 gl_pos)
{
#if 1
	glm::vec3 clicked_vol_pos(-gl_pos.x, gl_pos.y, gl_pos.z);
	clicked_vol_pos = (clicked_vol_pos + vec3(1.0f)) * 0.5f * *m_pgMPRengine->getVolRange(0);
#else
	glm::vec3 clicked_vol_pos(0.5f * *m_pgMPRengine->getVolRange(0));
#endif
	glm::vec3 center = *m_pgMPRengine->getMPRrotCenterInVol(0);

	glm::vec3 axial_plane_normal = m_pViewMPR[MPRViewType::AXIAL]->GetPlaneUpVector();
	glm::vec3 sagittal_plane_normal = m_pViewMPR[MPRViewType::SAGITTAL]->GetPlaneUpVector();
	glm::vec3 coronal_plane_normal = m_pViewMPR[MPRViewType::CORONAL]->GetPlaneUpVector();

	// move axial plane
	glm::vec3 projected_center = center - glm::dot(center, sagittal_plane_normal) * sagittal_plane_normal;
	projected_center = projected_center - glm::dot(projected_center, coronal_plane_normal) * coronal_plane_normal;

	glm::vec3 projected_clicked_vol_pos = clicked_vol_pos - glm::dot(clicked_vol_pos, sagittal_plane_normal) * sagittal_plane_normal;
	projected_clicked_vol_pos = projected_clicked_vol_pos - glm::dot(projected_clicked_vol_pos, coronal_plane_normal) * coronal_plane_normal;

	glm::vec3 projected_translated = projected_clicked_vol_pos - projected_center;

	int direction = (glm::dot(projected_translated, axial_plane_normal) < 0) ? -1 : 1;

	float axial_line_translated = m_pViewMPR[MPRViewType::AXIAL]->translateDraw(direction * glm::length(projected_translated), false);

	m_pViewMPR[MPRViewType::SAGITTAL]->translateCrossPt(UILineType::HORIZONTAL, -axial_line_translated);
	m_pViewMPR[MPRViewType::CORONAL]->translateCrossPt(UILineType::HORIZONTAL, -axial_line_translated);

	// move sagittal plane
	projected_center = center - glm::dot(center, axial_plane_normal) * axial_plane_normal;
	projected_center = projected_center - glm::dot(projected_center, coronal_plane_normal) * coronal_plane_normal;

	projected_clicked_vol_pos = clicked_vol_pos - glm::dot(clicked_vol_pos, axial_plane_normal) * axial_plane_normal;
	projected_clicked_vol_pos = projected_clicked_vol_pos - glm::dot(projected_clicked_vol_pos, coronal_plane_normal) * coronal_plane_normal;

	projected_translated = projected_clicked_vol_pos - projected_center;

	direction = (glm::dot(projected_translated, sagittal_plane_normal) < 0) ? -1 : 1;

	float sagittal_line_translated = m_pViewMPR[MPRViewType::SAGITTAL]->translateDraw(direction * glm::length(projected_translated), false);

	m_pViewMPR[MPRViewType::AXIAL]->translateCrossPt(UILineType::VERTICAL, -sagittal_line_translated);
	m_pViewMPR[MPRViewType::CORONAL]->translateCrossPt(UILineType::VERTICAL, -sagittal_line_translated);

	// move coronal plane
	projected_center = center - glm::dot(center, axial_plane_normal) * axial_plane_normal;
	projected_center = projected_center - glm::dot(projected_center, sagittal_plane_normal) * sagittal_plane_normal;

	projected_clicked_vol_pos = clicked_vol_pos - glm::dot(clicked_vol_pos, axial_plane_normal) * axial_plane_normal;
	projected_clicked_vol_pos = projected_clicked_vol_pos - glm::dot(projected_clicked_vol_pos, sagittal_plane_normal) * sagittal_plane_normal;

	projected_translated = projected_clicked_vol_pos - projected_center;

	direction = (glm::dot(projected_translated, coronal_plane_normal) < 0) ? -1 : 1;

	float coronal_line_translated = m_pViewMPR[MPRViewType::CORONAL]->translateDraw(direction * glm::length(projected_translated), false);

	m_pViewMPR[MPRViewType::AXIAL]->translateCrossPt(UILineType::HORIZONTAL, coronal_line_translated);
	m_pViewMPR[MPRViewType::SAGITTAL]->translateCrossPt(UILineType::VERTICAL, coronal_line_translated);

	slotSelectedMPR(current_mpr_overlay_plane_);
}

void CW3MPRViewMgr::slotUpdateMPROverlay()
{
	if (!m_pView3DMPR || !m_pView3DMPR->isDrawMPROverlay())
	{
		return;
	}

	m_pView3DMPR->UpdateVR();
}

void CW3MPRViewMgr::SetPanoEngine(const std::shared_ptr<PanoEngine>& pano_engine)
{
	pano_engine_ = pano_engine;
}

void CW3MPRViewMgr::EmitSendMPRPlaneInfo(const MPRViewType mpr_view_type)
{
	if (mpr_view_type == MPRViewType::MPR_END || mpr_view_type == MPRViewType::MPR_UNKNOWN)
	{
		return;
	}

	lightbox_resource::PlaneParams plane_params;
	m_pViewMPR[mpr_view_type]->GetViewPlaneParams(plane_params);

	emit sigSendMPRPlaneInfo(plane_params.right, plane_params.back, plane_params.available_depth);
}

const lightbox_resource::PlaneParams CW3MPRViewMgr::GetMPRPlaneParams(MPRViewType mpr_view_type)
{
	lightbox_resource::PlaneParams plane_params;
	m_pViewMPR[mpr_view_type]->GetViewPlaneParams(plane_params);

	return plane_params;
}

#ifdef WILL3D_EUROPE
void CW3MPRViewMgr::SetSyncControlButtonOut()
{
	for (int i = 0; i < MPRViewType::MPR_END; ++i)
	{
		auto view_mpr_target = m_pViewMPR[i].get();
		view_mpr_target->SetSyncControlButton(false);
	}

	m_pView3DMPR.get()->SetSyncControlButton(false);
	m_pViewZoom3D.get()->SetSyncControlButton(false);
}

MPRViewType CW3MPRViewMgr::GetMouseOverMPR()
{
	for (int i = 0; i < MPRViewType::MPR_END; ++i)
	{
		if (m_pViewMPR[i].get()->mouse_orver())
		{
			return static_cast<MPRViewType>(i);
		}
	}

	return MPRViewType::MPR_UNKNOWN;
}
#endif // WILL3D_EUROPE
