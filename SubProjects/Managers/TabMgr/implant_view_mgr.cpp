#include "implant_view_mgr.h"

#include <qapplication.h>
#include <qmath.h>
#include <QTextCodec>

#include "../../Engine/Common/Common/W3Logger.h"
#include "../../Engine/Common/Common/define_measure.h"
#include "../../Engine/Common/Common/define_pano.h"
#include <Engine/Common/Common/global_preferences.h>

#include "../../Engine/Resource/ResContainer/resource_container.h"
#include "../../Engine/Resource/Resource/cross_section_resource.h"
#include "../../Engine/Resource/Resource/implant_resource.h"
#include "../../Engine/Resource/Resource/nerve_resource.h"
#include "../../Engine/Resource/Resource/pano_resource.h"
#include "../../Engine/Resource/Resource/sagittal_resource.h"
#ifndef WILL3D_VIEWER
#include "../../Engine/Core/W3ProjectIO/project_io_implant.h"
#include "../../Engine/Core/W3ProjectIO/project_io_view.h"
#endif
#include <Engine/UIModule/UITools/implant_task_tool.h>

#include "../../Engine/Module/Panorama/pano_engine.h"

#include "../../Engine/UIModule/UIComponent/view_bone_density.h"
#include "../../Engine/UIModule/UIComponent/view_implant_3d.h"
#include "../../Engine/UIModule/UIComponent/view_implant_arch.h"
#include "../../Engine/UIModule/UIComponent/view_implant_cross_section.h"
#include "../../Engine/UIModule/UIComponent/view_implant_pano.h"
#include "../../Engine/UIModule/UIComponent/view_implant_sagittal.h"
#include "../../Engine/UIModule/UIPrimitive/text_edit.h"

ImplantViewMgr::ImplantViewMgr(QObject* parent) : BasePanoViewMgr(parent)
{
	InitializeViews();
	connections();
}

ImplantViewMgr::~ImplantViewMgr(void) {}

void ImplantViewMgr::connections()
{
	BasePanoViewMgr::connections();
	const ViewImplantArch* view_arch = view_arch_.get();
	connect(view_arch, &ViewImplantArch::sigRotateSagittal, this, &ImplantViewMgr::slotRotateSagittal);
	connect(view_arch, &ViewImplantArch::sigRequestPanoPosition, this, &ImplantViewMgr::slotGetPanoPosition);
	connect(view_arch, &ViewImplantArch::sigSelectImplant, this, &ImplantViewMgr::slotSelectImplantFromArch);
	connect(view_arch, &ViewImplantArch::sigTranslateImplant, this, &ImplantViewMgr::slotTranslateImplantFromArch);
	connect(view_arch, &ViewImplantArch::sigRotateImplant, this, &ImplantViewMgr::slotRotateImplantFromArch);
	connect(view_arch, &ViewImplantArch::sigUpdateImplantImages, this, &ImplantViewMgr::slotUpdateAllImplantImagesFromArch);

	const ViewImplantSagittal* view_sagittal = view_sagittal_.get();
	connect(view_sagittal, &ViewImplantSagittal::sigProcessedLightEvent, this, &ImplantViewMgr::slotProcessedLightEvent);
	connect(view_sagittal, &ViewImplantSagittal::sigSetAxialSlice, this, &ImplantViewMgr::slotSetAxialSliceFromSagittalView);
	connect(view_sagittal, &ViewImplantSagittal::sigImplantHovered, this, &ImplantViewMgr::slotImplantHoveredFromSagittal);
	connect(view_sagittal, &ViewImplantSagittal::sigGetImplantPlanePos, this, &ImplantViewMgr::slotSetImplantPlanePos);
	connect(view_sagittal, &ViewImplantSagittal::sigSelectImplant, this, &ImplantViewMgr::slotSelectImplantFromSagittal);
	connect(view_sagittal, &ViewImplantSagittal::sigTranslateImplant, this, &ImplantViewMgr::slotTranslateImplantFromSagittal);
	connect(view_sagittal, &ViewImplantSagittal::sigRotateImplant, this, &ImplantViewMgr::slotRotateImplantFromSagittal);
	connect(view_sagittal, &ViewImplantSagittal::sigUpdateImplantImages, this, &ImplantViewMgr::slotUpdateAllImplantImagesFromSagittal);
	connect(view_sagittal, &ViewImplantSagittal::sigRotateView, this, &ImplantViewMgr::slotRotateSagittal);
	connect(view_sagittal, &ViewImplantSagittal::sigZoomDone, this, &ImplantViewMgr::slotSagittalViewZoonDoneEvent);

	const ViewImplantPano* view_pano = view_pano_.get();
	connect(view_pano, &ViewImplantPano::sigSelectImplant, this, &ImplantViewMgr::slotSelectImplantFromPano);
	connect(view_pano, &ViewImplantPano::sigTranslateImplant, this, &ImplantViewMgr::slotTranslateImplantFromPano);
	connect(view_pano, &ViewImplantPano::sigRotateImplant, this, &ImplantViewMgr::slotRotateImplantFromPano);
	connect(view_pano, &ViewImplantPano::sigTranslateImplantIn3D, this, &ImplantViewMgr::slotTranslateImplantFromPano3D);
	connect(view_pano, &ViewImplantPano::sigRotateImplantIn3D, this, &ImplantViewMgr::slotRotateImplantFromPano3D);
	connect(view_pano, &ViewImplantPano::sigPlacedImplant, this, &ImplantViewMgr::slotPlacedImplantFromPano);
	connect(view_pano, &ViewImplantPano::sigUpdateImplantImages, this, &ImplantViewMgr::slotUpdateAllImplantImagesFromPano);
	connect(view_pano, &ViewImplantPano::sigUpdateImplantImagesIn3D, this, &ImplantViewMgr::slotUpdateAllImplantImagesFromPano3D);
	connect(view_pano, &ViewImplantPano::sigDeleteImplant, this, &ImplantViewMgr::sigDeleteImplantFromView);
	connect(view_pano, &ViewImplantPano::sigImplantHovered, this, &ImplantViewMgr::slotImplantHoveredFromPano);

	const ViewImplant3D* view_3d = view_3d_.get();
	connect(view_3d, &ViewImplant3D::sigProcessedLightEvent, this, &ImplantViewMgr::slotProcessedLightEvent);
	connect(view_3d, &ViewImplant3D::sigTranslateImplant, this, &ImplantViewMgr::slotTranslateImplantFrom3D);
	connect(view_3d, &ViewImplant3D::sigRotateImplant, this, &ImplantViewMgr::slotRotateImplantFrom3D);
	connect(view_3d, &ViewImplant3D::sigSelectImplant, this, &ImplantViewMgr::slotSelectImplantFrom3D);
	connect(view_3d, &ViewImplant3D::sigUpdateImplantImages, this, &ImplantViewMgr::slotUpdateAllImplantImagesFrom3D);
	connect(view_3d, &ViewImplant3D::sigRotated, this, &ImplantViewMgr::slotRotatedView3D);

	connect(view_bd_.get(), &ViewBoneDensity::sigRotated, this, &ImplantViewMgr::slotRotatedBoneDensityView);

	for (const auto& elem : view_cross_section_)
	{
		const ViewImplantCrossSection* view_cs = elem.second.get();
		connect(view_cs, &ViewImplantCrossSection::sigImplantHovered, this, &ImplantViewMgr::slotImplantHoveredFromCS);
		connect(view_cs, &ViewImplantCrossSection::sigSetImplantPosition, this, &ImplantViewMgr::slotSetImplantPositionFromCross);
		connect(view_cs, &ViewImplantCrossSection::sigTranslateImplant, this, &ImplantViewMgr::slotTranslateImplantFromCross);
		connect(view_cs, &ViewImplantCrossSection::sigRotateImplant, this, &ImplantViewMgr::slotRotateImplantFromCross);
		connect(view_cs, &ViewImplantCrossSection::sigUpdateImplantImages, this, &ImplantViewMgr::slotUpdateAllImplantImagesFromCross);
		connect(view_cs, &ViewImplantCrossSection::sigSelectImplant, this, &ImplantViewMgr::slotSelectImplantFromCross);
		connect(view_cs, &ViewImplantCrossSection::sigPlacedImplant, this, &ImplantViewMgr::slotPlacedImplantFromCross);
		connect(view_cs, &ViewImplantCrossSection::sigDeleteImplant, this, &ImplantViewMgr::sigDeleteImplantFromView);
		connect(view_cs, &ViewImplantCrossSection::sigMaximize, this, &ImplantViewMgr::slotMaximizeSingleCrossSectionView);
	}

#ifdef WILL3D_EUROPE
	connect(view_sagittal, &ViewImplantSagittal::sigShowButtonListDialog, this, &ImplantViewMgr::sigShowButtonListDialog);
	connect(view_arch, &ViewImplantArch::sigShowButtonListDialog, this, &ImplantViewMgr::sigShowButtonListDialog);
	connect(view_3d, &ViewImplant3D::sigShowButtonListDialog, this, &ImplantViewMgr::sigShowButtonListDialog);
	connect(view_pano, &ViewImplantPano::sigShowButtonListDialog, this, &ImplantViewMgr::sigShowButtonListDialog);
	
	connect(view_sagittal, &ViewImplantSagittal::sigSyncControlButton, [=](bool is_on)
	{
		view_arch_.get()->SetSyncControlButton(is_on);
		view_3d_.get()->SetSyncControlButton(is_on);
		view_pano_.get()->SetSyncControlButton(is_on);
	});

	connect(view_arch, &ViewImplantArch::sigSyncControlButton, [=](bool is_on)
	{
		view_sagittal_.get()->SetSyncControlButton(is_on);
		view_3d_.get()->SetSyncControlButton(is_on);
		view_pano_.get()->SetSyncControlButton(is_on);
	});

	connect(view_3d, &ViewImplant3D::sigSyncControlButton, [=](bool is_on)
	{
		view_sagittal_.get()->SetSyncControlButton(is_on);
		view_arch_.get()->SetSyncControlButton(is_on);
		view_pano_.get()->SetSyncControlButton(is_on);
	});

	connect(view_pano, &ViewImplantPano::sigSyncControlButton, [=](bool is_on)
	{
		view_sagittal_.get()->SetSyncControlButton(is_on);
		view_arch_.get()->SetSyncControlButton(is_on);
		view_3d_.get()->SetSyncControlButton(is_on);
	});
#endif // WILL3D_EUROPE
}

void ImplantViewMgr::ConnectPanoMenus()
{
	BasePanoViewMgr::ConnectPanoMenus();

	connect(task_tool_.get(), &ImplantTaskTool::sigImplantSagittalRotate, this, &ImplantViewMgr::slotChangedSagittalRotateAngle);
	connect(view_bd_.get(), &ViewBoneDensity::sigBoneDensityPopupMode, task_tool_.get(), &ImplantTaskTool::slotBoneDensityPopup);
	connect(task_tool_.get(), &ImplantTaskTool::sigImplantBDSyncPopupStatus, this, &ImplantViewMgr::slotImplantBDSyncPopupStatus);
}

void ImplantViewMgr::DisconnectPanoMenus()
{
	BasePanoViewMgr::DisconnectPanoMenus();

	disconnect(task_tool_.get(), &ImplantTaskTool::sigImplantSagittalRotate, this, &ImplantViewMgr::slotChangedSagittalRotateAngle);
	disconnect(view_bd_.get(), &ViewBoneDensity::sigBoneDensityPopupMode, task_tool_.get(), &ImplantTaskTool::slotBoneDensityPopup);
	disconnect(task_tool_.get(), &ImplantTaskTool::sigImplantBDSyncPopupStatus, this, &ImplantViewMgr::slotImplantBDSyncPopupStatus);
}

/**=================================================================================================
Public functions
*===============================================================================================**/
#ifndef WILL3D_VIEWER
void ImplantViewMgr::exportProject(ProjectIOImplant& out)
{
	float scene_scale = 0.0f, scene_to_gl = 0.0f;
	QPointF trans_gl;

	out.InitImplantTab();

	out.SaveCSAngle(task_tool_base_->GetCrossSectionAngle());

	view_arch_->exportProject(&scene_scale, &scene_to_gl, &trans_gl);
	out.InitializeView(ProjectIOImplant::ViewType::IMP_ARCH);
	out.GetViewIO().SaveViewInfo(scene_scale, scene_to_gl, trans_gl.x(),
		trans_gl.y());

	view_pano_->exportProject(&scene_scale, &scene_to_gl, &trans_gl);
	out.InitializeView(ProjectIOImplant::ViewType::IMP_PANO);
	out.GetViewIO().SaveViewInfo(scene_scale, scene_to_gl, trans_gl.x(),
		trans_gl.y());
	view_pano_->ExportProjectForMeasure3D(out.GetViewIO());

	for (auto& view : view_cross_section_)
	{
		view.second->exportProject(&scene_scale, &scene_to_gl, &trans_gl);
		out.InitializeView(ProjectIOImplant::ViewType::IMP_CS, view.first);
		out.GetViewIO().SaveViewInfo(scene_scale, scene_to_gl, trans_gl.x(),
			trans_gl.y());
	}

	view_sagittal_->exportProject(&scene_scale, &scene_to_gl, &trans_gl);
	out.InitializeView(ProjectIOImplant::ViewType::IMP_SAGITTAL);
	out.GetViewIO().SaveViewInfo(scene_scale, scene_to_gl, trans_gl.x(),
		trans_gl.y());

	view_3d_->exportProject(&scene_scale, &scene_to_gl, &trans_gl);
	out.InitializeView(ProjectIOImplant::ViewType::IMP_IMPLANT3D);
	out.GetViewIO().SaveViewInfo(scene_scale, scene_to_gl, trans_gl.x(),
		trans_gl.y());
	view_3d_->ExportProjectForMeasure3D(out.GetViewIO());
}

void ImplantViewMgr::importProject(ProjectIOImplant& in,
	const bool& is_counterpart_exists)
{
	float scene_scale = 0.0f, scene_to_gl = 0.0f;
	float trans_gl_x = 0.0f, trans_gl_y = 0.0f;

	task_tool_->blockSignals(true);

	float degree = 0.0f;
	in.LoadCSAngle(degree);
	task_tool_base_->SetCrossSectionAngle(degree);

	in.InitializeView(ProjectIOImplant::ViewType::IMP_ARCH);
	in.GetViewIO().LoadViewInfo(scene_scale, scene_to_gl, trans_gl_x, trans_gl_y);
	view_arch_->importProject(scene_scale, scene_to_gl,
		QPointF(trans_gl_x, trans_gl_y),
		is_counterpart_exists);

	in.InitializeView(ProjectIOImplant::ViewType::IMP_PANO);
	in.GetViewIO().LoadViewInfo(scene_scale, scene_to_gl, trans_gl_x, trans_gl_y);
	view_pano_->importProject(scene_scale, scene_to_gl,
		QPointF(trans_gl_x, trans_gl_y),
		is_counterpart_exists);
	view_pano_->ImportProjectForMeasure3D(in.GetViewIO());

	// sagittal 은 counterpart 없음
	in.InitializeView(ProjectIOImplant::ViewType::IMP_SAGITTAL);
	in.GetViewIO().LoadViewInfo(scene_scale, scene_to_gl, trans_gl_x, trans_gl_y);
	view_sagittal_->importProject(scene_scale, scene_to_gl,
		QPointF(trans_gl_x, trans_gl_y));

	// 3D 는 counterpart 없음
	in.InitializeView(ProjectIOImplant::ViewType::IMP_IMPLANT3D);
	in.GetViewIO().LoadViewInfo(scene_scale, scene_to_gl, trans_gl_x, trans_gl_y);
	view_3d_->importProject(scene_scale, scene_to_gl,
		QPointF(trans_gl_x, trans_gl_y));
	view_3d_->ImportProjectForMeasure3D(in.GetViewIO());

	int cross_cnt = cross_section_count();
	for (int i = 0; i < cross_cnt; ++i)
	{
		in.InitializeView(ProjectIOImplant::ViewType::IMP_CS, i);
		in.GetViewIO().LoadViewInfo(scene_scale, scene_to_gl, trans_gl_x,
			trans_gl_y);
		bool is_update_resource = (i == cross_cnt - 1) ? true : false;
		view_cross_section_[i]->importProject(
			scene_scale, scene_to_gl, QPointF(trans_gl_x, trans_gl_y),
			is_counterpart_exists, is_update_resource);
	}

	const auto& res_implant =
		ResourceContainer::GetInstance()->GetImplantResource();
	task_tool_->SetMemoText(res_implant.memo());
	task_tool_->blockSignals(false);
}
#endif
void ImplantViewMgr::SetImplantMenu(
	const std::shared_ptr<ImplantTaskTool>& implant_menu)
{
	task_tool_ = implant_menu;
	BasePanoViewMgr::set_task_tool_base(
		std::static_pointer_cast<BasePanoTaskTool>(implant_menu));
}

void ImplantViewMgr::SetVisibleViews(bool visible)
{
	BasePanoViewMgr::SetVisibleViews(visible);
	view_sagittal_->setVisible(visible);
	view_3d_->setVisible(visible);
	if (task_tool_ && !visible)
	{
		task_tool_.get()->slotBoneDensityPopup(visible);
	}
}

/**********************************************************************************************
Sets implant selected.
Implant tab의 AddImplant를 통해 결정된 implant의 선택 상태에 따라
각 뷰들을 업데이트 하기 위한 함수

@author	Seo Seok Man
@date	2018-04-27

@param	implant_id	Identifier for the implant.
@param	selected  	True if selected.
 **********************************************************************************************/
void ImplantViewMgr::SetImplantSelectedStatusFromTab(int implant_id, bool selected)
{
	pano_engine()->SetImplantSelected(implant_id, selected);

	if (selected)
	{
		MoveSagittalPlaneByImplant(implant_id);
		MoveCrossSectionByImplant(implant_id);
	}

	pano_engine()->ReconPanoramaImplantMask(view_pano_->GetSceneScale());

	view_bd_->SyncImplant3DCameraMatrix(view_3d_->GetRotateMatrix(),
		view_3d_->GetReorienMatrix(),
		view_3d_->GetViewMatrix());

	for (int i = 0; i < cross_section_count(); i++)
	{
		view_cross_section_[i]->UpdateImplantHandleAndSpec();
		view_cross_section_[i]->UpdateCrossSection();
	}
	view_arch_->UpdateImplantHandleAndSpec();
	view_pano_->UpdateImplantHandleAndSpec();
	view_sagittal_->UpdateImplantHandleAndSpec();
	view_3d_->UpdateImplantSpec();

	view_bd_->UpdateBoneDensity();
	view_3d_->UpdateVolume();
	view_sagittal_->SceneUpdate();
	view_pano_->UpdatedPano();
	view_arch_->UpdateSlice();
}

void ImplantViewMgr::DeleteImplantFromTab(int implant_id)
{
	view_arch_->DeleteImplant(implant_id);
	view_sagittal_->DeleteImplant(implant_id);
	view_pano_->DeleteImplant(implant_id);
	view_3d_->DeleteImplant(implant_id);
	for (int i = 0; i < cross_section_count(); i++)
		view_cross_section_[i]->DeleteImplant(implant_id);

	pano_engine()->CheckCollideImplant(view_3d_->GetProjectionViewMatrix());
	ReconImplantResource();
	// 다른 뷰들 업데이트
	UpdateSceneAllViews();
}

void ImplantViewMgr::DeleteAllImplantsFromTab()
{
	view_arch_->DeleteAllImplants();
	view_sagittal_->DeleteAllImplants();
	view_pano_->DeleteAllImplants();
	view_3d_->DeleteAllImplants();
	for (int i = 0; i < cross_section_count(); i++)
		view_cross_section_[i]->DeleteAllImplants();

	ReconImplantResource();

	// 다른 뷰들 업데이트
	UpdateSceneAllViews();
}

void ImplantViewMgr::ChangeImplantModelFromTab(int implant_id)
{
#if 1
	MoveSagittalPlaneByImplant(implant_id);
	MoveCrossSectionByImplant(implant_id);
#endif

	pano_engine()->ReconPanoramaImplantMask(view_pano_->GetSceneScale());
	view_arch_->ChangeSelectedImplantSpec();
	for (int i = 0; i < cross_section_count(); i++)
		view_cross_section_[i]->ChangeSelectedImplantSpec();
	view_pano_->ChangeSelectedImplantSpec();
	view_3d_->ChangeSelectedImplantSpec();

	view_bd_->SyncImplant3DCameraMatrix(view_3d_->GetRotateMatrix(),
		view_3d_->GetReorienMatrix(),
		view_3d_->GetViewMatrix());
	view_bd_->UpdateBoneDensity();
	view_3d_->UpdateVolume();
	view_pano_->UpdatedPano();
	view_arch_->UpdateSlice();
}
void ImplantViewMgr::MoveViewsToSelectedMeasure(
	const common::ViewTypeID& view_type, const unsigned int& measure_id)
{
	common::measure::VisibilityParams visibility_params;
	GetMeasureParamsInView(view_type, measure_id, &visibility_params);

	switch (view_type)
	{
	case common::ViewTypeID::PANO_ARCH:
		MoveArchViewToSelectedMeasure(visibility_params);
		break;
	case common::ViewTypeID::PANO:
		MovePanoViewToSelectedMeasure(visibility_params);
		break;
	case common::ViewTypeID::CROSS_SECTION:
		MoveCrossSectionViewToSelectedMeasure(visibility_params);
		break;
	case common::ViewTypeID::IMPLANT_SAGITTAL:
		MoveSagittalViewToSelectedMeasure(visibility_params);
		break;
	default:
		common::Logger::instance()->PrintAndAssert(
			common::LogType::ERR,
			"PanoViewMgr::MoveViewsToSelectedMeasure : invalid view type id");
		break;
	}
}
/**=================================================================================================
Private slots
*===============================================================================================**/

void ImplantViewMgr::VisibleNerve(bool is_visible)
{
	BasePanoViewMgr::VisibleNerve(is_visible);

	if (!view_sagittal_->isVisible() &&
		!view_3d_->isVisible())
	{
		return;
	}

	view_sagittal_->SceneUpdate();
	view_3d_->UpdateVolume();
}

void ImplantViewMgr::VisibleImplant(bool is_visible)
{
	BasePanoViewMgr::VisibleImplant(is_visible);

	if (!view_arch_->isVisible() &&
		!view_sagittal_->isVisible() &&
		!view_3d_->isVisible())
	{
		return;
	}

	for (int i = 0; i < cross_section_count(); i++)
		view_cross_section_[i]->UpdateImplantHandleAndSpec();
	view_arch_->UpdateImplantHandleAndSpec();
	view_sagittal_->UpdateImplantHandleAndSpec();
	view_3d_->UpdateImplantSpec();

	view_sagittal_->SceneUpdate();
	view_3d_->UpdateVolume();
}

void ImplantViewMgr::slotProcessedLightEvent()
{
	BasePanoViewMgr::slotProcessedLightEvent();
	view_sagittal_->SceneUpdate();
	view_3d_->UpdateVolume();
}

// Initialize 시에 arch의 중앙에 sagittal line을 놓는다.
void ImplantViewMgr::slotArchUpdateFinishFromPanoArch()
{
	BasePanoViewMgr::slotArchUpdateFinishFromPanoArch();

	InitSagittalResource();

	const auto& res_pano = ResourceContainer::GetInstance()->GetPanoResource();
	const auto& curve_ = res_pano.GetCurrentCurveData();
	QPointF pano_pos((double)res_pano.GetPanoPlaneWidth() * 0.5,
		(double)res_pano.GetPanoPlaneHeight() * 0.5);
	glm::vec3 vol_pos = pano_engine()->MapPanoPlaneToVol(pano_pos);
	UpdateSagittal(vol_pos);

	SetSagittalLineInArchView(vol_pos, pano_pos);
	InitAxialLineInSagittalView();
}

void ImplantViewMgr::slotTranslateZfromPanoArch(float z_pos_vol)
{
	BasePanoViewMgr::slotTranslateZfromPanoArch(z_pos_vol);

	if (pano_engine()->IsValidSagittal() && pano_engine()->IsValidPanorama())
	{
		QPointF pt_pano_plane =
			pano_engine()->MapVolToPanoPlane(view_arch_->GetCenterPosition());
		view_sagittal_->SetAxialLine(pt_pano_plane);
	}
}

void ImplantViewMgr::slotArchShifteFromPanoArch(float shifted_value_in_vol)
{
	BasePanoViewMgr::slotArchShifteFromPanoArch(shifted_value_in_vol);
	InitAxialLineInSagittalView();

	// const auto& implant_resource =
	// ResourceContainer::GetInstance()->GetImplantResource(); const auto&
	// implant_datas = implant_resource.data();

	// for (const auto& elem : implant_datas) {
	//	pano_engine()->SetImplantPositionInPanoPlane(elem.second->id());
	//	pano_engine()->SetImplantAxisPointPanoPlane(elem.second->id());
	//	view_pano_->UpdateImplantHandleAndSpec();
	//}
}

// 1. pano_engine()->SetImplantPositionVolAndPano3D implant position 결정
// 2. move xxx -> position에 따라 이동
// 3. pano_engine()->SetImplantPositionInPanoPlane move 결과로 세팅
void ImplantViewMgr::slotUpdateAllImplantImagesFromPano(
	int implant_id, const QPointF& pt_pano_plane)
{
	glm::vec3 vol_pt = pano_engine()->MapPanoPlaneToVol(pt_pano_plane);
	pano_engine()->SetImplantPositionVolAndPano3D(implant_id, vol_pt);
	pano_engine()->SetImplantAxisPointVolAndPano3D(implant_id);
	pano_engine()->SetImplantPositionInPanoPlane(implant_id);
	pano_engine()->SetImplantAxisPointPanoPlane(implant_id);

	MoveSagittalPlaneByImplant(implant_id);
	MoveCrossSectionByImplant(implant_id);

	view_3d_->UpdateImplantSpec();
	for (int i = 0; i < cross_section_count(); i++)
		view_cross_section_[i]->UpdateImplantHandleAndSpec();
	view_arch_->UpdateImplantHandleAndSpec();

	view_bd_->SyncImplant3DCameraMatrix(view_3d_->GetRotateMatrix(),
		view_3d_->GetReorienMatrix(),
		view_3d_->GetViewMatrix());
	view_bd_->UpdateBoneDensity();
	view_arch_->UpdateSlice();
	view_3d_->UpdateVolume();
}

void ImplantViewMgr::slotUpdateAllImplantImagesFromPano3D(int implant_id)
{
	const auto& res_implant =
		ResourceContainer::GetInstance()->GetImplantResource();
	auto iter = res_implant.data().find(implant_id);
	if (iter == res_implant.data().end())
	{
		return;
	}

	glm::vec3 vol_pt =
		pano_engine()->MapPano3DToVol(iter->second->position_in_pano());
	pano_engine()->SetImplantPositionVolAndPano3D(implant_id, vol_pt);
	pano_engine()->SetImplantAxisPointVolAndPano3D(implant_id);
	pano_engine()->SetImplantPositionInPanoPlane(implant_id);
	pano_engine()->SetImplantAxisPointPanoPlane(implant_id);

	MoveSagittalPlaneByImplant(implant_id);
	MoveCrossSectionByImplant(implant_id);

	pano_engine()->ReconPanoramaImplantMask(view_pano_->GetSceneScale());
	for (int i = 0; i < cross_section_count(); i++)
		view_cross_section_[i]->UpdateImplantHandleAndSpec();
	view_arch_->UpdateImplantHandleAndSpec();
	view_3d_->UpdateImplantSpec();
	view_3d_->UpdateVolume();

	view_bd_->SyncImplant3DCameraMatrix(view_3d_->GetRotateMatrix(),
		view_3d_->GetReorienMatrix(),
		view_3d_->GetViewMatrix());
	view_bd_->UpdateBoneDensity();
	view_arch_->UpdateSlice();
}

void ImplantViewMgr::slotTranslateImplantFromPano(
	int implant_id, const QPointF& pt_pano_plane)
{
	glm::vec3 vol_pt = pano_engine()->MapPanoPlaneToVol(pt_pano_plane);
	pano_engine()->SetImplantPositionVolAndPano3D(implant_id, vol_pt);
	pano_engine()->SetImplantPositionInPanoPlane(implant_id);

	pano_engine()->CheckCollideImplant(view_3d_->GetProjectionViewMatrix());
	pano_engine()->ReconPanoramaImplantMask(view_pano_->GetSceneScale());
	pano_engine()->SetImplantAxisPointPanoPlane(implant_id);
	pano_engine()->SetImplantAxisPointPano3D(implant_id);

	view_bd_->UpdateBoneDensity();
}

void ImplantViewMgr::slotRotateImplantFromPano(int implant_id,
	float delta_degree)
{
	pano_engine()->RotateImplantInPanoPlane(implant_id, delta_degree);
	pano_engine()->SetImplantAxisPointPanoPlane(implant_id);
	pano_engine()->SetImplantAxisPointPano3D(implant_id);
	pano_engine()->CheckCollideImplant(view_3d_->GetProjectionViewMatrix());
	pano_engine()->ReconPanoramaImplantMask(view_pano_->GetSceneScale());

	view_bd_->UpdateBoneDensity();
}

void ImplantViewMgr::slotTranslateImplantFromPano3D(
	const int& implant_id, const glm::vec3& translate)
{
	pano_engine()->TranslateImplantInPano3D(implant_id, translate);
	pano_engine()->SetImplantAxisPointPano3D(implant_id);
	pano_engine()->CheckCollideImplant(view_3d_->GetProjectionViewMatrix());
	view_bd_->UpdateBoneDensity();
}

void ImplantViewMgr::slotRotateImplantFromPano3D(const int& implant_id,
	const glm::vec3& rotate_axes,
	const float& rotate_degree)
{
	pano_engine()->RotateImplantInPano3D(implant_id, rotate_axes, rotate_degree);
	pano_engine()->SetImplantAxisPointPano3D(implant_id);
	pano_engine()->CheckCollideImplant(view_3d_->GetProjectionViewMatrix());
	view_bd_->UpdateBoneDensity();
}
void ImplantViewMgr::slotPlacedImplantFromPano()
{
	emit sigImplantPlaced();

	const auto& res_implant =
		ResourceContainer::GetInstance()->GetImplantResource();
	int implant_id = res_implant.selected_implant_id();
	pano_engine()->ReconPanoramaImplantMask(view_pano_->GetSceneScale());
	pano_engine()->SetImplantAxisPointVolAndPano3D(implant_id);

	MoveCrossSectionByImplant(implant_id);
	MoveSagittalPlaneByImplant(implant_id);

	view_pano_->UpdateImplantHandleAndSpec();
	for (int i = 0; i < cross_section_count(); i++)
		view_cross_section_[i]->UpdateImplantHandleAndSpec();
	view_arch_->UpdateImplantHandleAndSpec();
	view_3d_->UpdateImplantSpec();

	view_bd_->SyncImplant3DCameraMatrix(view_3d_->GetRotateMatrix(),
		view_3d_->GetReorienMatrix(),
		view_3d_->GetViewMatrix());
	view_bd_->UpdateBoneDensity();
	view_3d_->UpdateVolume();
	view_arch_->UpdateSlice();
}

void ImplantViewMgr::slotPlacedImplantFromCross()
{
	emit sigImplantPlaced();

	const auto& res_implant =
		ResourceContainer::GetInstance()->GetImplantResource();
	int implant_id = res_implant.selected_implant_id();
	pano_engine()->ReconPanoramaImplantMask(view_pano_->GetSceneScale());
	pano_engine()->SetImplantAxisPointVolAndPano3D(implant_id);

	//MoveCrossSectionByImplant(implant_id);
	MoveSagittalPlaneByImplant(implant_id);

	view_pano_->UpdateImplantHandleAndSpec();
	for (int i = 0; i < cross_section_count(); i++)
	{
		QObject* sender = QObject::sender();
		if (view_cross_section_[i].get() == sender)
		{
			continue;
		}
		view_cross_section_[i]->UpdateImplantHandleAndSpec();
		view_cross_section_[i]->UpdateCrossSection();
	}
	view_arch_->UpdateImplantHandleAndSpec();
	view_3d_->UpdateImplantSpec();

	view_bd_->SyncImplant3DCameraMatrix(
		view_3d_->GetRotateMatrix(),
		view_3d_->GetReorienMatrix(),
		view_3d_->GetViewMatrix()
	);
	view_bd_->UpdateBoneDensity();
	view_3d_->UpdateVolume();
	view_arch_->UpdateSlice();
	view_pano_->UpdatedPano();
}

void ImplantViewMgr::slotReconTypeChangedFromPano()
{
	BasePanoViewMgr::slotReconTypeChangedFromPano();
	view_pano_->UpdateImplantHandleAndSpec();
}

void ImplantViewMgr::slotSelectImplantFromPano(int implant_id)
{
	pano_engine()->SetImplantSelected(implant_id, true);

	MoveSagittalPlaneByImplant(implant_id);
	MoveCrossSectionByImplant(implant_id);

	pano_engine()->ReconPanoramaImplantMask(view_pano_->GetSceneScale());
	view_pano_->UpdateImplantHandleAndSpec();
	for (int i = 0; i < cross_section_count(); i++)
		view_cross_section_[i]->UpdateImplantHandleAndSpec();
	view_arch_->UpdateImplantHandleAndSpec();
	view_3d_->UpdateImplantSpec();

	view_bd_->SyncImplant3DCameraMatrix(view_3d_->GetRotateMatrix(),
		view_3d_->GetReorienMatrix(),
		view_3d_->GetViewMatrix());
	view_bd_->UpdateBoneDensity();
	view_3d_->UpdateVolume();
	view_pano_->UpdatedPano();
	view_arch_->UpdateSlice();

	// implant panel 에 선택된 상태 sync
	emit sigImplantSelectionChanged(implant_id);
}

void ImplantViewMgr::slotSelectImplantFrom3D(int implant_id)
{
	pano_engine()->SetImplantSelected(implant_id, true);

	MoveSagittalPlaneByImplant(implant_id);
	MoveCrossSectionByImplant(implant_id);

	pano_engine()->ReconPanoramaImplantMask(view_pano_->GetSceneScale());
	view_pano_->UpdateImplantHandleAndSpec();
	for (int i = 0; i < cross_section_count(); i++)
		view_cross_section_[i]->UpdateImplantHandleAndSpec();
	view_arch_->UpdateImplantHandleAndSpec();
	view_3d_->UpdateImplantSpec();

	view_bd_->SyncImplant3DCameraMatrix(view_3d_->GetRotateMatrix(),
		view_3d_->GetReorienMatrix(),
		view_3d_->GetViewMatrix());
	view_bd_->UpdateBoneDensity();
	view_3d_->UpdateVolume();
	view_pano_->UpdatedPano();
	view_arch_->UpdateSlice();

	// implant panel 에 선택된 상태 sync
	emit sigImplantSelectionChanged(implant_id);
}
void ImplantViewMgr::slotUpdateAllImplantImagesFrom3D(int implant_id)
{
	pano_engine()->SetImplantAxisPointVolAndPano3D(implant_id);
	pano_engine()->SetImplantPositionInPanoPlane(implant_id);
	pano_engine()->SetImplantAxisPointPanoPlane(implant_id);

	MoveSagittalPlaneByImplant(implant_id);
	MoveCrossSectionByImplant(implant_id);

	pano_engine()->ReconPanoramaImplantMask(view_pano_->GetSceneScale());
	view_pano_->UpdateImplantHandleAndSpec();
	for (int i = 0; i < cross_section_count(); i++)
		view_cross_section_[i]->UpdateImplantHandleAndSpec();
	view_arch_->UpdateImplantHandleAndSpec();
	view_3d_->UpdateImplantSpec();

	view_bd_->SyncImplant3DCameraMatrix(view_3d_->GetRotateMatrix(),
		view_3d_->GetReorienMatrix(),
		view_3d_->GetViewMatrix());
	view_bd_->UpdateBoneDensity();
	view_3d_->UpdateVolume();
	view_pano_->UpdatedPano();
	view_arch_->UpdateSlice();
}
void ImplantViewMgr::slotRotatedView3D()
{
	view_bd_->SyncImplant3DCameraMatrix(view_3d_->GetRotateMatrix(),
		view_3d_->GetReorienMatrix(),
		view_3d_->GetViewMatrix());
	view_bd_->UpdateBoneDensity();
}
void ImplantViewMgr::slotRotatedBoneDensityView()
{
	view_3d_->SyncBoneDensityRotateMatrix(view_bd_->GetRotateMatrix());
	view_3d_->UpdateImplantSpec();
	view_3d_->UpdateVolume();
}
void ImplantViewMgr::slotTranslateImplantFrom3D(const int& implant_id,
	const glm::vec3& translate)
{
	pano_engine()->TranslateImplantIn3D(implant_id, translate);
	pano_engine()->SetImplantAxisPointVol(implant_id);
	pano_engine()->CheckCollideImplant(view_3d_->GetProjectionViewMatrix());
	view_bd_->UpdateBoneDensity();
}

void ImplantViewMgr::slotRotateImplantFrom3D(const int& implant_id,
	const glm::vec3& rotate_axes,
	const float& rotate_degree)
{
	pano_engine()->RotateImplantIn3D(implant_id, rotate_axes, rotate_degree);
	pano_engine()->SetImplantAxisPointVol(implant_id);
	pano_engine()->CheckCollideImplant(view_3d_->GetProjectionViewMatrix());
	view_bd_->UpdateBoneDensity();
}

void ImplantViewMgr::slotImplantHoveredFromCS(const int& cross_section_id,
	const QPointF& pt_cs_plane,
	int& hovered_implant_id,
	QPointF& implant_pos_in_scene)
{
	pano_engine()->HoveredImplantInCSPlane(
		cross_section_id, pt_cs_plane, hovered_implant_id, implant_pos_in_scene);
}

void ImplantViewMgr::slotImplantBDSyncPopupStatus(bool popup)
{
	view_bd_->SyncPopupStatus(popup);
	emit sigSyncBDViewStatus();
}

void ImplantViewMgr::slotMaximizeSingleCrossSectionView()
{
	single_cross_section_maximized_ = !single_cross_section_maximized_;

	QObject* sender = QObject::sender();

	for (int i = 0; i < cross_section_count(); ++i)
	{
		if (view_cross_section_[i].get() == sender)
		{
			continue;
		}

		view_cross_section_[i]->blockSignals(single_cross_section_maximized_);
		view_cross_section_[i]->setVisible(!single_cross_section_maximized_);
	}

	emit sigMaximizeSingleCrossSectionView(single_cross_section_maximized_);
}

void ImplantViewMgr::slotRotateSagittal(float delta_angle)
{
	task_tool_.get()->UpdateRotateAngle((double)delta_angle);
}

void ImplantViewMgr::slotChangedSagittalRotateAngle(double /*angle*/)
{
	if (!pano_engine()->IsValidPanorama()) return;

	InitSagittalParams();

	UpdateSagittal(view_arch_->GetCurrentSagittalCenterPos());
	view_arch_->RotateSagittalLine();
}

// arch 선상의 sagittal position을 얻기 위해
// vol -> pano -> vol 순서로 변환한다.
void ImplantViewMgr::slotGetPanoPosition(const glm::vec3& clicked_vol_pos)
{
	const auto& res_pano = ResourceContainer::GetInstance()->GetPanoResource();
	QPointF pano_pos(
		pano_engine()->MapVolToPanoPlane_Sagittal(clicked_vol_pos).x(),
		static_cast<double>(res_pano.pano_3d_height()) / 2.0);
	glm::vec3 vol_pos = pano_engine()->MapPanoPlaneToVol(pano_pos);
	UpdateSagittal(vol_pos);
	SetSagittalLineInArchView(vol_pos, pano_pos);
	view_sagittal_->DisableImplantHandleAndSpec();
}

void ImplantViewMgr::slotSelectImplantFromArch(int implant_id)
{
	pano_engine()->SetImplantSelected(implant_id, true);

	MoveSagittalPlaneByImplant(implant_id);
	MoveCrossSectionByImplant(implant_id);

	pano_engine()->ReconPanoramaImplantMask(view_pano_->GetSceneScale());
	view_pano_->UpdateImplantHandleAndSpec();
	for (int i = 0; i < cross_section_count(); i++)
		view_cross_section_[i]->UpdateImplantHandleAndSpec();
	view_arch_->UpdateImplantHandleAndSpec();
	view_3d_->UpdateImplantSpec();

	view_bd_->SyncImplant3DCameraMatrix(view_3d_->GetRotateMatrix(),
		view_3d_->GetReorienMatrix(),
		view_3d_->GetViewMatrix());
	view_bd_->UpdateBoneDensity();
	view_3d_->UpdateVolume();
	view_pano_->UpdatedPano();

	// implant panel 에 선택된 상태 sync
	emit sigImplantSelectionChanged(implant_id);
}

void ImplantViewMgr::slotTranslateImplantFromArch(int implant_id,
	const glm::vec3& vol_pos)
{
	pano_engine()->SetImplantPositionVolAndPano3D(implant_id, vol_pos);
	pano_engine()->SetImplantAxisPointVol(implant_id);
	pano_engine()->CheckCollideImplant(view_3d_->GetProjectionViewMatrix());

	view_bd_->UpdateBoneDensity();
}

void ImplantViewMgr::slotRotateImplantFromArch(int implant_id,
	const glm::vec3& axis,
	float delta_degree)
{
	pano_engine()->RotateImplantIn3D(implant_id, axis, delta_degree);
	pano_engine()->SetImplantAxisPointVol(implant_id);
	pano_engine()->CheckCollideImplant(view_3d_->GetProjectionViewMatrix());
	view_bd_->UpdateBoneDensity();
}

void ImplantViewMgr::slotUpdateAllImplantImagesFromArch(
	int implant_id, const glm::vec3& vol_pos)
{
	pano_engine()->SetImplantPositionVolAndPano3D(implant_id, vol_pos);
	pano_engine()->SetImplantAxisPointVolAndPano3D(implant_id);
	pano_engine()->SetImplantPositionInPanoPlane(implant_id, false);
	pano_engine()->SetImplantAxisPointPanoPlane(implant_id);

	MoveSagittalPlaneByImplant(implant_id);
	MoveCrossSectionByImplant(implant_id);

	pano_engine()->ReconPanoramaImplantMask(view_pano_->GetSceneScale());
	view_pano_->UpdateImplantHandleAndSpec();
	view_3d_->UpdateImplantSpec();
	for (int i = 0; i < cross_section_count(); i++)
		view_cross_section_[i]->UpdateImplantHandleAndSpec();

	view_bd_->SyncImplant3DCameraMatrix(view_3d_->GetRotateMatrix(),
		view_3d_->GetReorienMatrix(),
		view_3d_->GetViewMatrix());
	view_bd_->UpdateBoneDensity();
	view_3d_->UpdateVolume();
	view_pano_->UpdatedPano();
}

void ImplantViewMgr::slotSetImplantPositionFromCross(const int implant_id, const glm::vec3& vol_pos)
{
	pano_engine()->SetImplantPositionVolAndPano3D(implant_id, vol_pos);
	pano_engine()->SetImplantPositionInPanoPlane(implant_id);

	pano_engine()->SetImplantAxisPointVol(implant_id);
	pano_engine()->ReconPanoramaImplantMask(view_pano_->GetSceneScale());
	pano_engine()->CheckCollideImplant(view_3d_->GetProjectionViewMatrix());
	pano_engine()->SetImplantAxisPointPanoPlane(implant_id);
	pano_engine()->SetImplantAxisPointPano3D(implant_id);
}

void ImplantViewMgr::slotTranslateImplantFromCross(int implant_id,
	const glm::vec3& vol_pos)
{
#if 1
#if 0
	pano_engine()->SetImplantPositionVolAndPano3D(implant_id, vol_pos);
#else
	pano_engine()->AppendImplantPosition(implant_id, vol_pos);
#endif
	pano_engine()->SetImplantAxisPointVol(implant_id);
	pano_engine()->CheckCollideImplant(view_3d_->GetProjectionViewMatrix());

	view_bd_->UpdateBoneDensity();
#else
	pano_engine()->SetImplantPositionVolAndPano3D(implant_id, vol_pos);
	pano_engine()->SetImplantPositionInPanoPlane(implant_id);

	pano_engine()->SetImplantAxisPointVol(implant_id);
	pano_engine()->ReconPanoramaImplantMask(view_pano_->GetSceneScale());
	pano_engine()->CheckCollideImplant(view_3d_->GetProjectionViewMatrix());
	pano_engine()->SetImplantAxisPointPanoPlane(implant_id);
	pano_engine()->SetImplantAxisPointPano3D(implant_id);

	view_bd_->UpdateBoneDensity();
#endif
}

void ImplantViewMgr::slotRotateImplantFromCross(int implant_id,
	const glm::vec3& axis,
	float delta_degree)
{
	int cross_id = -1;
	for (int i = 0; i < cross_section_count(); i++)
	{
		QObject* sender = QObject::sender();
		if (view_cross_section_[i].get() == sender)
		{
			cross_id = i;
			break;
		}
	}

	if (cross_id < 0)
	{
		common::Logger::instance()->Print(common::LogType::ERR, "ImplantViewMgr::slotRotateImplantFromCross : Not found (QObject::Sender).");
		return;
	}

	pano_engine()->RotateImplantInCross(implant_id, cross_id, delta_degree);
	pano_engine()->SetImplantAxisPointVol(implant_id);
	pano_engine()->CheckCollideImplant(view_3d_->GetProjectionViewMatrix());
	view_bd_->UpdateBoneDensity();
}

void ImplantViewMgr::slotUpdateAllImplantImagesFromCross(
	int implant_id, const glm::vec3& vol_pos)
{
#if 0
	pano_engine()->SetImplantPositionVolAndPano3D(implant_id, vol_pos);
#else
	pano_engine()->AppendImplantPosition(implant_id, vol_pos);
#endif
	pano_engine()->SetImplantAxisPointVolAndPano3D(implant_id);
	pano_engine()->SetImplantPositionInPanoPlane(implant_id, false);
	pano_engine()->SetImplantAxisPointPanoPlane(implant_id);

	pano_engine()->ReconPanoramaImplantMask(view_pano_->GetSceneScale());

	view_sagittal_->UpdateImplantHandleAndSpec();
	for (int i = 0; i < cross_section_count(); i++)
	{
		QObject* sender = QObject::sender();
		if (view_cross_section_[i].get() == sender)
		{
			continue;
		}
		view_cross_section_[i]->UpdateImplantHandleAndSpec();
		view_cross_section_[i]->UpdateCrossSection();
	}
	view_arch_->UpdateImplantHandleAndSpec();
	view_pano_->UpdateImplantHandleAndSpec();
	view_3d_->UpdateImplantSpec();

	view_bd_->SyncImplant3DCameraMatrix(view_3d_->GetRotateMatrix(),
		view_3d_->GetReorienMatrix(),
		view_3d_->GetViewMatrix());
	view_bd_->UpdateBoneDensity();
	view_3d_->UpdateVolume();
	view_pano_->UpdatedPano();
	view_arch_->UpdateSlice();

	view_sagittal_->SceneUpdate();
}

void ImplantViewMgr::slotSelectImplantFromCross(int implant_id, int cross_section_id)
{
	pano_engine()->SetImplantSelected(implant_id, true);

	MoveSagittalPlaneByImplant(implant_id);
	MoveCrossSectionByImplant(implant_id, cross_section_id);

	pano_engine()->ReconPanoramaImplantMask(view_pano_->GetSceneScale());
	view_pano_->UpdateImplantHandleAndSpec();
	for (int i = 0; i < cross_section_count(); i++)
		view_cross_section_[i]->UpdateImplantHandleAndSpec();
	view_arch_->UpdateImplantHandleAndSpec();
	view_3d_->UpdateImplantSpec();

	view_bd_->SyncImplant3DCameraMatrix(view_3d_->GetRotateMatrix(),
		view_3d_->GetReorienMatrix(),
		view_3d_->GetViewMatrix());
	view_bd_->UpdateBoneDensity();
	view_3d_->UpdateVolume();
	view_pano_->UpdatedPano();
	view_arch_->UpdateSlice();

	// implant panel 에 선택된 상태 sync
	emit sigImplantSelectionChanged(implant_id);
}

void ImplantViewMgr::slotSetAxialSliceFromPanoView(
	const QPointF& pt_pano_plane)
{
	BasePanoViewMgr::slotSetAxialSliceFromPanoView(pt_pano_plane);
	InitAxialLineInSagittalView();
}

void ImplantViewMgr::slotSetAxialSliceFromSagittalView(
	const glm::vec3& pt_vol)
{
	if (!pano_engine()->IsValidVolumePos(pt_vol)) return;

	this->SetAxialZposition(pt_vol);
}

void ImplantViewMgr::slotImplantHoveredFromSagittal(
	const QPointF& pt_sagittal_plane, int* hovered_id) const
{
	pano_engine()->HoveredImplantInSagittalPlane(pt_sagittal_plane, hovered_id);
}

void ImplantViewMgr::slotSetImplantPlanePos(int implant_id,
	QPointF& pt_sagittal_plane)
{
	const auto& res_implant_data = ResourceContainer::GetInstance()->GetImplantResource().data();
	if (res_implant_data.find(implant_id) == res_implant_data.end())
	{
		return;
	}

	glm::vec3 implant_pos_in_vol =
		res_implant_data.at(implant_id)->position_in_vol();
	pt_sagittal_plane = pano_engine()->MapVolToSagittalPlane(implant_pos_in_vol);
}

void ImplantViewMgr::slotSelectImplantFromSagittal(int implant_id)
{
	pano_engine()->SetImplantSelected(implant_id, true);

	MoveSagittalPlaneByImplant(implant_id);
	MoveCrossSectionByImplant(implant_id);

	pano_engine()->ReconPanoramaImplantMask(view_pano_->GetSceneScale());
	view_pano_->UpdateImplantHandleAndSpec();
	view_pano_->UpdatedPano();

	view_bd_->SyncImplant3DCameraMatrix(view_3d_->GetRotateMatrix(),
		view_3d_->GetReorienMatrix(),
		view_3d_->GetViewMatrix());
	view_bd_->UpdateBoneDensity();

	view_3d_->UpdateImplantSpec();
	view_3d_->UpdateVolume();

	for (int i = 0; i < cross_section_count(); i++)
		view_cross_section_[i]->UpdateImplantHandleAndSpec();
	view_arch_->UpdateImplantHandleAndSpec();
	view_arch_->UpdateSlice();

	// implant panel 에 선택된 상태 sync
	emit sigImplantSelectionChanged(implant_id);
}

void ImplantViewMgr::slotTranslateImplantFromSagittal(
	int implant_id, const QPointF& pt_sagittal_plane)
{
	glm::vec3 vol_pos = pano_engine()->MapSagittalPlaneToVol(pt_sagittal_plane);
	pano_engine()->SetImplantPositionVolAndPano3D(implant_id, vol_pos);
	pano_engine()->SetImplantAxisPointVol(implant_id);
	pano_engine()->CheckCollideImplant(view_3d_->GetProjectionViewMatrix());
	view_bd_->UpdateBoneDensity();
}

void ImplantViewMgr::slotRotateImplantFromSagittal(int implant_id,
	float delta_degree)
{
	pano_engine()->RotateImplantInSagittal(implant_id, delta_degree);
	pano_engine()->SetImplantAxisPointVol(implant_id);
	pano_engine()->CheckCollideImplant(view_3d_->GetProjectionViewMatrix());
	view_bd_->UpdateBoneDensity();
}

void ImplantViewMgr::slotUpdateAllImplantImagesFromSagittal(
	int implant_id, const QPointF& pt_sagittal_plane)
{
	glm::vec3 vol_pos = pano_engine()->MapSagittalPlaneToVol(pt_sagittal_plane);
	pano_engine()->SetImplantPositionVolAndPano3D(implant_id, vol_pos);
	pano_engine()->SetImplantAxisPointVolAndPano3D(implant_id);
	pano_engine()->SetImplantPositionInPanoPlane(implant_id, false);
	pano_engine()->SetImplantAxisPointPanoPlane(implant_id);

	MoveCrossSectionByImplant(implant_id);

	pano_engine()->ReconPanoramaImplantMask(view_pano_->GetSceneScale());
	view_sagittal_->UpdateImplantHandleAndSpec();
	view_pano_->UpdateImplantHandleAndSpec();
	for (int i = 0; i < cross_section_count(); i++)
		view_cross_section_[i]->UpdateImplantHandleAndSpec();
	view_arch_->UpdateImplantHandleAndSpec();
	view_3d_->UpdateImplantSpec();

	view_bd_->SyncImplant3DCameraMatrix(view_3d_->GetRotateMatrix(),
		view_3d_->GetReorienMatrix(),
		view_3d_->GetViewMatrix());
	view_bd_->UpdateBoneDensity();
	view_pano_->UpdatedPano();
	view_3d_->UpdateVolume();
	view_arch_->UpdateSlice();
}

void ImplantViewMgr::slotSagittalViewZoonDoneEvent()
{
	view_sagittal_->SceneUpdate();
}
/**=================================================================================================
Private functions
*===============================================================================================**/

void ImplantViewMgr::InitializeViews()
{
	view_arch_.reset(new ViewImplantArch());
	BasePanoViewMgr::set_base_view_pano_arch(
		std::static_pointer_cast<BaseViewPanoArch>(view_arch_));

	view_pano_.reset(new ViewImplantPano());
	BasePanoViewMgr::set_base_view_pano(
		std::static_pointer_cast<BaseViewPano>(view_pano_));

	for (int i = 0; i < common::kMaxCrossSection; i++)
	{
		view_cross_section_[i].reset(new ViewImplantCrossSection(i));

		BasePanoViewMgr::set_base_view_pano_cross_section(
			i, std::static_pointer_cast<BaseViewPanoCrossSection>(
				view_cross_section_[i]));
	}

	view_sagittal_.reset(new ViewImplantSagittal());
	BaseViewMgr::SetCastedView(view_sagittal_->view_id(),
		std::static_pointer_cast<View>(view_sagittal_));

	view_3d_.reset(new ViewImplant3D());
	BaseViewMgr::SetCastedView(view_3d_->view_id(),
		std::static_pointer_cast<View>(view_3d_));

	view_bd_.reset(new ViewBoneDensity());
	BaseViewMgr::SetCastedView(view_bd_->view_id(),
		std::static_pointer_cast<View>(view_bd_));

	SetVisibleViews(false);
}
void ImplantViewMgr::ChangedArchType(const ArchTypeID& type)
{
	if (type == pano_engine()->curr_arch_type()) return;

	pano_engine()->SetCurrentArchType(type);

	view_arch_->ForceRotateMatrix(pano_engine()->reorien_mat());
	view_arch_->SetCurrentArchType(type);

	if (pano_engine()->IsValidPanorama())
	{
		const auto& res_pano = ResourceContainer::GetInstance()->GetPanoResource();
		float auto_arch_slice_pos = res_pano.pano_ctrl_points().front().z;
		view_arch_->SetSliceInVol(auto_arch_slice_pos);

		InitArchLinePanoView();
	}

	if (!IsSetPanoArch())
	{
		slotRequestInitializeFromPanoArch();
	}
	else
	{
		slotArchUpdateFinishFromPanoArch();
	}

	if (pano_engine()->IsValidImplant())
	{
		const auto& res_implant =
			ResourceContainer::GetInstance()->GetImplantResource();

		int selected_id = res_implant.selected_implant_id();
		if (selected_id > 0)
		{
			MoveAllViewPlanesByImplant(selected_id);
		}
	}

	for (int i = 0; i < cross_section_count(); i++)
		view_cross_section_[i]->UpdateImplantHandleAndSpec();
	view_arch_->UpdateImplantHandleAndSpec();
	view_sagittal_->UpdateImplantHandleAndSpec();
	view_pano_->UpdateImplantHandleAndSpec();

	view_3d_->UpdateImplantSpec();
	view_3d_->ResetVolume();

	emit sigSyncArchType();
}

void ImplantViewMgr::UpdateSurfaceObjs()
{
	BasePanoViewMgr::UpdateSurfaceObjs();

	int selected_id = ResourceContainer::GetInstance()
		->GetImplantResource()
		.selected_implant_id();
	if (selected_id > 0)
		MoveAllViewPlanesByImplant(selected_id);
	else
	{
		view_sagittal_->UpdateImplantHandleAndSpec();
		view_sagittal_->SceneUpdate();
	}

	view_3d_->UpdateVolume();
	view_3d_->UpdateImplantSpec();

	view_bd_->SyncImplant3DCameraMatrix(view_3d_->GetRotateMatrix(),
		view_3d_->GetReorienMatrix(),
		view_3d_->GetViewMatrix());
	view_bd_->UpdateBoneDensity();
}
void ImplantViewMgr::InitSagittalResource()
{
	pano_engine()->InitSagittalResource();
	InitSagittalParams();
}

void ImplantViewMgr::InitAxialLineInSagittalView()
{
	if (pano_engine()->IsValidSagittal())
	{
		QPointF pt_pano_plane =
			pano_engine()->MapVolToPanoPlane(view_arch_->GetCenterPosition());
		view_sagittal_->SetAxialLine(pt_pano_plane);
	}
}

void ImplantViewMgr::InitSagittalParams()
{
	int width, height;
	float degree;
	GetSagittalParamsFromTools(&width, &height, &degree);
	pano_engine()->SetSagittalParams(width, height, degree);
}

void ImplantViewMgr::UpdateSagittal(const glm::vec3& vol_pt)
{
	pano_engine()->UpdateSagittal(vol_pt);
	view_sagittal_->SceneUpdate();
}

void ImplantViewMgr::SetSagittalLineInArchView(const glm::vec3& vol_pt,
	const QPointF& pano_pos)
{
	glm::vec3 pt_prev, pt_next;
	pano_engine()->GetPanoDirection(pano_pos, pt_prev, pt_next);
	view_arch_->SetSagittalLineFromVolPos(vol_pt, pt_prev, pt_next);
}

void ImplantViewMgr::GetSagittalParamsFromTools(int* width, int* height,
	float* degree)
{
	const auto& pano_roi = pano_engine()->GetPanoROI();
	const CW3Image3D& vol = ResourceContainer::GetInstance()->GetMainVolume();
	const float base_pixel_spacing =
		std::min(vol.pixelSpacing(), vol.sliceSpacing());
	*width = (int)(ViewImplantArch::kSzSagittalImage / base_pixel_spacing);
	*height = (int)(pano_roi.bottom - pano_roi.top);
	*degree = (float)(task_tool_.get()->RotateValue());
}

void ImplantViewMgr::MoveAllViewPlanesByImplant(int implant_id)
{
	// pano 앞/뒤 이동
	MovePanoramaPlaneByImplant(implant_id);

	// axial 이동 -> arch, cs, pano recon & scene update
	MoveArchPlaneByImplant(implant_id);

	// sagittal 위치 이동
	MoveSagittalPlaneByImplant(implant_id);

	// cross section 관련 뷰들 업데이트
	MoveCrossSectionByImplant(implant_id);
}

void ImplantViewMgr::MovePanoramaPlaneByImplant(const int& implant_id)
{
	const auto& res_implant_data = ResourceContainer::GetInstance()->GetImplantResource().data();
	if (res_implant_data.find(implant_id) == res_implant_data.end())
	{
		return;
	}

	const auto& res_pano = ResourceContainer::GetInstance()->GetPanoResource();
	const glm::vec3& pt_pano_plane_3d =
		res_implant_data.at(implant_id)->position_in_pano();
	const int implant_z_pos = static_cast<int>(pt_pano_plane_3d.z + 0.05f);
	const int origin_z_pos = res_pano.pano_3d_depth() / 2;
	int slice_num = implant_z_pos + origin_z_pos;

	if (slice_num != view_pano_->GetSliceNum())
		view_pano_->SetSliceNum(slice_num);
	else
	{
		pano_engine()->ReconPanoramaImplantMask(view_pano_->GetSceneScale());
		view_pano_->UpdatedPano();
	}

	view_pano_->UpdateImplantHandleAndSpec();
}
void ImplantViewMgr::MoveArchPlaneByImplant(const int& implant_id)
{
	const auto& res_implant_data = ResourceContainer::GetInstance()->GetImplantResource().data();
	if (res_implant_data.find(implant_id) == res_implant_data.end())
	{
		return;
	}

	const glm::vec3& implant_pos_in_vol =
		res_implant_data.at(implant_id)->position_in_vol();

	if (view_arch_->GetSliceInVol() != (int)implant_pos_in_vol.z)
		view_arch_->SetSliceInVol(implant_pos_in_vol.z);
	else
		view_arch_->UpdateSlice();

	for (int i = 0; i < cross_section_count(); i++)
		view_cross_section_[i]->UpdateImplantHandleAndSpec();
	view_arch_->UpdateImplantHandleAndSpec();
}
void ImplantViewMgr::MoveSagittalPlaneByImplant(const int& implant_id)
{
	if (implant_id == ResourceContainer::GetInstance()->GetImplantResource().add_implant_id())
	{
		return;
	}

	const auto& res_implant_data = ResourceContainer::GetInstance()->GetImplantResource().data();
	if (res_implant_data.find(implant_id) == res_implant_data.end())
	{
		return;
	}

	const auto& res_pano = ResourceContainer::GetInstance()->GetPanoResource();
	const glm::vec3& implant_pos_in_vol =
		res_implant_data.at(implant_id)->position_in_vol();
	const QPointF pano_pos(
		pano_engine()->MapVolToPanoPlane(implant_pos_in_vol).x(),
		static_cast<double>(res_pano.pano_3d_height()) / 2.0);
	UpdateSagittal(implant_pos_in_vol);
	SetSagittalLineInArchView(implant_pos_in_vol, pano_pos);
	view_sagittal_->UpdateImplantHandleAndSpec();
}
void ImplantViewMgr::MoveCrossSectionByImplant(const int& implant_id, const int& cross_section_id)
{
	if (implant_id == ResourceContainer::GetInstance()->GetImplantResource().add_implant_id() ||
		cross_section_count() < 1)
	{
		return;
	}

#if 0
	bool synchronize_angle_with_implant = GlobalPreferences::GetInstance()->preferences_.advanced.cross_section_view.synchronize_angle_with_implant;
	bool symmetry_by_implant = GlobalPreferences::GetInstance()->preferences_.advanced.cross_section_view.symmetry_by_implant;
#else // ini 변경 내용 실시간 적용
	GlobalPreferences* global_preferences = GlobalPreferences::GetInstance();
	//QSettings settings(global_preferences->ini_path(), QSettings::IniFormat);
	//settings.setIniCodec(QTextCodec::codecForName(global_preferences->ini_codec_name()));

	bool synchronize_angle_with_implant = global_preferences->GetSynchronizeAngleWithImplant();
	bool symmetry_by_implant = global_preferences->GetSymmetryByImplant();
#endif

	if (synchronize_angle_with_implant)
	{
		pano_engine()->SetCrossSectionAngle(implant_id);
		float implant_rotate_degree_in_pano_plane = pano_engine()->GetImplantRotateDegreeInPanoPlane(implant_id);
		task_tool_base_->SetCrossSectionAngle(implant_rotate_degree_in_pano_plane);
	}

	int id = (cross_section_id < 0) ?
		(symmetry_by_implant ?
		(cross_section_count() - 1) / 2
			: 0)
		: cross_section_id;

	pano_engine()->ShiftCrossSectionByImplant(implant_id, id);

	view_pano_->CrossSectionUpdated();
	view_arch_->CrossSectionUpdated();
	for (auto& cs_view : view_cross_section_)
		cs_view.second->UpdateCrossSection();
}

void ImplantViewMgr::UpdateSceneAllViews()
{
	view_arch_->UpdateSlice();
	view_sagittal_->SceneUpdate();
	view_pano_->UpdatedPano();
	view_3d_->UpdateVolume();
	for (auto& cs_view : view_cross_section_)
		cs_view.second->UpdateCrossSection();
	view_bd_->UpdateBoneDensity();
}

void ImplantViewMgr::ApplyPreferences()
{
	BasePanoViewMgr::ApplyPreferences();

	pano_engine()->CheckCollideImplant(view_3d_->GetProjectionViewMatrix());

	ReconAllResources();

	view_sagittal_.get()->ApplyPreferences();
	view_3d_.get()->ApplyPreferences();
	view_arch_.get()->ApplyPreferences();
	view_pano_.get()->ApplyPreferences();
	for (int i = 0; i < common::kMaxCrossSection; i++)
		view_cross_section_[i].get()->ApplyPreferences();

	UpdateSceneAllViews();
}

bool ImplantViewMgr::tmpIsRenderPano2D() const
{
	return view_pano_->tmpIsRender2D();
}

void ImplantViewMgr::MoveSagittalViewToSelectedMeasure(
	const common::measure::VisibilityParams& visibility_param)
{
	const auto& res_pano = ResourceContainer::GetInstance()->GetPanoResource();

	int slice_num = pano_engine()->GetZValuePanorama(visibility_param.center);
	if (slice_num != view_pano_->GetSliceNum())
		view_pano_->SetSliceNum(slice_num);

	const QPointF pano_pos(
		pano_engine()->MapVolToPanoPlane(visibility_param.center).x(),
		static_cast<double>(res_pano.pano_3d_height()) / 2.0);

	UpdateSagittal(visibility_param.center);
	SetSagittalLineInArchView(visibility_param.center, pano_pos);

	glm::vec3 cross =
		glm::cross(view_sagittal_->GetUpVector(), visibility_param.up);
	float sign = (cross.x + cross.y + cross.z > 0.0f) ? 1.0f : -1.0f;

	float th = acos(glm::clamp(
		glm::dot(view_sagittal_->GetUpVector(), visibility_param.up),
		-1.f, 1.f)) *
		sign * 180.0f / M_PI;
	task_tool_.get()->UpdateRotateAngle((double)th);

	view_sagittal_->UpdateImplantHandleAndSpec();
}

void ImplantViewMgr::Clip3DOnOff(const bool clip_on)
{
	view_3d_->Clip3DOnOff(clip_on);
}

void ImplantViewMgr::CheckCrossSectionMaximizeAlone()
{
	int index = 0;
	bool maximize_check = false;
	for (int i = 0; i < cross_section_count(); ++i)
	{
		index = i;
		if (view_cross_section_[i].get()->maximize())
		{
			maximize_check = true;
			break;
		}
	}

	if (maximize_check)
	{
		view_cross_section_[index]->blockSignals(false);
		view_cross_section_[index]->setVisible(true);
		for (int i = 0; i < cross_section_count(); ++i)
		{
			if (index == i)
			{
				continue;
			}
			view_cross_section_[i]->blockSignals(true);
			view_cross_section_[i]->setVisible(false);
		}
	}
}

#ifdef WILL3D_EUROPE
void ImplantViewMgr::SetSyncControlButtonOut()
{
	view_arch_.get()->SetSyncControlButton(false);
	view_pano_.get()->SetSyncControlButton(false);
	view_sagittal_.get()->SetSyncControlButton(false);
	view_3d_.get()->SetSyncControlButton(false);
}
#endif // WILL3D_EUROPE
