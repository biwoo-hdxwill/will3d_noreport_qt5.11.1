#include "base_pano_view_mgr.h"

#include <QDebug>
#include <QtConcurrent/QtConcurrent>

#include <Engine/Common/Common/global_preferences.h>

#include <Engine/Common/Common/W3Math.h>
#include "../../Engine/Common/Common/W3Logger.h"
#include "../../Engine/Common/Common/W3ProgressDialog.h"
#include "../../Engine/Common/Common/define_measure.h"
#include "../../Engine/Common/Common/define_pano.h"

#include "../../Engine/Common/GLfunctions/W3GLTypes.h"

#include "../../Engine/Resource/ResContainer/resource_container.h"
#include "../../Engine/Resource/Resource/cross_section_resource.h"
#include "../../Engine/Resource/Resource/implant_resource.h"
#include "../../Engine/Resource/Resource/nerve_resource.h"
#include "../../Engine/Resource/Resource/pano_resource.h"

#include "../../Engine/Module/Panorama/pano_algorithm.h"
#include "../../Engine/Module/Panorama/pano_engine.h"

#include "../../Engine/UIModule/UIComponent/base_view_pano.h"
#include "../../Engine/UIModule/UIComponent/base_view_pano_arch.h"
#include "../../Engine/UIModule/UIComponent/base_view_pano_cross_section.h"
#include "../../Engine/UIModule/UIPrimitive/W3SpanSlider.h"

#include <Engine/UIModule/UITools/base_pano_tool.h>

BasePanoViewMgr::BasePanoViewMgr(QObject* parent) : BaseViewMgr(parent) {}
BasePanoViewMgr::~BasePanoViewMgr() {}

void BasePanoViewMgr::connections() {
	for (const auto& elem : base_view_cross_sections_) {
		const auto& view = elem.second.get();
		connect(view, &BaseViewPanoCrossSection::sigWheelDelta, this,
			&BasePanoViewMgr::slotCrossTranslateFromCrossView);

		connect(view, &BaseViewPanoCrossSection::sigProcessedLightEvent, this,
			&BasePanoViewMgr::slotProcessedLightEvent);
		connect(view, &BaseViewPanoCrossSection::sigProcessedZoomEvent, this, 
			&BasePanoViewMgr::slotCrossViewProcessedZoomEvent);
		connect(view, &BaseViewPanoCrossSection::sigProcessedPanEvent, this,
			&BasePanoViewMgr::slotCrossViewProcessedPanEvent);
		connect(view, &BaseViewPanoCrossSection::sigSetSharpen, this,
			&BasePanoViewMgr::slotCrossViewSharpen);
		connect(view, &BaseViewPanoCrossSection::sigHoverView, this,
			&BasePanoViewMgr::slotCrossViewHovered);
		connect(view, &BaseViewPanoCrossSection::sigSetAxialSlice, this,
			&BasePanoViewMgr::slotSetAxialSliceFromCrossView);
		connect(view, &BaseViewPanoCrossSection::sigZoomDone, this,
			&BasePanoViewMgr::slotCrossViewZoomDoneEvent);
		connect(view, &BaseViewPanoCrossSection::sigMeasureCreated, this,
			&BasePanoViewMgr::slotCrossMeasureCreated);
		connect(view, &BaseViewPanoCrossSection::sigMeasureDeleted, this,
			&BasePanoViewMgr::slotCrossMeasureDeleted);
		connect(view, &BaseViewPanoCrossSection::sigMeasureModified, this,
			&BasePanoViewMgr::slotCrossMeasureModified);
		connect(view, &BaseViewPanoCrossSection::sigMeasureResourceUpdateNeeded,
			this, &BasePanoViewMgr::slotCrossMeasureUpdateNeeded);

		connect(view, &BaseViewPanoCrossSection::sigRequestSyncViewStatus, this,
			&BasePanoViewMgr::slotSetCrossSectionViewStatus);
	}

	const auto& view_arch = base_view_pano_arch_.get();
	connect(view_arch, &BaseViewPanoArch::sigChangedArchType, this,
		&BasePanoViewMgr::slotChangedArchTypeFromPanoArch);
	connect(view_arch, &BaseViewPanoArch::sigUpdatedArch, this,
		&BasePanoViewMgr::slotArchUpdateFromPanoArch);
	connect(view_arch, &BaseViewPanoArch::sigProcessedLightEvent, this,
		&BasePanoViewMgr::slotProcessedLightEvent);
	connect(view_arch, &BaseViewPanoArch::sigShiftedArch, this,
		&BasePanoViewMgr::slotArchShifteFromPanoArch);
	connect(view_arch, &BaseViewPanoArch::sigUpdatedFinishArch, this,
		&BasePanoViewMgr::slotArchUpdateFinishFromPanoArch);
	connect(view_arch, &BaseViewPanoArch::sigRequestInitialize, this,
		&BasePanoViewMgr::slotRequestInitializeFromPanoArch);
	connect(view_arch, &BaseViewPanoArch::sigTranslateZ, this,
		&BasePanoViewMgr::slotTranslateZfromPanoArch);
	//connect(view_arch, &BaseViewPanoArch::sigAutoArch, this, &BasePanoViewMgr::slotAutoArch);

	const auto& view_pano = base_view_pano_.get();
	connect(view_pano, &BaseViewPano::sigProcessedLightEvent, this,
		&BasePanoViewMgr::slotProcessedLightEvent);
	connect(view_pano, &BaseViewPano::sigReconTypeChanged, this,
		&BasePanoViewMgr::slotReconTypeChangedFromPano);
	connect(view_pano, &BaseViewPano::sigTranslateZ, this,
		&BasePanoViewMgr::slotTranslateZfromPano);
	connect(view_pano, &BaseViewPano::sigReconPanoResource, this,
		&BasePanoViewMgr::slotReconPanoResource);
	connect(view_pano, &BaseViewPano::sigTranslatedCrossSection, this,
		&BasePanoViewMgr::slotCrossTranslateFromPanoView);
	connect(view_pano, &BaseViewPano::sigCrossSectionLineReleased, this, &BasePanoViewMgr::slotCrossSectionLineReleased);
	connect(view_pano, &BaseViewPano::sigSetAxialSlice, this,
		&BasePanoViewMgr::slotSetAxialSliceFromPanoView);
	connect(view_pano, &BaseViewPano::sigRotateCrossSection, this,
		&BasePanoViewMgr::slotCrossSectionRotate);
	connect(view_pano, &BaseViewPano::sigDisplayDICOMInfo, this,
		&BasePanoViewMgr::slotDisplayDICOMInfoFromPanoView);
	connect(view_pano, &BaseViewPano::sigGetProfileData, this,
		&BasePanoViewMgr::slotSetProfileDataFromPanoView);
	connect(view_pano, &BaseViewPano::sigGetROIData, this,
		&BasePanoViewMgr::slotSetROIDataFromPanoView);

	connect(view_pano, &BaseViewPano::sigZoomDone, this,
		&BasePanoViewMgr::slotPanoViewZoomDoneEvent);

	connect(view_pano, &BaseViewPano::sigToggleImplantRenderingType, this, &BasePanoViewMgr::slotToggleImplantRenderingType);
}

void BasePanoViewMgr::ConnectPanoMenus() {
	task_tool_base_->Connections();
	connect(task_tool_base_.get(),
		&BasePanoTaskTool::sigBasePanoClipParamsChanged, this,
		&BasePanoViewMgr::slotBasePanoClipParamsChanged);
	connect(task_tool_base_.get(), &BasePanoTaskTool::sigBasePanoCSParamsChanged,
		this, &BasePanoViewMgr::slotCrossSectionToolsChanged);

	connect(task_tool_base_.get(), &BasePanoTaskTool::sigBasePanoArchRangeChanged,
		this, &BasePanoViewMgr::slotChangePanoArchRange);
	connect(task_tool_base_.get(),
		&BasePanoTaskTool::sigBasePanoArchThicknessChanged, this,
		&BasePanoViewMgr::slotChangePanoThickness);

	connect(task_tool_base_.get(), &BasePanoTaskTool::sigBasePanoVisible, this,
		&BasePanoViewMgr::slotBasePanoVisible);
}

void BasePanoViewMgr::DisconnectPanoMenus()
{
	task_tool_base_->Disconnections();
	disconnect(task_tool_base_.get(), &BasePanoTaskTool::sigBasePanoClipParamsChanged, this, &BasePanoViewMgr::slotBasePanoClipParamsChanged);
	disconnect(task_tool_base_.get(), &BasePanoTaskTool::sigBasePanoCSParamsChanged, this, &BasePanoViewMgr::slotCrossSectionToolsChanged);

	disconnect(task_tool_base_.get(), &BasePanoTaskTool::sigBasePanoArchRangeChanged, this, &BasePanoViewMgr::slotChangePanoArchRange);
	disconnect(task_tool_base_.get(), &BasePanoTaskTool::sigBasePanoArchThicknessChanged, this, &BasePanoViewMgr::slotChangePanoThickness);

	disconnect(task_tool_base_.get(), &BasePanoTaskTool::sigBasePanoVisible, this, &BasePanoViewMgr::slotBasePanoVisible);
}

void BasePanoViewMgr::SyncPanoResource() {
	base_view_pano_arch_->SetCurrentArchType(pano_engine_->curr_arch_type());

	if (pano_engine_->IsValidPanorama()) {
		const auto& res_pano = ResourceContainer::GetInstance()->GetPanoResource();
		const auto& vol = ResourceContainer::GetInstance()->GetMainVolume();
		const float base_pixel_spacing =
			std::min(vol.pixelSpacing(), vol.sliceSpacing());

		float thickness_mm = res_pano.thickness_value() * base_pixel_spacing;
		float range_mm = res_pano.range_value_mm();

		task_tool_base_->blockSignals(true);
		if (range_mm != 0.0f) task_tool_base_->SetArchRange(range_mm);
		task_tool_base_->SetArchThickness(thickness_mm);
		task_tool_base_->blockSignals(false);

		base_view_pano_arch_->ForceRotateMatrix(pano_engine_->reorien_mat());

		const auto& pano_roi = pano_engine_->GetPanoROI();
		base_view_pano_arch_->SetSliceInVol(pano_roi.slice);

		if (base_view_pano_arch_->is_view_ready()) {
			const std::vector<glm::vec3>& points = res_pano.pano_ctrl_points();
			std::vector<glm::vec3> old_points;
			base_view_pano_arch_->GetArchCtrlPointsInVol(old_points);

			float difference = 0.0f;
			bool is_same = false;

			if (old_points.size() == points.size()) {
				const glm::vec3 pano_back_vector = res_pano.back_vector();
				for (int i = 0; i < points.size(); i++) {
					const glm::vec3 np = points[i] - points[i] * pano_back_vector;
					const glm::vec3 op = old_points[i] - old_points[i] * pano_back_vector;
					difference += glm::length(np - op);
				}
				if (difference < 1.0f) is_same = true;
			}

			if (is_same) {
				base_view_pano_->SetSliceNum((int)res_pano.shifted_value() +
					res_pano.pano_3d_depth() / 2);

				ReconPanoResource();

				BasePanoViewMgr::ReconNerveResource();
				pano_engine_->ReconPanoramaImplantMask(
					base_view_pano_->GetSceneScale());

				base_view_pano_->UpdatedPano();

				InitCrossSectionResource();

				UpdateCrossSection();
			}
			else {
				base_view_pano_arch_->SetArchPointsInVol(points);
			}

			UpdateAxialLine();
			InitArchLinePanoView();
		}
	}
}

void BasePanoViewMgr::SyncMeasureResource()
{
	base_view_pano_->SyncMeasureResourceCounterparts(true);
	base_view_pano_arch_->SyncMeasureResourceCounterparts(true);

#if 1
	for (int i = 0; i < cross_section_count_; ++i)
	{
		bool is_update_resource = (i == cross_section_count_ - 1) ? true : false;
		base_view_cross_sections_[i]->SyncMeasureResourceCounterparts(is_update_resource, i == 0);
	}
#else
	bool is_update_resource = (0 == cross_section_count_ - 1) ? true : false;
	base_view_cross_sections_[0]->SyncMeasureResourceCounterparts(is_update_resource);
#endif

}

void BasePanoViewMgr::SetPanoEngine(
	const std::shared_ptr<PanoEngine>& pano_engine) {
	pano_engine_ = pano_engine;
}

void BasePanoViewMgr::UpdateCrossSection(bool is_set_view_pano) {
	base_view_pano_arch_->CrossSectionUpdated();

	for (int i = 0; i < cross_section_count_; i++)
	{
		base_view_cross_sections_[i]->UpdateCrossSection();
		//base_view_cross_sections_[i]->SyncMeasureResourceSiblings(true);
}

	if (is_set_view_pano) base_view_pano_->CrossSectionUpdated();
}

void BasePanoViewMgr::InitCrossSectionResource() {
	pano_engine_->InitCrossSectionResource();
	SetCrossSectionParams();
}
void BasePanoViewMgr::SelectLayoutCrossSection(int row, int col)
{
	cross_section_count_ = row * col;

	for (int i = 0; i < common::kMaxCrossSection; ++i)
	{
		base_view_cross_sections_[i]->SetEnabledSharpenUI(false);
	}
	base_view_cross_sections_[col - 1]->SetEnabledSharpenUI(true);

	if (pano_engine_->IsValidPanorama())
	{
		base_view_pano_->ClearCrossSectionLine();

		InitCrossSectionResource();

		UpdateCrossSection();
		InitAxialLineCrossView();

		for (int i = 0; i < cross_section_count_; ++i)
		{
			base_view_cross_sections_[i]->SyncMeasureResourceSiblings(false);
		}
	}
}

void BasePanoViewMgr::SetVisibleViews(bool bVisible) {
	base_view_pano_->setVisible(bVisible);
	base_view_pano_arch_->setVisible(bVisible);

	for (int i = 0; i < cross_section_count_; i++) {
		base_view_cross_sections_[i]->setVisible(bVisible);
	}
}

void BasePanoViewMgr::set_task_tool_base(
	const std::shared_ptr<BasePanoTaskTool>& task_tool) {
	task_tool_base_ = task_tool;
	task_tool_base_->SetActivateClippingTools(false);
	ConnectPanoMenus();
}

std::vector<QWidget*> BasePanoViewMgr::GetViewPanoCrossSectionWidget() const {
	std::vector<QWidget*> widgets;
	widgets.reserve(base_view_cross_sections_.size());
	for (const auto& elem : base_view_cross_sections_)
		widgets.push_back((QWidget*)(elem.second.get()));

	return widgets;
}

void BasePanoViewMgr::slotProcessedLightEvent() {
	base_view_pano_->UpdatedPano();
	base_view_pano_arch_->UpdateSlice();

	for (int i = 0; i < cross_section_count_; i++)
		base_view_cross_sections_[i]->UpdateCrossSection();
}

void BasePanoViewMgr::slotTranslateZfromPanoArch(float z_pos_vol) {
	pano_engine_->SetPanoROISlice(z_pos_vol);

	if (pano_engine_->IsValidPanorama()) {
		QPointF pt_pano_plane = pano_engine_->MapVolToPanoPlane(
			base_view_pano_arch_->GetCenterPosition());
		base_view_pano_->SetAxialLine(pt_pano_plane);
	}

	if (pano_engine_->IsValidCrossSection()) {
		for (int i = 0; i < cross_section_count_; i++) {
			QPointF pt_cross_plane = pano_engine_->MapVolToCrossPlane(
				i, base_view_pano_arch_->GetCenterPosition());
			base_view_cross_sections_[i]->SetAxialLine(pt_cross_plane);
		}
	}
}

void BasePanoViewMgr::slotTranslateZfromPano(float z_delta) {
	const auto& pano_res = ResourceContainer::GetInstance()->GetPanoResource();
	int origin_z_pos = pano_res.pano_3d_depth() / 2;
	int shifted_value = (int)z_delta - origin_z_pos;

	if (base_view_pano_->recon_type() == BaseViewPano::ReconType::RECON_MPR) {
		base_view_pano_arch_->SetShiftedArch(shifted_value);
	}
}

void BasePanoViewMgr::slotReconPanoResource() {
	if (base_view_pano_->recon_type() == BaseViewPano::RECON_MPR) {
		pano_engine_->ReconPanoramaNerveMask();
	}
	else if (base_view_pano_->recon_type() == BaseViewPano::RECON_XRAY) {
		pano_engine_->ReconPanoramaXrayNerveMask();
	}

	ReconPanoResource();
}

void BasePanoViewMgr::slotBasePanoClipParamsChanged(
	const std::vector<glm::vec4>& clipping_planes, bool is_enable) {
	base_view_pano_->SetCliping(clipping_planes, is_enable);
}

void BasePanoViewMgr::slotCrossSectionToolsChanged() {
	if (!pano_engine_->IsValidPanorama()) return;

	SetCrossSectionParams();
	UpdateCrossSection();
}

void BasePanoViewMgr::SetCrossSectionParams() {
	float interval, degree, thickness;
	int width, height;

	GetCrossSectionParamsFromTools(&interval, &width, &height, &thickness,
		&degree);

#if 1
	emit sigSyncCrossSectionParams(task_tool_base_->GetCrossSectionInterval(), task_tool_base_->GetCrossSectionAngle(), task_tool_base_->GetCrossSectionThickness());
#endif

	pano_engine_->SetCrossSectionParams(interval, width, height, thickness,
		degree, cross_section_count_);
}

void BasePanoViewMgr::GetCrossSectionParamsFromTools(float* interval, int* width, int* height, float* thickness, float* degree) 
{
	const CW3Image3D& vol = ResourceContainer::GetInstance()->GetMainVolume();
	const float interval_mm = task_tool_base_->GetCrossSectionInterval();
	const float length_mm = task_tool_base_->GetArchRange();
	const float thickness_mm = task_tool_base_->GetCrossSectionThickness();

	const float base_pixel_spacing = std::min(vol.pixelSpacing(), vol.sliceSpacing());
	const auto& pano_roi = pano_engine_->GetPanoROI();

	*interval = interval_mm / base_pixel_spacing;
	*width = length_mm / base_pixel_spacing;
	*height = pano_roi.bottom - pano_roi.top;
	*thickness = thickness_mm / base_pixel_spacing;
	*degree = task_tool_base_->GetCrossSectionAngle();

	const auto& res_cross = ResourceContainer::GetInstance()->GetCrossSectionResource();
	if (&res_cross) 
	{
		const auto& pano_curve = res_cross.pano_curve();
		if (*interval * cross_section_count_ > (float)(pano_curve.points().size()) && !pano_curve.points().empty()) 
		{
			*interval = (float)(pano_curve.points().size()) / cross_section_count_;
			task_tool_base_->SetCrossSectionInterval((double)*interval);
		}
	}
}

void BasePanoViewMgr::slotAutoArch(const bool use_current_slice) {
	try {
		const CW3Image3D& vol = ResourceContainer::GetInstance()->GetMainVolume();
		std::vector<glm::vec3> points;
		ArchTypeID arch_type = pano_engine_->curr_arch_type();

		int target_slice = (use_current_slice) ? base_view_pano_arch_->GetSliceInVol() : -1;
		switch (arch_type) {
		case ARCH_MAXILLA:
			PanoAlgorithm::RunMaxillaAutoArch(&vol, points, target_slice);
			break;
		case ARCH_MANDLBLE:
			PanoAlgorithm::RunMandibleAutoArch(&vol, points, target_slice);
			break;
		default:
			assert(false);
			break;
		}

		if (!points.empty())
		{
			if (!use_current_slice)
			{
				base_view_pano_arch_->SetSliceInVol(points.front().z);
			}
			base_view_pano_arch_->SetArchPointsInVol(points);
		}
	}
	catch (const std::exception& e) {
		common::Logger::instance()->Print(common::LogType::WRN, e.what());
		return;
	}
}

void BasePanoViewMgr::slotReconTypeChangedFromPano() {
	if (base_view_pano_->recon_type() == BaseViewPano::ReconType::RECON_3D) {
		task_tool_base_->SetActivateClippingTools(true);
	}
	else {
		task_tool_base_->SetActivateClippingTools(false);
		InitAxialLinePanoView();
	}
}

void BasePanoViewMgr::slotDisplayDICOMInfoFromPanoView(
	const QPointF& pt_pano_plane, glm::vec4& vol_info) {
	const CW3Image3D& vol = ResourceContainer::GetInstance()->GetMainVolume();
	if (!&vol) {
		common::Logger::instance()->Print(
			common::LogType::ERR,
			"BasePanoViewMgr::slotDisplayDICOMInfoFromPanoView : volume is "
			"nullptr");
		assert(false);
	}
	glm::vec3 vol_pt = pano_engine_->MapPanoPlaneToVol(pt_pano_plane, false);
	vol_info = vol.GetVolumeInfo(vol_pt);
}

void BasePanoViewMgr::slotSetProfileDataFromPanoView(
	const QPointF& start_plane_pos, const QPointF& end_plane_pos,
	std::vector<short>& data) {
	pano_engine_->GetProfileDataInPanoPlane(start_plane_pos, end_plane_pos, data);
}

void BasePanoViewMgr::slotSetROIDataFromPanoView(const QPointF& start_plane_pos,
	const QPointF& end_plane_pos,
	std::vector<short>& data) {
	pano_engine_->GetROIDataInPanoPlane(start_plane_pos, end_plane_pos, data);
}

void BasePanoViewMgr::slotImplantHoveredFromPano(const QPointF& pt_pano_plane,
	int* hovered_id) const {
	pano_engine_->HoveredImplantInPanoPlane(pt_pano_plane, hovered_id);
}

void BasePanoViewMgr::slotSetCrossSectionViewStatus(float* scene_scale,
	QPointF* gl_translate) {
	if (!pano_engine_->IsValidPanorama()) return;

	*scene_scale = base_view_cross_sections_[0]->GetSceneScale();
	*gl_translate = base_view_cross_sections_[0]->GetGLTranslate();
}

void BasePanoViewMgr::slotArchUpdateFinishFromPanoArch()
{
	GeneratePanorama();
}

void BasePanoViewMgr::slotCrossSectionRotate(float delta_angle) {
	task_tool_base_->UpdateCrossSectionAngle(delta_angle);
}

void BasePanoViewMgr::slotSetAxialSliceFromPanoView(
	const QPointF& pt_pano_plane) {
	glm::vec3 pt_vol = pano_engine()->MapPanoPlaneToVol(pt_pano_plane, false);
	if (!pano_engine()->IsValidVolumePos(pt_vol)) return;

	this->SetAxialZposition(pt_vol);
}

void BasePanoViewMgr::slotCrossTranslateFromPanoView(const float& shifted) {
	pano_engine_->EditCrossSectionShiftedValue(shifted);
#if 0
	UpdateCrossSection(false);
#else
	UpdateCrossSection();
#endif
}

void BasePanoViewMgr::slotCrossSectionLineReleased()
{
	pano_engine_->ResetDeltaShiftedValue();
}

void BasePanoViewMgr::slotCrossTranslateFromCrossView(int delta) {
	const auto& res_cross =
		ResourceContainer::GetInstance()->GetCrossSectionResource();
	if (&res_cross == nullptr) return;

	float interval = res_cross.params().interval;

	bool slide_as_set = GlobalPreferences::GetInstance()->preferences_.advanced.cross_section_view.slide_as_set;
	int slide_interval = 1;
	if (slide_as_set)
	{
		slide_interval = cross_section_count_;
	}

	pano_engine_->EditCrossSectionShiftedValue(interval * (float)delta * slide_interval);

	UpdateCrossSection();
}

void BasePanoViewMgr::slotCrossViewProcessedZoomEvent(float scale) {
	QObject* sender = QObject::sender();

	for (int i = 0; i < common::kMaxCrossSection; i++) {
		if (base_view_cross_sections_[i].get() == sender) continue;

		base_view_cross_sections_[i]->SetZoomScale(scale);
	}
}
void BasePanoViewMgr::slotCrossViewProcessedPanEvent(const QPointF& translate) {
	QObject* sender = QObject::sender();

	for (int i = 0; i < common::kMaxCrossSection; i++) {
		if (base_view_cross_sections_[i].get() == sender) continue;

		base_view_cross_sections_[i]->SetPanTranslate(translate);
	}
}
void BasePanoViewMgr::slotCrossViewHovered(bool is_hovered) {
	QObject* sender = QObject::sender();

	for (int i = 0; i < cross_section_count_; i++) {
		if (base_view_cross_sections_[i].get() != sender) continue;

		base_view_pano_arch_->SetHighlightCrossSection(i, is_hovered);
		base_view_pano_->SetHighlightCrossSection(i, is_hovered);
	}
}
void BasePanoViewMgr::slotCrossViewSharpen(SharpenLevel level) {
	QObject* sender = QObject::sender();

	for (int i = 0; i < common::kMaxCrossSection; i++) {
		if (base_view_cross_sections_[i].get() == sender) continue;

		base_view_cross_sections_[i]->SetSharpen(level);
	}
}
void BasePanoViewMgr::slotCrossMeasureCreated(const int& cross_section_id, const unsigned int& measure_id)
{
	for (int i = 0; i < cross_section_count_; ++i)
	{
		if (i == cross_section_id)
		{
			continue;
		}

		base_view_cross_sections_[i]->SyncCreateMeasureUI(measure_id);
	}
}

void BasePanoViewMgr::slotCrossMeasureDeleted(const int& cross_section_id,
	const unsigned int& measure_id) {
	for (int i = 0; i < cross_section_count_; i++) {
		if (i == cross_section_id) continue;

		base_view_cross_sections_[i]->SyncDeleteMeasureUI(measure_id);
	}
}

void BasePanoViewMgr::slotCrossMeasureModified(const int& cross_section_id, const unsigned int& measure_id)
{
	for (int i = 0; i < cross_section_count_; i++)
	{
		if (i == cross_section_id)
		{
			continue;
		}

		base_view_cross_sections_[i]->SyncModifyMeasureUI(measure_id);
	}
}

void BasePanoViewMgr::slotCrossMeasureUpdateNeeded(const int& cross_section_id,
	bool& is_need_update) {
	is_need_update =
		(cross_section_id == cross_section_count_ - 1) ? true : false;
}

void BasePanoViewMgr::slotSetAxialSliceFromCrossView(int cross_id,
	const glm::vec3& pt_vol) {
	if (!pano_engine()->IsValidVolumePos(pt_vol)) return;

	this->SetAxialZposition(pt_vol);
}

void BasePanoViewMgr::slotChangedArchTypeFromPanoArch(const ArchTypeID& type) {
	ChangedArchType(type);
}
void BasePanoViewMgr::slotArchUpdateFromPanoArch() {
	if (IsPanoReconMPR()) {
		BasePanoViewMgr::InitPanoResource();

		BasePanoViewMgr::ReconPanoResource();
		BasePanoViewMgr::ReconNerveResource();
		pano_engine_->ReconPanoramaImplantMask(base_view_pano_->GetSceneScale());

		base_view_pano_arch_->UpdatedPanoRuler();
		base_view_pano_->UpdatedPano();
	}

	DeleteMeasuresInPanorama();
}

// TODO : 과거 arch view에서 시작된 initialize 때문에 있는것.
// Engine 으로 init을 옮겼기 때문에 나중에 정리해줘야 함 with 유대리님
void BasePanoViewMgr::slotArchShifteFromPanoArch(float shifted_value_in_vol) {
	pano_engine_->SetPanoramaShiftedValue(shifted_value_in_vol);

	ReconAllResources();

	base_view_pano_arch_->UpdatedPanoRuler();
	base_view_pano_->UpdatedPano();
	UpdateCrossSection();
	UpdateAxialLine();
	InitArchLinePanoView();
}
void BasePanoViewMgr::slotBasePanoVisible(const VisibleID& visible_id,
	bool is_visible) {
	if (visible_id == VisibleID::NERVE)
		VisibleNerve(is_visible);
	else if (visible_id == VisibleID::IMPLANT)
		VisibleImplant(is_visible);
}

// TODO. panotab과 implanttab이 같은 check box UI를 사용하고 있어서
// panotab에서 발생한 checkbox signal이 implant tab에서 발생함..............
// 임시방편으로 view의 visible on인 상태에만 동작하도록 함.
void BasePanoViewMgr::VisibleNerve(bool is_visible) {
	if (!base_view_pano_->isVisible() &&
		!base_view_pano_arch_->isVisible())
	{
		return;
	}

	pano_engine_->SetNerveVisibleAll(is_visible);

	this->ReconNerveResource();

	base_view_pano_->UpdatedPano();
	base_view_pano_arch_->UpdateSlice();
	for (int i = 0; i < cross_section_count_; i++) {
		base_view_cross_sections_[i]->UpdateCrossSection();
	}
}
void BasePanoViewMgr::VisibleImplant(bool is_visible) {
	if (!base_view_pano_->isVisible() &&
		!base_view_pano_arch_->isVisible())
	{
		return;
	}

	pano_engine_->SetImplantVisibleAll(is_visible);

	this->ReconImplantResource();

	base_view_pano_->UpdateImplantHandleAndSpec();
	base_view_pano_->UpdatedPano();

	base_view_pano_arch_->UpdateSlice();
	for (int i = 0; i < cross_section_count_; i++) {
		base_view_cross_sections_[i]->UpdateCrossSection();
	}
}

void BasePanoViewMgr::slotChangedArchRangeFromPanoArch(float range_mm) {
	task_tool_base_->SetArchRange(range_mm);
	float range = task_tool_base_->GetArchRange();
	base_view_pano_arch_->SetPanoArchRange(range);
}

void BasePanoViewMgr::slotChangePanoArchRange(double value) {
	base_view_pano_arch_->SetPanoArchRange(value);
}

void BasePanoViewMgr::slotChangePanoThickness(double value) {
	base_view_pano_arch_->SetPanoArchThickness(value);
}
void BasePanoViewMgr::slotRequestInitializeFromPanoArch() {
	const auto& pano_roi = pano_engine_->GetPanoROI();

	base_view_pano_arch_->InitSliceRange(pano_roi.top, pano_roi.bottom,
		pano_roi.slice);
	base_view_pano_arch_->SetPanoArchRange(task_tool_base_->GetArchRange());
	base_view_pano_arch_->ForceRotateMatrix(pano_engine_->reorien_mat());

	if (pano_engine_->IsValidPanorama()) {
		const auto& res_pano = ResourceContainer::GetInstance()->GetPanoResource();
		const std::vector<glm::vec3>& points = res_pano.pano_ctrl_points();
		float shifted_value = res_pano.shifted_value();
		base_view_pano_arch_->SetArchPointsInVol(points);

		base_view_pano_->SetSliceNum(
			(int)shifted_value +
			ResourceContainer::GetInstance()->GetPanoResource().pano_3d_depth() /
			2);

		base_view_pano_arch_->SetCurrentArchType(pano_engine_->curr_arch_type());
	}
	else if (!init_arch_from_mpr_)
	{
		slotAutoArch(false);
		GeneratePanorama();
	}

	bool is_update_surface_objs = false;

	if (pano_engine_->IsValidImplant()) {
		pano_engine_->UpdateImplantPositions();

		if (!task_tool_base_->IsVisibleImplantEnable() ||
			task_tool_base_->IsVisibleImplantChecked()) {
			pano_engine_->SetImplantVisibleAll(true);
		}
		is_update_surface_objs = true;
	}

	if (pano_engine_->IsValidNerve()) {
		pano_engine_->UpdateNerveResource();
		is_update_surface_objs = true;
	}

	if (is_update_surface_objs) UpdateSurfaceObjs();
}

void BasePanoViewMgr::ReconAllResources() {
	ReconPanoResource();
	ReconNerveResource();
	ReconImplantResource();
}

void BasePanoViewMgr::slotCrossViewZoomDoneEvent() {
	for (int i = 0; i < cross_section_count_; i++) {
		base_view_cross_sections_[i]->UpdateCrossSection();
	}
}
void BasePanoViewMgr::slotPanoViewZoomDoneEvent() {
	pano_engine_->ReconPanoramaImplantMask(base_view_pano_->GetSceneScale());
	base_view_pano_->UpdatedPano();
}

/**
Private functions
*/

void BasePanoViewMgr::ReconPanoResource() {
	if (base_view_pano_->recon_type() == BaseViewPano::RECON_MPR) {
		pano_engine_->ReconPanoramaPlane();
	}
	else if (base_view_pano_->recon_type() == BaseViewPano::RECON_XRAY) {
		CW3ProgressDialog* progress =
			CW3ProgressDialog::getInstance(CW3ProgressDialog::WAITTING);
		QFutureWatcher<void> watcher;
		connect(&watcher, SIGNAL(finished()), progress, SLOT(hide()));

		auto future = QtConcurrent::run(pano_engine_.get(),
			&PanoEngine::ReconPanoramaXrayEnhancement);
		watcher.setFuture(future);
		progress->exec();
		watcher.waitForFinished();
	}
	else if (base_view_pano_->recon_type() == BaseViewPano::RECON_3D) {
		CW3ProgressDialog* progress =
			CW3ProgressDialog::getInstance(CW3ProgressDialog::WAITTING);
		QFutureWatcher<void> watcher;
		connect(&watcher, SIGNAL(finished()), progress, SLOT(hide()));

		auto future =
			QtConcurrent::run(pano_engine_.get(), &PanoEngine::ReconPanorama3D);
		watcher.setFuture(future);
		progress->exec();
		watcher.waitForFinished();
	}
	else
		assert(false);
}
void BasePanoViewMgr::ReconNerveResource() {
	if (base_view_pano_->recon_type() == BaseViewPano::RECON_MPR) {
		pano_engine_->ReconPanoramaNerveMask();
	}
	else if (base_view_pano_->recon_type() == BaseViewPano::RECON_XRAY) {
		pano_engine_->ReconPanoramaXrayNerveMask();
	}
}
void BasePanoViewMgr::ReconImplantResource() {
	if (base_view_pano_->recon_type() == BaseViewPano::RECON_MPR ||
		base_view_pano_->recon_type() == BaseViewPano::RECON_XRAY) {
		pano_engine_->ReconPanoramaImplantMask(base_view_pano_->GetSceneScale());
	}
}
void BasePanoViewMgr::UpdateSurfaceObjs() {
	pano_engine()->CheckCollideNerve();

	this->ReconNerveResource();
	this->ReconImplantResource();

	base_view_pano_->UpdatedPano();
	base_view_pano_arch_->UpdateSlice();
	for (int i = 0; i < cross_section_count(); i++) {
		base_view_cross_sections_[i]->UpdateCrossSection();
	}
}
void BasePanoViewMgr::SetViewPanoSliceRange() {
	if (IsSetPanoArch() && IsPanoReconMPR()) {
		const auto& res_pano = ResourceContainer::GetInstance()->GetPanoResource();
		base_view_pano_->SetEnableSlider(true);
		base_view_pano_->SetSliceRange(0, res_pano.pano_3d_depth() - 1);
		base_view_pano_->SetSliceNum((int)res_pano.shifted_value() +
			res_pano.pano_3d_depth() / 2);
	}
	else {
		base_view_pano_->SetEnableSlider(false);
	}
}
void BasePanoViewMgr::SetAxialZposition(const glm::vec3& vol) {
	float delta = glm::dot(base_view_pano_arch_->GetUpVector(),
		vol - base_view_pano_arch_->GetCenterPosition());

	if (abs(delta) > std::numeric_limits<float>::epsilon()) {
		base_view_pano_arch_->SetSliceInVol(base_view_pano_arch_->GetSliceInVol() +
			delta);
	}
}

void BasePanoViewMgr::InitPanoResource() 
{
	std::vector<glm::vec3> pano_points, pano_ctrl_points;
	glm::vec3 pano_back_vector;
	float pano_depth, pano_shifted, pano_thickness, pano_range_mm;

	GetPanoPoints(pano_points);
	GetPanoCtrlPoints(pano_ctrl_points);
	GetPanoBackVector(pano_back_vector);
	GetPanoRange(pano_depth, pano_range_mm);
	GetPanoShifted(pano_shifted);
	GetPanoThickness(pano_thickness);

	pano_engine_->InitPanoramaResource(pano_points, pano_ctrl_points,
		pano_back_vector, pano_depth, pano_range_mm, pano_shifted,
		pano_thickness);

	SetViewPanoSliceRange();
}

void BasePanoViewMgr::UpdateAxialLine() {
	InitAxialLinePanoView();
	InitAxialLineCrossView();
}

/**********************************************************************************************
Initializes the panorama roi.
 Axial View의 현재 위치와 범위를 초기화된다. 이 결과, 파노라마 위치도
초기화된다.
 **********************************************************************************************/

void BasePanoViewMgr::InitPanoramaROI() {
	auto& volume = ResourceContainer::GetInstance()->GetMainVolume();
	pano_engine_->InitSliceLocation(volume);

	// 아래 SetSliceXXX 에서 Engine의 ROI가 바뀌기 때문에
	// 값 복사로 roi를 갖고 있어야 함
	const PanoEngine::PanoROI& roi = pano_engine_->GetPanoROI();

	int min, max;
	base_view_pano_arch_->GetSliceRange(&min, &max);
	if ((int)roi.top != min || (int)roi.bottom != max)
		base_view_pano_arch_->SetSliceRange(roi.top, roi.bottom);

	if (base_view_pano_arch_->GetSliceInVol() != (int)roi.slice)
		base_view_pano_arch_->SetSliceInVol(roi.slice);
}

void BasePanoViewMgr::InitAxialLinePanoView() {
	if (pano_engine_->IsValidPanorama()) {
		QPointF pt_pano_plane = pano_engine_->MapVolToPanoPlane(
			base_view_pano_arch_->GetCenterPosition());
		base_view_pano_->SetAxialLine(pt_pano_plane);
	}
}

void BasePanoViewMgr::InitArchLinePanoView() {
	if (pano_engine_->IsValidPanorama()) {
		const auto& res_pano = ResourceContainer::GetInstance()->GetPanoResource();
		QPointF pt_pano_plane = pano_engine_->MapVolToPanoPlane(
			res_pano.pano_ctrl_points().at(res_pano.pano_ctrl_points().size() / 2));
		base_view_pano_->SetArchLine(pt_pano_plane);
	}
}

void BasePanoViewMgr::InitAxialLineCrossView() {
	if (pano_engine_->IsValidCrossSection()) {
		for (int i = 0; i < cross_section_count_; i++) {
			QPointF pt_cross_plane = pano_engine_->MapVolToCrossPlane(
				i, base_view_pano_arch_->GetCenterPosition());
			base_view_cross_sections_[i]->SetAxialLine(pt_cross_plane);
		}
	}
}

void BasePanoViewMgr::GeneratePanorama() {
	// TODO UI에 있는 값과 싱크하기 위한 코드. GeneratePanorama함수에서 처리 할
	// 기능이 아님.
	// UI 값이 바뀌는 순간이나 뷰가 초기화 될 때 불러야 됨.
	base_view_pano_arch_->SetPanoArchRange(task_tool_base_->GetArchRange(),
		false);
	base_view_pano_arch_->SetPanoArchThickness(
		task_tool_base_->GetArchThickness(), false);
	// TODO end

	InitPanoResource();
	ReconPanoResource();

	if (pano_engine_->IsValidImplant()) pano_engine_->UpdateImplantPositions();
	if (pano_engine_->IsValidNerve()) pano_engine_->UpdateNerveResource();

	InitCrossSectionResource();

	ReconNerveResource();
	ReconImplantResource();

	base_view_pano_arch_->UpdatedPanoRuler();
	base_view_pano_->UpdatedPano();
	UpdateCrossSection();
	UpdateAxialLine();

	// Move axial line to view center
	if (GlobalPreferences::GetInstance()->preferences_.advanced.panorama_view.move_axial_line_to_view_center)
	{
		MoveAxialLineToPanoViewCenter();
	}
	if (GlobalPreferences::GetInstance()->preferences_.advanced.cross_section_view.move_axial_line_to_view_center)
	{
		MoveAxialLineToCrossSectionViewCenter();
	}
	//

	InitArchLinePanoView();
}

bool BasePanoViewMgr::IsPanoReconMPR() const {
	if (base_view_pano_->recon_type() == BaseViewPano::RECON_MPR)
		return true;
	else
		return false;
}
bool BasePanoViewMgr::IsSetPanoArch() const {
	return base_view_pano_arch_->IsSetPanoArch();
}
void BasePanoViewMgr::set_base_view_pano(
	const std::shared_ptr<BaseViewPano>& base_view_pano) {
	base_view_pano_ = base_view_pano;
	BaseViewMgr::SetCastedView(base_view_pano_->view_id(),
		std::static_pointer_cast<View>(base_view_pano_));
}
void BasePanoViewMgr::set_base_view_pano_arch(
	const std::shared_ptr<BaseViewPanoArch>& base_view_pano_arch) {
	base_view_pano_arch_ = base_view_pano_arch;
	BaseViewMgr::SetCastedView(
		base_view_pano_arch_->view_id(),
		std::static_pointer_cast<View>(base_view_pano_arch_));
}
void BasePanoViewMgr::set_base_view_pano_cross_section(
	int id,
	const std::shared_ptr<BaseViewPanoCrossSection>& base_view_cross_sections) {
	base_view_cross_sections_[id] = base_view_cross_sections;
	BaseViewMgr::SetCastedView(
		base_view_cross_sections_[id]->view_id(),
		std::static_pointer_cast<View>(base_view_cross_sections_[id]));
}
void BasePanoViewMgr::GetPanoPoints(std::vector<glm::vec3>& points) const {
	std::vector<glm::vec3> vol_arch_points;
	base_view_pano_arch_->GetArchPointsInVol(vol_arch_points);

	GetPanoTopPoints(vol_arch_points, points);
}
void BasePanoViewMgr::GetPanoCtrlPoints(
	std::vector<glm::vec3>& ctrl_points) const {
	base_view_pano_arch_->GetArchCtrlPointsInVol(ctrl_points);
}
void BasePanoViewMgr::GetPanoTopPoints(
	const std::vector<glm::vec3>& vol_arch_points,
	std::vector<glm::vec3>& points) const {
	const auto& pano_roi = pano_engine_->GetPanoROI();
	const float scalar_slice_to_top = pano_roi.slice - pano_roi.top - 0.5f;
	const glm::vec3 up_vector = base_view_pano_arch_->GetUpVector();
	const glm::vec3 up_dist = scalar_slice_to_top * up_vector;

	points.reserve(vol_arch_points.size());
	for (const glm::vec3& vol_point : vol_arch_points)
		points.push_back(vol_point - up_dist);
}
void BasePanoViewMgr::GetPanoBackVector(glm::vec3& back_vector) const {
	const auto& pano_roi = pano_engine_->GetPanoROI();
	const float pano_height = pano_roi.bottom - pano_roi.top;

	// BaseViewPanoArch에서 up_vector가 panorama의 back_vector가 된다.
	back_vector = base_view_pano_arch_->GetUpVector() * (pano_height);
}
void BasePanoViewMgr::GetPanoRange(float& depth, float& range) const {
	depth = base_view_pano_arch_->GetArchRangeInVol();
	range = task_tool_base_->GetPanoRange();
}
void BasePanoViewMgr::GetPanoShifted(float& shifted) const {
	shifted = base_view_pano_arch_->GetArchShiftedInVol();
}
void BasePanoViewMgr::GetPanoThickness(float& thickness) const {
	thickness = base_view_pano_arch_->GetArchThicknessInVol();
}

void BasePanoViewMgr::ApplyPreferences() {
	pano_engine_->EditCrossSectionShiftedValue(0.0f);
	//UpdateCrossSection();
	BaseViewMgr::ApplyPreferences();
}

void BasePanoViewMgr::DeleteMeasuresInPanorama() {
	for (int i = 0; i < common::kMaxCrossSection; ++i) {
		base_view_cross_sections_[i]->SetCommonToolOnce(
			common::CommonToolTypeOnce::M_DEL_ALL, true);
	}

	base_view_pano_->SetCommonToolOnce(common::CommonToolTypeOnce::M_DEL_ALL,
		true);
}

void BasePanoViewMgr::MoveArchViewToSelectedMeasure(
	const common::measure::VisibilityParams& visibility_param) {
	glm::vec3 curr_center = base_view_pano_arch_->GetCenterPosition();
	glm::vec3 curr_up_vector = base_view_pano_arch_->GetUpVector();

	glm::vec3 curr_center_to_moved_center = visibility_param.center - curr_center;
	float displacement = glm::dot(curr_center_to_moved_center, curr_up_vector);
	base_view_pano_arch_->SetSliceInVol(base_view_pano_arch_->GetSliceInVol() +
		displacement);
}

void BasePanoViewMgr::MovePanoViewToSelectedMeasure(
	const common::measure::VisibilityParams& visibility_param) {
	const auto& res_pano = ResourceContainer::GetInstance()->GetPanoResource();
	int slice_num = res_pano.pano_3d_depth() / 2 + (int)visibility_param.center.z;
	if (slice_num != base_view_pano_->GetSliceNum())
		base_view_pano_->SetSliceNum(slice_num);
}

void BasePanoViewMgr::MoveCrossSectionViewToSelectedMeasure(
	const common::measure::VisibilityParams& visibility_param) {
	//for (int i = 0; i < 2; ++i)
		{
			const auto& res_cross = ResourceContainer::GetInstance()->GetCrossSectionResource();
			if (&res_cross == nullptr)
			{
				return;
			}

			int pano_slice_num = pano_engine()->GetZValuePanorama(visibility_param.center);
			if (pano_slice_num != base_view_pano_->GetSliceNum())
			{
				base_view_pano_->SetSliceNum(pano_slice_num);
			}

			pano_engine()->MoveCrossSection(visibility_param.center);

			glm::vec3 up_vector = res_cross.data().begin()->second->up_vector();
			for (auto& cs_view : base_view_cross_sections_)
			{
				cs_view.second->UpdateCrossSection();
			}

#if 0
			glm::vec3 cross = glm::cross(up_vector, visibility_param.up);
			float sign = (cross.x * cross.y * cross.z > 0.0f) ? 1.0f : -1.0f;
			int cs_index = res_cross.data().at(0)->index();
			if (cs_index < 0)
			{
				sign *= -1.0f;
			}

			float angle = sign * W3::GetAngle(up_vector, visibility_param.up);

			//task_tool_base_->UpdateCrossSectionAngle(angle);

			qDebug() << "visibility_param :" << visibility_param.up.x << visibility_param.up.y << visibility_param.up.z;
			qDebug() << "up_vector 1 :" << up_vector.x << up_vector.y << up_vector.z;
			qDebug() << sign << "(" << cross.x << cross.y << cross.z << ")" << angle;
#else
			glm::vec3 cross = glm::cross(up_vector, visibility_param.up);
			float angle = W3::GetAngle(up_vector, visibility_param.up);
#endif
			if (angle == 0.0f)
			{
				return;
			}
#if 1
			qDebug() << "up_vector 1 :" << up_vector.x << up_vector.y << up_vector.z;
			qDebug() << "angle 1 :" << angle;

			glm::mat4 rotate = glm::rotate(glm::radians(angle), cross);
			up_vector = glm::vec3(rotate * glm::vec4(up_vector, 1.0f));

			qDebug() << "difference :" << W3::GetAngle(up_vector, visibility_param.up);

			if (W3::GetAngle(up_vector, visibility_param.up) < 0.1f)
			{
				angle *= -1.0f;
			}

			qDebug() << "up_vector 2 :" << up_vector.x << up_vector.y << up_vector.z;
			qDebug() << "angle 2 :" << angle;

			task_tool_base_->UpdateCrossSectionAngle(angle);
#endif

			pano_engine()->MoveCrossSection(visibility_param.center);
			base_view_pano_->CrossSectionUpdated();
			base_view_pano_arch_->CrossSectionUpdated();
			for (auto& cs_view : base_view_cross_sections_)
			{
				cs_view.second->UpdateCrossSection();
			}

			up_vector = res_cross.data().begin()->second->up_vector();
			qDebug() << "up_vector 3 :" << up_vector.x << up_vector.y << up_vector.z;

			glm::vec3 dst_back_vector = visibility_param.back;

			glm::vec3 back_vector = res_cross.data().begin()->second->back_vector();
			glm::vec3 right_vector = res_cross.data().begin()->second->right_vector();
			qDebug() << "";
			qDebug() << back_vector.x << back_vector.y << back_vector.z;
			qDebug() << right_vector.x << right_vector.y << right_vector.z;
			qDebug() << "";
		}
}

void BasePanoViewMgr::slotToggleImplantRenderingType()
{
	pano_engine_->SetIsImplantWire(!pano_engine_->GetIsImplantWire());
	ReconImplantResource();
	base_view_pano_->UpdatedPano();
}

void BasePanoViewMgr::MoveAxialLineToPanoViewCenter()
{
	if (pano_engine_->IsValidPanorama())
	{
		base_view_pano_->MoveAxialLineToViewCenter();
	}
}

void BasePanoViewMgr::MoveAxialLineToCrossSectionViewCenter()
{
	if (pano_engine_->IsValidCrossSection())
	{
		for (int i = 0; i < cross_section_count_; i++)
		{
			base_view_cross_sections_[i]->MoveAxialLineToViewCenter();
		}
	}
}
