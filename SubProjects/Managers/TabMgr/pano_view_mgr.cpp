#include "pano_view_mgr.h"

#include <QTime>

#include <Engine/Common/Common/W3Logger.h>
#include <Engine/Common/Common/W3MessageBox.h>
#include <Engine/Common/Common/define_measure.h>
#include <Engine/Common/Common/define_pano.h>
#include <Engine/Common/Common/language_pack.h>

#include <Engine/Resource/ResContainer/resource_container.h>
#include <Engine/Resource/Resource/cross_section_resource.h>
#include <Engine/Resource/Resource/nerve_resource.h>
#include <Engine/Resource/Resource/pano_resource.h>
#ifndef WILL3D_VIEWER
#include <Engine/Core/W3ProjectIO/project_io_panorama.h>
#include <Engine/Core/W3ProjectIO/project_io_view.h>
#endif
#include <Engine/Module/Panorama/pano_engine.h>

#include <Engine/UIModule/UITools/pano_task_tool.h>

#include <Engine/UIModule/UIComponent/view_pano.h>
#include <Engine/UIModule/UIComponent/view_pano_arch.h>
#include <Engine/UIModule/UIComponent/view_pano_cross_section.h>
#include <Engine/UIModule/UIComponent/view_pano_orientation.h>

PanoViewMgr::PanoViewMgr(QObject* parent) : BasePanoViewMgr(parent) 
{
	InitializeViews();
	for (int id = 0; id < ReorientViewID::REORI_VIEW_END; ++id) 
	{
		ReorientViewID view_type = static_cast<ReorientViewID>(id);
		view_pano_orient_[id].reset(new ViewPanoOrientation(view_type));
		view_pano_orient_[id]->setVisible(false);

		BaseViewMgr::SetCastedView(view_pano_orient_[id]->view_id(), std::static_pointer_cast<View>(view_pano_orient_[id]));
	}

	connections();
}

PanoViewMgr::~PanoViewMgr() 
{

}

void PanoViewMgr::connections()
{
	BasePanoViewMgr::connections();
	for (int i = 0; i < common::kMaxCrossSection; ++i)
	{
		const auto& view = view_cross_section_[i].get();
		connect(view, &ViewPanoCrossSection::sigAddNerve, this, &PanoViewMgr::slotNerveAddFromCrossView);
		connect(view, &ViewPanoCrossSection::sigTranslatedNerve, this, &PanoViewMgr::slotNerveTranslateFromCrossView);
		connect(view, &ViewPanoCrossSection::sigEndEditNerve, this, &PanoViewMgr::slotNerveFinishFromCrossView);
		connect(view, &ViewPanoCrossSection::sigPressedKeyESC, this, &PanoViewMgr::slotPressedKeyESCfromCrossView);
		connect(view, &ViewPanoCrossSection::sigMaximize, this, &PanoViewMgr::slotMaximizeSingleCrossSectionView);
		connect(view, &ViewPanoCrossSection::sigCreateDCMFiles_ushort, this, &PanoViewMgr::sigCreateDCMFiles_ushort);
		connect(view, &ViewPanoCrossSection::sigCreateDCMFiles_uchar, this, &PanoViewMgr::sigCreateDCMFiles_uchar);
	}

	const auto& view_arch = view_pano_arch_.get();
	connect(view_arch, &ViewPanoArch::sigChangeEditArchMode, this, &PanoViewMgr::slotChangeEditArchMode);
	connect(view_arch, &ViewPanoArch::sigDeletedArch, this, &PanoViewMgr::slotArchDeleteFromPanoArch);
	connect(view_arch, &ViewPanoArch::sigChangedArchRange, this, &PanoViewMgr::slotChangedArchRangeFromPanoArch);

	const auto& view_pano = view_pano_.get();
	connect(view_pano, &ViewPano::sigAddedNerveEllipse, this, &PanoViewMgr::slotNerveAddFromPanoView);
	connect(view_pano, &ViewPano::sigTranslatedNerveEllipse, this, &PanoViewMgr::slotNerveTranslateFromPanoView);
	connect(view_pano, &ViewPano::sigEndEditedNerve, this, &PanoViewMgr::slotNerveFinishFromPanoView);
	connect(view_pano, &ViewPano::sigClearedNerve, this, &PanoViewMgr::slotNerveClear);
	connect(view_pano, &ViewPano::sigRemovedNerveEllipse, this, &PanoViewMgr::slotNerveRemovePoint);
	connect(view_pano, &ViewPano::sigCancelLastNerveEllipse, this, &PanoViewMgr::slotNerveCancelLastPoint);
	connect(view_pano, &ViewPano::sigInsertedNerveEllipse, this, &PanoViewMgr::slotNerveInsertPoint);
	connect(view_pano, &ViewPano::sigModifyNerveEllipse, this, &PanoViewMgr::slotNerveModifyPoint);

	for (int id = 0; id < ReorientViewID::REORI_VIEW_END; ++id)
	{
		const auto& view_orient = view_pano_orient_[id].get();
		connect(view_orient, SIGNAL(sigRotateMat(ReorientViewID, glm::mat4)), this, SLOT(slotOrientationViewRotate(ReorientViewID, glm::mat4)));
		connect(view_orient, &BaseViewOrientation::sigRenderQuality, this, &PanoViewMgr::slotOrientationViewRenderQuality);
		connect(view_orient, &BaseViewOrientation::sigUpdateDoneROI, this, &PanoViewMgr::slotUpdateDoneROIFromOrienView);
	}

	const auto& view_orient_r = view_pano_orient_[ReorientViewID::ORIENT_R].get();
	connect(view_orient_r, &BaseViewOrientation::sigChangedROI, this, &PanoViewMgr::slotChangeROIFromOrienView);
	connect(view_orient_r, &BaseViewOrientation::sigChangedSlice, this, &PanoViewMgr::slotChangeSliceFromOrienView);

#ifdef WILL3D_EUROPE
	connect(view_arch, &ViewPanoArch::sigShowButtonListDialog, this, &PanoViewMgr::sigShowButtonListDialog);
	connect(view_pano, &ViewPano::sigShowButtonListDialog, this, &PanoViewMgr::sigShowButtonListDialog);
	connect(view_arch, &ViewPanoArch::sigSyncControlButton, [=](bool is_on)
	{
		view_pano->SetSyncControlButton(is_on);
		for (int i = 0; i < common::kMaxCrossSection; ++i)
		{
			view_cross_section_[i].get()->SetSyncControlButton(is_on);
		}
	});
	connect(view_pano, &ViewPano::sigSyncControlButton, [=](bool is_on)
	{
		view_arch->SetSyncControlButton(is_on);
		for (int i = 0; i < common::kMaxCrossSection; ++i)
		{
			view_cross_section_[i].get()->SetSyncControlButton(is_on);
		}
	});
	for (int i = 0; i < common::kMaxCrossSection; ++i)
	{
		connect(view_cross_section_[i].get(), &ViewPanoCrossSection::sigShowButtonListDialog, this, &PanoViewMgr::sigShowButtonListDialog);
		connect(view_cross_section_[i].get(), &ViewPanoCrossSection::sigSyncControlButton, [=](bool is_on)
		{
			view_arch->SetSyncControlButton(is_on);
			view_pano->SetSyncControlButton(is_on);
		});
	}
#endif // WILL3D_EUROPE
}

void PanoViewMgr::ConnectPanoMenus() 
{
	BasePanoViewMgr::ConnectPanoMenus();
	connect(task_tool_.get(), &PanoTaskTool::sigPanoOrientRotate, this, &PanoViewMgr::slotOrientationFromTaskTool);
	connect(task_tool_.get(), &PanoTaskTool::sigPanoNerveParamsChanged, this, &PanoViewMgr::slotNerveChangedValuesFromTool);
	connect(task_tool_.get(), &PanoTaskTool::sigPanoNerveRecordHovered, this, &PanoViewMgr::slotNerveHoveredFromTool);
	connect(task_tool_.get(), &PanoTaskTool::sigPanoNerveToggledDraw, this, &PanoViewMgr::slotNerveDrawModeFromTool);
	connect(task_tool_.get(), &PanoTaskTool::sigPanoNerveDelete, this, &PanoViewMgr::slotDeleteNerveFromTool);
	connect(task_tool_.get(), &PanoTaskTool::sigPanoArchTypeChanged, this, &PanoViewMgr::slotArchTypeChanged);
}

void PanoViewMgr::DisconnectPanoMenus()
{
	BasePanoViewMgr::DisconnectPanoMenus();
	disconnect(task_tool_.get(), &PanoTaskTool::sigPanoOrientRotate, this, &PanoViewMgr::slotOrientationFromTaskTool);
	disconnect(task_tool_.get(), &PanoTaskTool::sigPanoNerveParamsChanged, this, &PanoViewMgr::slotNerveChangedValuesFromTool);
	disconnect(task_tool_.get(), &PanoTaskTool::sigPanoNerveRecordHovered, this, &PanoViewMgr::slotNerveHoveredFromTool);
	disconnect(task_tool_.get(), &PanoTaskTool::sigPanoNerveToggledDraw, this, &PanoViewMgr::slotNerveDrawModeFromTool);
	disconnect(task_tool_.get(), &PanoTaskTool::sigPanoNerveDelete, this, &PanoViewMgr::slotDeleteNerveFromTool);
	disconnect(task_tool_.get(), &PanoTaskTool::sigPanoArchTypeChanged, this, &PanoViewMgr::slotArchTypeChanged);
}

/**=================================================================================================
Public functions
 *===============================================================================================**/
#ifndef WILL3D_VIEWER
void PanoViewMgr::exportProject(ProjectIOPanorama& out)
{
	float scene_scale = 0.0f, scene_to_gl = 0.0f;
	QPointF trans_gl;

	out.InitPanoTab();

	out.SaveCSAngle(task_tool_base_->GetCrossSectionAngle());
	out.SaveCSInterval(task_tool_base_->GetCrossSectionInterval());
	out.SaveCSThickness(task_tool_base_->GetCrossSectionThickness());
	out.SaveArchRange(task_tool_base_->GetArchRange());
	out.SaveArchThickness(task_tool_base_->GetArchThickness());

	view_pano_arch_->exportProject(&scene_scale, &scene_to_gl, &trans_gl);
	out.InitializeView(ProjectIOPanorama::ViewType::PANO_ARCH);
	out.GetViewIO().SaveViewInfo(scene_scale, scene_to_gl, trans_gl.x(), trans_gl.y());

	view_pano_->exportProject(&scene_scale, &scene_to_gl, &trans_gl);
	out.InitializeView(ProjectIOPanorama::ViewType::PANO_PANO);
	out.GetViewIO().SaveViewInfo(scene_scale, scene_to_gl, trans_gl.x(), trans_gl.y());
	view_pano_->ExportProjectForMeasure3D(out.GetViewIO());

	for (auto& view : view_cross_section_)
	{
		view.second->exportProject(&scene_scale, &scene_to_gl, &trans_gl);
		out.InitializeView(ProjectIOPanorama::ViewType::PANO_CS, view.first);
		out.GetViewIO().SaveViewInfo(scene_scale, scene_to_gl, trans_gl.x(), trans_gl.y());
	}

	ArchTypeID curr_arch_type = pano_engine()->curr_arch_type();
	out.SaveOrientDegrees(curr_arch_type,
		task_tool_->GetOrientDegree(ReorientViewID::ORIENT_R),
		task_tool_->GetOrientDegree(ReorientViewID::ORIENT_I),
		task_tool_->GetOrientDegree(ReorientViewID::ORIENT_A));

	ArchTypeID prev_arch_type = (ArchTypeID)!curr_arch_type;
	out.SaveOrientDegrees(prev_arch_type, save_orien_degrees_.value_r,
		save_orien_degrees_.value_i,
		save_orien_degrees_.value_a);
}

void PanoViewMgr::importProject(ProjectIOPanorama& in_pano,
	const bool& is_counterpart_exists) {
	float scene_scale = 0.0f, scene_to_gl = 0.0f;
	float trans_gl_x = 0.0f, trans_gl_y = 0.0f;

	for (int id = 0; id < ReorientViewID::REORI_VIEW_END; ++id)
		view_pano_orient_[id]->blockSignals(true);

	task_tool_->blockSignals(true);

	float degree = 0.f;
	float interval = 0.f;
	float cs_thickness = 0.f;
	float arch_range = 0.f;
	float arch_thickness = 0.f;

	in_pano.LoadCSAngle(degree);
	
	//project 파일에서 아래 값이 없는 경우가 있어서, 값이 없을 때는 기본값을 읽어오도록 수정.
	if (!in_pano.LoadCSInterval(interval))
	{
		interval = task_tool_base_->GetCrossSectionInterval();
	}
	if (!in_pano.LoadCSThickness(cs_thickness))
	{
		cs_thickness = task_tool_base_->GetCrossSectionThickness();
	}
	if (!in_pano.LoadArchRange(arch_range))
	{
		arch_range = task_tool_base_->GetArchRange();
	}
	if (!in_pano.LoadArchThickness(arch_thickness))
	{		
		arch_thickness = task_tool_base_->GetArchThickness();
	}

	task_tool_base_->SetCrossSectionAngle(degree);
	task_tool_base_->SetCrossSectionInterval(interval);
	task_tool_base_->SetCrossSectionThickness(cs_thickness);
	task_tool_base_->SetArchRange(arch_range);
	task_tool_base_->SetArchThickness(arch_thickness);

	in_pano.InitializeView(ProjectIOPanorama::ViewType::PANO_ARCH);
	in_pano.GetViewIO().LoadViewInfo(scene_scale, scene_to_gl, trans_gl_x, trans_gl_y);
	view_pano_arch_->importProject(scene_scale, scene_to_gl, QPointF(trans_gl_x, trans_gl_y), is_counterpart_exists);

	in_pano.InitializeView(ProjectIOPanorama::ViewType::PANO_PANO);
	in_pano.GetViewIO().LoadViewInfo(scene_scale, scene_to_gl, trans_gl_x, trans_gl_y);
	view_pano_->importProject(scene_scale, scene_to_gl, QPointF(trans_gl_x, trans_gl_y), is_counterpart_exists);
	view_pano_->ImportProjectForMeasure3D(in_pano.GetViewIO());

	int cross_cnt = cross_section_count();
	for (int i = 0; i < cross_cnt; ++i)
	{
		in_pano.InitializeView(ProjectIOPanorama::ViewType::PANO_CS, i);
		in_pano.GetViewIO().LoadViewInfo(scene_scale, scene_to_gl, trans_gl_x, trans_gl_y);
		bool is_update_resource = (i == cross_cnt - 1) ? true : false;
		view_cross_section_[i]->importProject(
			scene_scale, scene_to_gl, QPointF(trans_gl_x, trans_gl_y),
			is_counterpart_exists, is_update_resource);
	}

	ArchTypeID curr_arch_type = pano_engine()->curr_arch_type();

	glm::mat4 reorien_mat = pano_engine()->reorien_mat();
	for (int id = 0; id < ReorientViewID::REORI_VIEW_END; ++id)
		view_pano_orient_[id]->SetRotateMatrix(reorien_mat);

	int orient_r, orient_i, orient_a;
	in_pano.LoadOrientDegrees(curr_arch_type, orient_r, orient_i, orient_a);
	task_tool_->SetOrientDegrees(orient_a, orient_r, orient_i);

	view_pano_orient_[ReorientViewID::ORIENT_A]->SetAngleDegree(orient_a);
	view_pano_orient_[ReorientViewID::ORIENT_I]->SetAngleDegree(orient_i);
	view_pano_orient_[ReorientViewID::ORIENT_R]->SetAngleDegree(orient_r);

	ArchTypeID prev_arch_type = (ArchTypeID)!curr_arch_type;
	in_pano.LoadOrientDegrees(prev_arch_type, save_orien_degrees_.value_r,
		save_orien_degrees_.value_i,
		save_orien_degrees_.value_a);

	task_tool_->SetPanoArchTypeRadioUI(pano_engine()->curr_arch_type());

	task_tool_->blockSignals(false);

	for (int id = 0; id < ReorientViewID::REORI_VIEW_END; ++id)
		view_pano_orient_[id]->blockSignals(false);
}
#endif
void PanoViewMgr::SetPanoMenu(const std::shared_ptr<PanoTaskTool>& task_menu) {
	task_tool_ = task_menu;
	BasePanoViewMgr::set_task_tool_base(
		std::static_pointer_cast<BasePanoTaskTool>(task_tool_));
}

void PanoViewMgr::SyncPanoResource() {
	BasePanoViewMgr::SyncPanoResource();

	const auto& roi = pano_engine()->GetPanoROI();
	view_pano_orient_[ReorientViewID::ORIENT_R]->ReadyROILines(roi.top, roi.slice,
		roi.bottom);

	task_tool_->SetPanoArchTypeRadioUI(pano_engine()->curr_arch_type());
}
void PanoViewMgr::ResetOrientationAndResetArch(
	const bool& is_redraw_auto_arch) {
	ResetOrientationParams();

	view_pano_->ClearUnfinishedNerve();
	if (is_redraw_auto_arch) {
		GeneratePanorama();
	}
}

void PanoViewMgr::GridOnOffOrientation(const bool& on) {
	for (int id = 0; id < ReorientViewID::REORI_VIEW_END; ++id)
		view_pano_orient_[id]->SetGridOnOff(on);
}

void PanoViewMgr::InitializeViews() {
	view_pano_arch_.reset(new ViewPanoArch());
	BasePanoViewMgr::set_base_view_pano_arch(
		std::static_pointer_cast<BaseViewPanoArch>(view_pano_arch_));
	view_pano_arch_->setVisible(false);

	view_pano_.reset(new ViewPano());
	BasePanoViewMgr::set_base_view_pano(
		std::static_pointer_cast<BaseViewPano>(view_pano_));
	view_pano_->setVisible(false);

	for (int i = 0; i < common::kMaxCrossSection; i++) {
		view_cross_section_[i].reset(new ViewPanoCrossSection(i));
		view_cross_section_[i]->setVisible(false);

		BasePanoViewMgr::set_base_view_pano_cross_section(
			i, std::static_pointer_cast<BaseViewPanoCrossSection>(
				view_cross_section_[i]));
	}
}

void PanoViewMgr::InitCrossSectionResource() {
	if (view_pano_arch_->is_edit_mode()) {
		pano_engine()->ClearCrossSectionResource();
		return;
	}

	BasePanoViewMgr::InitCrossSectionResource();
}

void PanoViewMgr::InitPanoramaROI() {
	BasePanoViewMgr::InitPanoramaROI();

	const auto& roi = pano_engine()->GetPanoROI();
	view_pano_orient_[ReorientViewID::ORIENT_R]->ReadyROILines(roi.top, roi.slice,
		roi.bottom);
	view_pano_orient_[ReorientViewID::ORIENT_R]->InitROILines();
}

void PanoViewMgr::ChangedArchType(const ArchTypeID& type) {
#if 0
	if (type == pano_engine()->curr_arch_type())
	{
		return;
	}
#endif

	pano_engine()->SetCurrentArchType(type);
	glm::mat4 reorien_mat = pano_engine()->reorien_mat();

	SaveOrienDegrees tmp_orien_degrees;

	for (int id = 0; id < ReorientViewID::REORI_VIEW_END; ++id)
		disconnect(view_pano_orient_[id].get(),
			SIGNAL(sigRotateMat(ReorientViewID, glm::mat4)), this,
			SLOT(slotOrientationViewRotate(ReorientViewID, glm::mat4)));

	tmp_orien_degrees.value_a =
		task_tool_->GetOrientDegree(ReorientViewID::ORIENT_A);
	tmp_orien_degrees.value_r =
		task_tool_->GetOrientDegree(ReorientViewID::ORIENT_R);
	tmp_orien_degrees.value_i =
		task_tool_->GetOrientDegree(ReorientViewID::ORIENT_I);

	task_tool_->SetOrientDegrees(save_orien_degrees_.value_a,
		save_orien_degrees_.value_r,
		save_orien_degrees_.value_i);

	for (int id = 0; id < ReorientViewID::REORI_VIEW_END; ++id) {
		view_pano_orient_[id]->SetRotateMatrix(reorien_mat);
		view_pano_orient_[id]->SetAngleDegree(save_orien_degrees_.value_a);
		view_pano_orient_[id]->SetRenderQuality();
	}

	for (int id = 0; id < ReorientViewID::REORI_VIEW_END; ++id)
		connect(view_pano_orient_[id].get(),
			SIGNAL(sigRotateMat(ReorientViewID, glm::mat4)), this,
			SLOT(slotOrientationViewRotate(ReorientViewID, glm::mat4)));

#if 0
	if (pano_engine()->IsValidPanorama()) {
		const auto& res_pano = ResourceContainer::GetInstance()->GetPanoResource();
		float auto_arch_slice_pos = res_pano.pano_ctrl_points().front().z;
		view_pano_arch_->SetArchSliceNumber(type);
	}
#endif

	pano_engine()->SetReorienMat(reorien_mat);
	view_pano_arch_->ForceRotateMatrix(reorien_mat);
	save_orien_degrees_ = tmp_orien_degrees;

	view_pano_arch_->SetCurrentArchType(type);

	if (IsSetPanoArch()) {
		view_pano_arch_->SetArchSliceNumber(type);
		GeneratePanorama();
	}
	else {
		slotRequestInitializeFromPanoArch();
	}

	view_pano_->UpdateImplantHandleAndSpec();

	task_tool_->SetPanoArchTypeRadioUI(pano_engine()->curr_arch_type());
	}

void PanoViewMgr::UpdateSurfaceObjs() {
	BasePanoViewMgr::UpdateSurfaceObjs();
	UpdatePanoViewNerveCtrlPoints();

	const auto& res_nerve = ResourceContainer::GetInstance()->GetNerveResource();
	const auto& nerve_data = res_nerve.GetNerveDataInPano();
	for (const auto& elem : nerve_data)
		view_pano_->SetVisibleNerve(elem.first, elem.second->is_visible());
}

void PanoViewMgr::ResetOrientationParams() {
	task_tool_->ResetOrientDegreesUI();

	for (int id = 0; id < ReorientViewID::REORI_VIEW_END; ++id)
		view_pano_orient_[id]->SetOrientationAngle(0);

#if 1
	for (int id = 0; id < ReorientViewID::REORI_VIEW_END; ++id)
		view_pano_orient_[id]->SetRotateMatrix(glm::mat4(1.0f));
#endif

	for (int id = 0; id < ReorientViewID::REORI_VIEW_END; ++id)
		view_pano_orient_[id]->SetRenderQuality();

	view_pano_arch_->ForceRotateMatrix(glm::mat4(1.0f));
	pano_engine()->SetReorienMat(glm::mat4(1.0f));
}

void PanoViewMgr::MoveViewsToSelectedMeasure(
	const common::ViewTypeID& view_type, const unsigned int& measure_id) {
	common::measure::VisibilityParams visibility_params;
	GetMeasureParamsInView(view_type, measure_id, &visibility_params);

	switch (view_type) {
	case common::ViewTypeID::PANO_ARCH:
		MoveArchViewToSelectedMeasure(visibility_params);
		break;
	case common::ViewTypeID::PANO:
		MovePanoViewToSelectedMeasure(visibility_params);
		break;
	case common::ViewTypeID::CROSS_SECTION:
		MoveCrossSectionViewToSelectedMeasure(visibility_params);
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
void PanoViewMgr::slotArchShifteFromPanoArch(float shifted_value_in_vol) {
	BasePanoViewMgr::slotArchShifteFromPanoArch(shifted_value_in_vol);
	UpdatePanoViewNerveCtrlPoints();
}

void PanoViewMgr::VisibleNerve(bool is_visible) {
	BasePanoViewMgr::VisibleNerve(is_visible);

	if (!view_pano_->isVisible()) return;

	view_pano_->SetVisibleNerveAll(is_visible);
	task_tool_->NerveToolVisibilityChange(is_visible);
}

void PanoViewMgr::slotArchUpdateFinishFromPanoArch() {
	task_tool_->TaskManualDeactivate();
	BasePanoViewMgr::slotArchUpdateFinishFromPanoArch();
}

void PanoViewMgr::slotCrossSectionToolsChanged() {
	BasePanoViewMgr::slotCrossSectionToolsChanged();
	pano_engine()->SetCrossSectionNervePoint();

	for (int i = 0; i < cross_section_count(); i++)
		view_cross_section_[i]->UpdateModifyNerveEll();
}

void PanoViewMgr::slotArchDeleteFromPanoArch() {
	task_tool_->NerveToolDeactivate();

	view_pano_->SetEditNerveMode(false);
	for (int i = 0; i < common::kMaxCrossSection; i++)
		view_cross_section_[i]->SetEditNerveMode(false);

	pano_engine()->ClearCrossSectionResource();

	BasePanoViewMgr::InitPanoResource();
	BasePanoViewMgr::ReconPanoResource();

	view_pano_->UpdatedPano();
	BasePanoViewMgr::UpdateCrossSection();
	BasePanoViewMgr::UpdateAxialLine();

	DeleteMeasuresInPanorama();
}

void PanoViewMgr::slotTranslateZfromPanoArch(float z_pos_vol) {
	BasePanoViewMgr::slotTranslateZfromPanoArch(z_pos_vol);

	view_pano_orient_[ReorientViewID::ORIENT_R]->SetSliceInVol(z_pos_vol);
}

void PanoViewMgr::slotArchTypeChanged(const ArchTypeID& arch_type) {
	if (arch_type == pano_engine()->curr_arch_type())
	{
		return;
	}

	ChangedArchType(arch_type);
}

void PanoViewMgr::slotChangeEditArchMode() {
	if (view_pano_arch_->is_edit_mode()) {
		const auto& res_pano = ResourceContainer::GetInstance()->GetPanoResource();
		view_pano_->SetSliceNum(res_pano.pano_3d_depth() / 2);
	}

	BasePanoViewMgr::InitCrossSectionResource();
	BasePanoViewMgr::UpdateCrossSection();
	InitAxialLineCrossView();
}
void PanoViewMgr::TaskManualArch() { view_pano_arch_->SetManualArchMode(); }

void PanoViewMgr::slotNerveAddFromPanoView(int nerve_id,
	const QPointF& pt_in_pano_plane) {
	glm::vec3 point_in_vol = pano_engine()->MapPanoPlaneToVol(pt_in_pano_plane);

	pano_engine()->AddNerveCtrlPoint(nerve_id, point_in_vol);

	for (int i = 0; i < cross_section_count(); i++)
		view_cross_section_[i]->UpdateModifyNerveEll();

	pano_engine()->EditCrossSectionShiftedValue(point_in_vol);
	BasePanoViewMgr::UpdateCrossSection();
}

void PanoViewMgr::slotNerveAddFromCrossView(int cross_id,
	const QPointF& pt_in_cross_plane) {
	glm::vec3 point_in_vol =
		pano_engine()->MapCrossPlaneToVol(cross_id, pt_in_cross_plane);

	pano_engine()->AddNerveCtrlPoint(view_pano_->GetCurrentNerveID(),
		point_in_vol);

	UpdatePanoViewNerveCtrlPoints();

	for (int i = 0; i < cross_section_count(); i++)
		view_cross_section_[i]->UpdateModifyNerveEll();
}

void PanoViewMgr::slotNerveFinishFromCrossView() {
	view_pano_->EndEditNerveFromCromssSection();
}

void PanoViewMgr::slotPressedKeyESCfromCrossView() {
	view_pano_->PressedKeyESC();
}

void PanoViewMgr::slotNerveTranslateFromPanoView(
	int nerve_id, int nerve_selected_index, const QPointF& pt_in_pano_plane) {
	glm::vec3 point_in_vol = pano_engine()->MapPanoPlaneToVol(pt_in_pano_plane);

	pano_engine()->EditNerveCtrlPoint(nerve_id, nerve_selected_index,
		point_in_vol);

	UpdateSurfaceObjs();
}
void PanoViewMgr::slotNerveTranslateFromCrossView(
	int cross_id, const QPointF& pt_in_cross_plane) {
	glm::vec3 point_in_vol =
		pano_engine()->MapCrossPlaneToVol(cross_id, pt_in_cross_plane);

	pano_engine()->EditNerveCtrlModifiedPoint(point_in_vol);

	UpdatePanoViewNerveCtrlPoints();
	UpdateSurfaceObjs();
}
void PanoViewMgr::slotNerveFinishFromPanoView(int nerve_id) {
	task_tool_->NerveCreated(nerve_id);

	UpdateSurfaceObjs();

	task_tool_->SyncVisibilityResources();

}
void PanoViewMgr::slotNerveClear(int nerve_id) {
	task_tool_->NerveDeleted(nerve_id);
	pano_engine()->DeleteNerve(nerve_id);

	UpdateSurfaceObjs();

	task_tool_->SyncVisibilityResources();

}

void PanoViewMgr::slotNerveCancelLastPoint(int nerve_id,
	int nerve_removed_index) {
	pano_engine()->CancleNerveCtrlPoint(nerve_id, nerve_removed_index);

	for (int i = 0; i < cross_section_count(); i++) {
		view_cross_section_[i]->UpdateModifyNerveEll();
	}
}
void PanoViewMgr::slotNerveRemovePoint(int nerve_id, int nerve_removed_index) {
	pano_engine()->RemoveNerveCtrlPoint(nerve_id, nerve_removed_index);
	UpdateSurfaceObjs();
}

void PanoViewMgr::slotNerveInsertPoint(int nerve_id, int nerve_inserted_index,
	const QPointF& pt_in_pano_plane) {
	glm::vec3 point_in_vol;
	point_in_vol = pano_engine()->MapPanoPlaneToVol(pt_in_pano_plane);
	pano_engine()->InsertNerveCtrlPoint(nerve_id, nerve_inserted_index,
		point_in_vol);
	UpdateSurfaceObjs();
}

void PanoViewMgr::slotNerveModifyPoint(int nerve_id, int nerve_selected_index,
	bool is_modify) {
	pano_engine()->SetModifyNervePoint(nerve_id, nerve_selected_index, is_modify);

	BasePanoViewMgr::UpdateCrossSection();
}

void PanoViewMgr::slotNerveChangedValuesFromTool(const bool& nerve_visible,
	const int& nerve_id,
	const float& diameter,
	const QColor& color) {
	auto res_container = ResourceContainer::GetInstance();
	const CW3Image3D& vol = (res_container->GetMainVolume());
	float nerve_rad_mm = static_cast<float>(diameter) * 0.5f;
	float base_pixel_spacing_ = std::min(vol.pixelSpacing(), vol.sliceSpacing());
	float nerve_radius = nerve_rad_mm / base_pixel_spacing_;

	pano_engine()->SetNerveParams(nerve_visible, nerve_id, nerve_radius, color);

	view_pano_->SetVisibleNerve(nerve_id, nerve_visible);
	for (int i = 0; i < cross_section_count(); i++) {
		view_cross_section_[i]->UpdateModifyNerveEll();
	}
	UpdateSurfaceObjs();
}
void PanoViewMgr::slotNerveDrawModeFromTool(bool is_draw_mode) {
	if (!pano_engine()->IsValidPanorama() && is_draw_mode) {
		task_tool_->NerveToolDeactivate();
		CW3MessageBox msg_box(QString("Will3D"), lang::LanguagePack::msg_75());
		msg_box.exec();
		return;
	}

	if (view_pano_arch_->is_edit_mode()) {
		bool res = view_pano_arch_->SetApplyArch();
		if (!res) return;
	}

	// common task tool 의 CancelSelectedTool() 를 호출하기 위함
	emit sigNerveDrawModeOn();

	if (is_draw_mode)
	{
		float previous_arch_thickness_value = task_tool_base_->GetArchThickness();
		task_tool_base_->SetPreviousArchThickness(previous_arch_thickness_value);
		task_tool_base_->SetArchThickness(0.0f);		
	} else
	{
		task_tool_base_->SetArchThickness(task_tool_base_->GetPreviousArchThickness());
		task_tool_base_->SetPreviousArchThickness(0.0f);
	}

	view_pano_->SetEditNerveMode(is_draw_mode);
	for (int i = 0; i < common::kMaxCrossSection; i++)
		view_cross_section_[i]->SetEditNerveMode(is_draw_mode);
}

void PanoViewMgr::slotNerveHoveredFromTool(int nerve_id, bool is_hovered) {
	view_pano_->SetNerveHover(nerve_id, is_hovered);
}

void PanoViewMgr::slotDeleteNerveFromTool(int nerve_id) {
	view_pano_->ClearNerve(nerve_id);
	task_tool_->SyncVisibilityResources();

}

void PanoViewMgr::slotOrientationViewRotate(const ReorientViewID& view_type,
	const glm::mat4& orien) {
	for (int curr_id = 0; curr_id < ReorientViewID::REORI_VIEW_END; ++curr_id) {
		if (curr_id == view_type) {
			int degree_view = view_pano_orient_[view_type]->GetAngleDegree();
			task_tool_->SyncOrientDegreeUIOnly(view_type, degree_view);
		}
		else {
			view_pano_orient_[curr_id]->SetRotateMatrix(orien);
		}
	}

	pano_engine()->SetReorienMat(orien);
	view_pano_arch_->ForceRotateMatrix(orien);
	view_pano_arch_->SetCommonToolOnce(common::CommonToolTypeOnce::M_DEL_ALL,
		true);
}

void PanoViewMgr::slotOrientationViewRenderQuality(
	const ReorientViewID& view_type) {
	for (int id = 0; id < ReorientViewID::REORI_VIEW_END; ++id) {
		if (id == view_type) continue;
		view_pano_orient_[id]->SetRenderQuality();
	}
}

void PanoViewMgr::slotOrientationFromTaskTool(const ReorientViewID& view_type,
	int angle) {
	view_pano_orient_[view_type]->SetOrientationAngle(angle);
	UpdatePanorama();
}

void PanoViewMgr::slotChangeROIFromOrienView() {
	float top = view_pano_orient_[ReorientViewID::ORIENT_R]->GetTopInVol();
	float bottom = view_pano_orient_[ReorientViewID::ORIENT_R]->GetBottomInVol();

	pano_engine()->SetPanoROITop(top);
	pano_engine()->SetPanoROIBottom(bottom);

	view_pano_arch_->SetSliceRange(top, bottom);

	UpdatePanorama();
}
void PanoViewMgr::slotUpdateDoneROIFromOrienView() {
	float top = view_pano_orient_[ReorientViewID::ORIENT_R]->GetTopInVol();
	float bottom = view_pano_orient_[ReorientViewID::ORIENT_R]->GetBottomInVol();

	pano_engine()->SetPanoROITop(top);
	pano_engine()->SetPanoROIBottom(bottom);

	view_pano_arch_->SetSliceRange(top, bottom);

	UpdatePanorama();

	DeleteMeasuresInPanorama();
}

void PanoViewMgr::slotChangeSliceFromOrienView() {
	float slice = view_pano_orient_[ReorientViewID::ORIENT_R]->GetSliceInVol();
	pano_engine()->SetPanoROISlice(slice);
	view_pano_arch_->SetSliceInVol(slice);
}

/**=================================================================================================
Private functions
*===============================================================================================**/

void PanoViewMgr::UpdatePanorama() {
	if (!IsSetPanoArch() || view_pano_->recon_type() != BaseViewPano::RECON_MPR)
		return;

	BasePanoViewMgr::InitPanoResource();

	pano_engine()->UpdateImplantPositions();
	pano_engine()->UpdateNerveResource();

	BasePanoViewMgr::InitCrossSectionResource();

	BasePanoViewMgr::ReconPanoResource();
	BasePanoViewMgr::ReconNerveResource();
	BasePanoViewMgr::ReconImplantResource();

	view_pano_->UpdatedPano();
	UpdateCrossSection();
	UpdateAxialLine();
	InitArchLinePanoView();
}

void PanoViewMgr::UpdatePanoViewNerveCtrlPoints() {
	std::map<int, std::vector<QPointF>> ctrl_nerve_in_pano_plane;
	pano_engine()->GetNerveCtrlPointsInPanoPlane(&ctrl_nerve_in_pano_plane);

	view_pano_->UpdateNerveCtrlPoints(ctrl_nerve_in_pano_plane);
}

void PanoViewMgr::ApplyPreferences() {
	BasePanoViewMgr::ApplyPreferences();

	view_pano_arch_->ApplyPreferences();
	view_pano_->ApplyPreferences();
	for (int i = 0; i < common::kMaxCrossSection; i++)
		view_cross_section_[i]->ApplyPreferences();

	for (int id = 0; id < ReorientViewID::REORI_VIEW_END; ++id)
		view_pano_orient_[id]->ApplyPreferences();

	//UpdatePanorama();
}

void PanoViewMgr::SetCurrentArchType(const ArchTypeID& arch_type)
{
	if (!view_pano_arch_)
	{
		return;
	}
	view_pano_arch_->SetCurrentArchType(arch_type);
}

void PanoViewMgr::UpdateArchFromMPR(ArchTypeID arch_type, const std::vector<glm::vec3>& points, const glm::mat4& orientation_matrix, const int slice_number)
{
	view_pano_arch_->SetCurrentArchType(arch_type);
	view_pano_arch_->SetSliceInVol(slice_number);
	view_pano_arch_->ForceRotateMatrix(orientation_matrix);

#if 1
	for (int id = 0; id < ReorientViewID::REORI_VIEW_END; ++id)
	{
		view_pano_orient_[id]->SetRotateMatrix(orientation_matrix);
	}

#if 0
	task_tool_->SetPanoArchTypeRadioUI(arch_type);
	pano_engine()->SetCurrentArchType(arch_type);
	view_pano_arch_->SetCurrentArchType(arch_type);
	view_pano_arch_->ForceRotateMatrix(orientation_matrix);
	pano_engine()->SetReorienMat(orientation_matrix, arch_type);
#else
	pano_engine()->SetReorienMat(orientation_matrix, arch_type);
	pano_engine()->SetPanoROISlice(slice_number);

	view_pano_arch_->SetArchPointsInVol(points, arch_type);

#if 1
	ChangedArchType(arch_type);
#else
	view_pano_->UpdateImplantHandleAndSpec();
	task_tool_->SetPanoArchTypeRadioUI(pano_engine()->curr_arch_type());
#endif
#endif

#else
	pano_engine()->SetCurrentArchType(arch_type);
	pano_engine()->SetReorienMat(orientation_matrix);
	ChangedArchType(arch_type);
#endif
}

void PanoViewMgr::ClearArch(ArchTypeID arch_type)
{
	if (!view_pano_arch_)
	{
		return;
}
	view_pano_arch_->ClearArch(arch_type);
}

void PanoViewMgr::CheckCrossSectionMaximizeAlone()
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
		view_cross_section_[index]->setVisible(true);
		for (int i = 0; i < cross_section_count(); ++i)
		{
			if (index == i)
			{
				continue;
			}
			view_cross_section_[i]->setVisible(false);
		}
	}
}

#ifndef WILL3D_VIEWER
const int PanoViewMgr::GetCrossSectionFilterLevel()
{
	return view_cross_section_[0]->GetCrossSectionFilterLevel();
}

void PanoViewMgr::RequestedCreateCrossSectionDCMFiles(bool nerve_visible, bool implant_visible, int filter, int thickness)
{
	if (view_cross_section_.empty())
	{
		return;
	}

	QTime time = QTime::currentTime();
	QString str = QString::number(time.msecsSinceStartOfDay()) + "_";

	int cnt = cross_section_count();
	for (int i = 0; i < cnt; ++i)
	{
		view_cross_section_[i]->RequestedCreateDCMFiles(str, nerve_visible, implant_visible, filter, thickness);
	}
}
#endif

#ifdef WILL3D_EUROPE
void PanoViewMgr::SetSyncControlButtonOut()
{
	for (int i = 0; i < common::kMaxCrossSection; ++i)
	{
		view_cross_section_[i].get()->SetSyncControlButton(false);
	}

	view_pano_arch_.get()->SetSyncControlButton(false);
	view_pano_.get()->SetSyncControlButton(false);

}
#endif // WILL3D_EUROPE

void PanoViewMgr::slotMaximizeSingleCrossSectionView()
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
