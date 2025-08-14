#include "tmj_view_mgr.h"

#include <Engine/Common/Common/global_preferences.h>
#include <Engine/Common/Common/W3Logger.h>
#include <Engine/Common/Common/define_measure.h>
#include <Engine/Resource/Resource/tmj_resource.h>
#ifndef WILL3D_VIEWER
#include <Engine/Core/W3ProjectIO/project_io_tmj.h>
#include <Engine/Core/W3ProjectIO/project_io_view.h>
#endif
#include <Engine/UIModule/UIComponent/view_tmj_3d.h>
#include <Engine/UIModule/UIComponent/view_tmj_axial.h>
#include <Engine/UIModule/UIComponent/view_tmj_frontal.h>
#include <Engine/UIModule/UIComponent/view_tmj_lateral.h>
#include <Engine/UIModule/UIComponent/view_tmj_orientation.h>

#include <Engine/Module/TMJ/tmj_engine.h>

TMJViewMgr::TMJViewMgr(QObject* parent) : BaseViewMgr(parent)
{
	InitializeViews();
	connections();
}
TMJViewMgr::~TMJViewMgr() {}

/**=================================================================================================
public functions
*===============================================================================================**/
#ifndef WILL3D_VIEWER
void TMJViewMgr::exportProject(ProjectIOTMJ& out)
{
	float scene_scale = 0.0f, scene_to_gl = 0.0f;
	QPointF trans_gl;

	out.InitTMJTab();
	view_axial_->exportProject(&scene_scale, &scene_to_gl, &trans_gl);
	out.InitializeView(ProjectIOTMJ::ViewType::AXIAL);
	out.GetViewIO().SaveViewInfo(scene_scale, scene_to_gl, trans_gl.x(),
		trans_gl.y());

	view_3d_[TMJDirectionType::TMJ_LEFT]->exportProject(&scene_scale,
		&scene_to_gl, &trans_gl);
	out.InitializeView(ProjectIOTMJ::ViewType::THREE_D_LEFT);
	out.GetViewIO().SaveViewInfo(scene_scale, scene_to_gl, trans_gl.x(),
		trans_gl.y());

	view_3d_[TMJDirectionType::TMJ_RIGHT]->exportProject(&scene_scale,
		&scene_to_gl, &trans_gl);
	out.InitializeView(ProjectIOTMJ::ViewType::THREE_D_RIGHT);
	out.GetViewIO().SaveViewInfo(scene_scale, scene_to_gl, trans_gl.x(),
		trans_gl.y());

	for (int i = 0; i < common::kMaxFrontalCount; ++i)
	{
		view_frontal_[TMJDirectionType::TMJ_LEFT][i]->ExportProject(
			out, &scene_scale, &scene_to_gl, &trans_gl);
		out.InitializeView(ProjectIOTMJ::ViewType::FRONTAL_LEFT, i);
		out.GetViewIO().SaveViewInfo(scene_scale, scene_to_gl, trans_gl.x(),
			trans_gl.y());

		view_frontal_[TMJDirectionType::TMJ_RIGHT][i]->ExportProject(
			out, &scene_scale, &scene_to_gl, &trans_gl);
		out.InitializeView(ProjectIOTMJ::ViewType::FRONTAL_RIGHT, i);
		out.GetViewIO().SaveViewInfo(scene_scale, scene_to_gl, trans_gl.x(),
			trans_gl.y());
	}

	for (int i = 0; i < common::kMaxLateralCount; ++i)
	{
		view_lateral_[TMJDirectionType::TMJ_LEFT][i]->exportProject(
			&scene_scale, &scene_to_gl, &trans_gl);
		out.InitializeView(ProjectIOTMJ::ViewType::LATERAL_LEFT, i);
		out.GetViewIO().SaveViewInfo(scene_scale, scene_to_gl, trans_gl.x(),
			trans_gl.y());

		view_lateral_[TMJDirectionType::TMJ_RIGHT][i]->exportProject(
			&scene_scale, &scene_to_gl, &trans_gl);
		out.InitializeView(ProjectIOTMJ::ViewType::LATERAL_RIGHT, i);
		out.GetViewIO().SaveViewInfo(scene_scale, scene_to_gl, trans_gl.x(),
			trans_gl.y());
	}
}

void TMJViewMgr::importProject(ProjectIOTMJ& in)
{
	float scene_scale = 0.0f, scene_to_gl = 0.0f;
	float trans_gl_x = 0.0f, trans_gl_y = 0.0f;

	in.InitializeView(ProjectIOTMJ::ViewType::AXIAL);
	in.GetViewIO().LoadViewInfo(scene_scale, scene_to_gl, trans_gl_x, trans_gl_y);
	view_axial_->importProject(scene_scale, scene_to_gl,
		QPointF(trans_gl_x, trans_gl_y));

	in.InitializeView(ProjectIOTMJ::ViewType::THREE_D_LEFT);
	in.GetViewIO().LoadViewInfo(scene_scale, scene_to_gl, trans_gl_x, trans_gl_y);
	view_3d_[TMJDirectionType::TMJ_LEFT]->importProject(
		scene_scale, scene_to_gl, QPointF(trans_gl_x, trans_gl_y));

	in.InitializeView(ProjectIOTMJ::ViewType::THREE_D_RIGHT);
	in.GetViewIO().LoadViewInfo(scene_scale, scene_to_gl, trans_gl_x, trans_gl_y);
	view_3d_[TMJDirectionType::TMJ_RIGHT]->importProject(
		scene_scale, scene_to_gl, QPointF(trans_gl_x, trans_gl_y));

	for (int d = 0; d < TMJDirectionType::TMJ_TYPE_END; ++d)
	{
		TMJDirectionType direction_type = static_cast<TMJDirectionType>(d);
		int frontal_cnt = frontal_count_[d];
		ProjectIOTMJ::ViewType view_type =
			direction_type == TMJDirectionType::TMJ_LEFT
			? ProjectIOTMJ::ViewType::FRONTAL_LEFT
			: ProjectIOTMJ::ViewType::FRONTAL_RIGHT;

		for (int i = 0; i < frontal_cnt; ++i)
		{
			in.InitializeView(view_type, i);
			in.GetViewIO().LoadViewInfo(scene_scale, scene_to_gl, trans_gl_x,
				trans_gl_y);
			bool is_update_resource = (i == frontal_cnt - 1) ? true : false;
			view_frontal_[d][i]->ImportProject(in, scene_scale, scene_to_gl,
				QPointF(trans_gl_x, trans_gl_y),
				is_update_resource);
		}

		int lateral_cnt = GetLateralViewCount(direction_type);
		view_type = direction_type == TMJDirectionType::TMJ_LEFT
			? ProjectIOTMJ::ViewType::LATERAL_LEFT
			: ProjectIOTMJ::ViewType::LATERAL_RIGHT;

		for (int i = 0; i < lateral_cnt; ++i)
		{
			in.InitializeView(view_type, i);
			in.GetViewIO().LoadViewInfo(scene_scale, scene_to_gl, trans_gl_x,
				trans_gl_y);
			bool is_update_resource = (i == lateral_cnt - 1) ? true : false;
			view_lateral_[d][i]->importProject(scene_scale, scene_to_gl,
				QPointF(trans_gl_x, trans_gl_y), false,
				is_update_resource);
		}
	}

	const auto& tmj_roi = tmj_engine_->GetTmjROI();
	view_axial_->SetSliceRange(tmj_roi.top, tmj_roi.bottom);
	view_axial_->SetSliceInVol(tmj_roi.slice);
	glm::mat4 reorien_mat = tmj_engine_->reorien();
	view_axial_->ForceRotateMatrix(reorien_mat);
	for (int id = 0; id < ReorientViewID::REORI_VIEW_END; ++id)
	{
		view_orient_[id]->blockSignals(true);
		view_orient_[id]->SetRotateMatrix(reorien_mat);
		view_orient_[id]->blockSignals(false);
	}

	view_orient_[ReorientViewID::ORIENT_R]->ReadyROILines(
		tmj_roi.top, tmj_roi.slice, tmj_roi.bottom);
	// set slice range tmj_roi.slice
	// set reorientation matrix
}
#endif
void TMJViewMgr::MoveViewsToSelectedMeasure(const common::ViewTypeID& view_type,
	const unsigned int& measure_id)
{
	common::measure::VisibilityParams visibility_params;
	GetMeasureParamsInView(view_type, measure_id, &visibility_params);

	switch (view_type)
	{
	case common::ViewTypeID::TMJ_FRONTAL_LEFT:
		MoveFrontalViewToSelectedMeasure(TMJDirectionType::TMJ_LEFT,
			visibility_params);
		break;
	case common::ViewTypeID::TMJ_FRONTAL_RIGHT:
		MoveFrontalViewToSelectedMeasure(TMJDirectionType::TMJ_RIGHT,
			visibility_params);
		break;
	case common::ViewTypeID::TMJ_LATERAL_LEFT:
		MoveLateralViewToSelectedMeasure(TMJDirectionType::TMJ_LEFT,
			visibility_params);
		break;
	case common::ViewTypeID::TMJ_LATERAL_RIGHT:
		MoveLateralViewToSelectedMeasure(TMJDirectionType::TMJ_RIGHT,
			visibility_params);
		break;
	case common::ViewTypeID::TMJ_ARCH:
		MoveArchViewToSelectedMeasure(visibility_params);
		break;
	default:
		common::Logger::instance()->PrintAndAssert(
			common::LogType::ERR,
			"PanoViewMgr::MoveViewsToSelectedMeasure : invalid view type id");
		break;
	}
}

void TMJViewMgr::SyncTMJItemVisibleUI()
{
	for (int i = 0; i < TMJDirectionType::TMJ_TYPE_END; i++)
	{
		TMJDirectionType type = (TMJDirectionType)i;
		bool is_visible_lateral = view_lateral_[i][0]->isVisible();
		bool is_visible_frontal = view_frontal_[i][0]->isVisible();

		view_axial_->SetVisibleTMJitemLateralLine(type, is_visible_lateral);
		view_axial_->SetVisibleTMJitemFrontalLine(type, is_visible_frontal);

		for (int view_id = 0; view_id < common::kMaxFrontalCount; ++view_id)
			view_frontal_[type][view_id]->SetLateralVisible(is_visible_lateral);
	}
}

void TMJViewMgr::SetTMJengine(const std::shared_ptr<TMJengine>& tmj_engine)
{
	tmj_engine_ = tmj_engine;

	if (tmj_engine_->initialized())
	{
		const TMJengine::TmjROI& roi = tmj_engine_->GetTmjROI();

		int min, max;
		view_axial_->GetSliceRange(&min, &max);
		if ((int)roi.top != min || (int)roi.bottom != max)
			view_axial_->SetSliceRange(roi.top, roi.bottom);

		if (view_axial_->GetSliceInVol() != (int)roi.slice)
			view_axial_->SetSliceInVol(roi.slice);

		tmj_engine_->SetTMJROISlice(roi.slice);
		tmj_engine_->SetTMJROITop(roi.top);
		tmj_engine_->SetTMJROIBottom(roi.bottom);

		view_orient_[ReorientViewID::ORIENT_R]->ReadyROILines(roi.top, roi.slice,
			roi.bottom);
	}
}

void TMJViewMgr::SelectLayoutLateral(const TMJDirectionType& type,
	const int& row, const int& column)
{
	if (lateral_layout_shape_[type].row == row &&
		lateral_layout_shape_[type].col == column)
		return;

	lateral_layout_shape_[type].row = row;
	lateral_layout_shape_[type].col = column;
	int lateral_count = GetLateralViewCount(type);

	for (int i = 0; i < common::kMaxLateralCount; i++)
	{
		view_lateral_[type][i]->SetEnabledSharpenUI(false);
	}

	view_lateral_[type][column - 1]->SetEnabledSharpenUI(true);

	for (int view_id = 0; view_id < common::kMaxFrontalCount; ++view_id)
	{
		view_frontal_[type][view_id]->ClearLateralLine();
	}

	view_axial_->SetTMJlateralCount(type, lateral_count);

	task_tool_->SetFrontalWidthLowerBound(static_cast<TMJDirectionType>(type),
		lateral_count);
}

void TMJViewMgr::SelectLayoutFrontal(const TMJDirectionType& type,
	const int& count)
{
	frontal_count_[type] = count;

	for (int i = 0; i < common::kMaxFrontalCount; ++i)
		view_frontal_[type][i]->SetEnabledSharpenUI(false);

	view_frontal_[type][count - 1]->SetEnabledSharpenUI(true);
}

void TMJViewMgr::set_task_tool(const std::shared_ptr<TMJTaskTool>& task_tool)
{
	task_tool_ = task_tool;
	ConnectTMJMenus();
}

QWidget* TMJViewMgr::GetViewAxial() const
{
	return (QWidget*)(view_axial_.get());
}
QWidget* TMJViewMgr::GetViewOrient(const ReorientViewID& id) const
{
	return (QWidget*)(view_orient_[id].get());
}

QWidget* TMJViewMgr::GetViewFrontal3DWidget(
	const TMJDirectionType& type) const
{
	return (QWidget*)(view_3d_[type].get());
}

std::vector<QWidget*> TMJViewMgr::GetViewFrontalWidget(
	const TMJDirectionType& type) const
{
	std::vector<QWidget*> widgets;
	widgets.reserve(common::kMaxFrontalCount);
	for (int i = 0; i < common::kMaxFrontalCount; i++)
		widgets.push_back((QWidget*)(view_frontal_[type][i].get()));

	return widgets;
}

std::vector<QWidget*> TMJViewMgr::GetViewLateralWidget(
	const TMJDirectionType& type) const
{
	std::vector<QWidget*> widgets;
	widgets.reserve(common::kMaxLateralCount);
	for (int i = 0; i < common::kMaxLateralCount; i++)
		widgets.push_back((QWidget*)(view_lateral_[type][i].get()));

	return widgets;
}

/**=================================================================================================
Private functions
*===============================================================================================**/

void TMJViewMgr::InitializeViews()
{
	// create axial view
	view_axial_.reset(new ViewTMJaxial());
	view_axial_->setVisible(false);
	BaseViewMgr::SetCastedView(view_axial_->view_id(),
		std::static_pointer_cast<View>(view_axial_));

	// create orientation views
	for (int id = 0; id < ReorientViewID::REORI_VIEW_END; ++id)
	{
		ReorientViewID view_type = static_cast<ReorientViewID>(id);
		view_orient_[id].reset(new ViewTMJorientation(view_type));
		view_orient_[id]->setVisible(false);
		BaseViewMgr::SetCastedView(
			view_orient_[id]->view_id(),
			std::static_pointer_cast<View>(view_orient_[id]));
	}

	for (int direction = 0; direction < TMJ_TYPE_END; direction++)
	{
		const TMJDirectionType direction_type =
			static_cast<TMJDirectionType>(direction);
		common::ViewTypeID view_type = (direction == TMJ_LEFT)
			? common::ViewTypeID::TMJ_FRONTAL_LEFT
			: common::ViewTypeID::TMJ_FRONTAL_RIGHT;

		// create frontal views
		for (int i = 0; i < common::kMaxFrontalCount; i++)
		{
			view_frontal_[direction_type][i].reset(
				new ViewTmjFrontal(i, view_type, direction_type));
			view_frontal_[direction_type][i]->setVisible(false);

			BaseViewMgr::SetCastedView(
				view_frontal_[direction_type][i]->view_id(),
				std::static_pointer_cast<View>(view_frontal_[direction_type][i]));
		}
		// create frontal 3d views
		view_3d_[direction].reset(new ViewTMJ3D(direction_type));
		view_3d_[direction]->setVisible(false);

		BaseViewMgr::SetCastedView(
			view_3d_[direction]->view_id(),
			std::static_pointer_cast<View>(view_3d_[direction]));

		// create lateral views
		for (int view_id = 0; view_id < common::kMaxLateralCount; view_id++)
		{
			common::ViewTypeID view_type =
				(direction == TMJ_LEFT) ? common::ViewTypeID::TMJ_LATERAL_LEFT
				: common::ViewTypeID::TMJ_LATERAL_RIGHT;

			view_lateral_[direction][view_id].reset(
				new ViewTmjLateral(view_id, view_type, direction_type));
			view_lateral_[direction][view_id]->setVisible(false);

			BaseViewMgr::SetCastedView(
				view_lateral_[direction][view_id]->view_id(),
				std::static_pointer_cast<View>(view_lateral_[direction][view_id]));
		}
	}
}

void TMJViewMgr::connections()
{
	connect(view_orient_[ReorientViewID::ORIENT_R].get(), SIGNAL(sigChangedROI()), this, SLOT(slotChangeROIFromOrienView()));
	connect(view_orient_[ReorientViewID::ORIENT_R].get(), SIGNAL(sigChangedSlice()), this, SLOT(slotChangeSliceFromOrienView()));

	for (int id = 0; id < ReorientViewID::REORI_VIEW_END; ++id)
	{
		const auto& view_orient = view_orient_[id].get();
		connect(view_orient, SIGNAL(sigRotateMat(ReorientViewID, glm::mat4)), this, SLOT(slotOrientationViewRotate(ReorientViewID, glm::mat4)));
		connect(view_orient, &ViewTMJorientation::sigRenderQuality, this, &TMJViewMgr::slotOrientationViewRenderQuality);
		connect(view_orient, &ViewTMJorientation::sigUpdateDoneROI, this, &TMJViewMgr::slotUpdateDoneROIFromOrienView);
	}

	for (int direction_type = 0; direction_type < TMJDirectionType::TMJ_TYPE_END; ++direction_type)
	{
		for (int view_id = 0; view_id < common::kMaxLateralCount; ++view_id)
		{
			const auto& view_lateral = view_lateral_[direction_type][view_id].get();
			connect(view_lateral, &ViewTmjLateral::sigSetSharpen, [=](SharpenLevel level)
			{
				slotTMJLateralSharpen(static_cast<TMJDirectionType>(direction_type), level);
			});

			connect(view_lateral, &ViewTmjLateral::sigWindowingDone, this, &TMJViewMgr::slotSyncWindowing);
			connect(view_lateral, &ViewTmjLateral::sigZoomDone, this, &TMJViewMgr::slotSyncLateralZoom);
			connect(view_lateral, &ViewTmjLateral::sigPanDone, this, &TMJViewMgr::slotSyncLateralPan);
			connect(view_lateral, &ViewTmjLateral::sigWheelEvent, this, &TMJViewMgr::slotWheelEventFromLateralView);
			connect(view_lateral, &ViewTmjLateral::sigLateralViewSelect, this, &TMJViewMgr::slotLateralViewSelected);
			connect(view_lateral, &ViewTmjLateral::sigSetFrontalSlice, this, &TMJViewMgr::slotSetFrontalSliceFromLateralView);
			connect(view_lateral, &ViewTmjLateral::sigSetAxialSlice, this, &TMJViewMgr::slotSetAxialZ);
			connect(view_lateral, &ViewTmjLateral::sigMeasureCreated, this, &TMJViewMgr::slotLateralMeasureCreated);
			connect(view_lateral, &ViewTmjLateral::sigMeasureDeleted, this, &TMJViewMgr::slotLateralMeasureDeleted);
			connect(view_lateral, &ViewTmjLateral::sigMeasureModified, this, &TMJViewMgr::slotLateralMeasureModified);
			connect(view_lateral, &ViewTmjLateral::sigRequestSyncViewStatus, this, &TMJViewMgr::slotSetLateralViewStatus);
		}

		for (int view_id = 0; view_id < common::kMaxFrontalCount; ++view_id)
		{
			const auto& view_frontal = view_frontal_[direction_type][view_id].get();
			connect(view_frontal, &ViewTmjFrontal::sigWindowingDone, this, &TMJViewMgr::slotSyncWindowing);
			connect(view_frontal, &ViewTmjFrontal::sigZoomDone, this, &TMJViewMgr::slotSyncFrontalZoom);
			connect(view_frontal, &ViewTmjFrontal::sigPanDone, this, &TMJViewMgr::slotSyncFrontalPan);
			connect(view_frontal, &ViewTmjFrontal::sigWheelEvent, this, &TMJViewMgr::slotWheelEventFromFrontalView);
			connect(view_frontal, &ViewTmjFrontal::sigSetLateralSlice, this, &TMJViewMgr::slotSetLateralSliceFromFrontalView);
			connect(view_frontal, &ViewTmjFrontal::sigSetAxialSlice, this, &TMJViewMgr::slotSetAxialZ);
			connect(view_frontal, &ViewTmjFrontal::sigSetSharpen, [=](SharpenLevel level)
			{
				slotTMJFrontalSharpen(static_cast<TMJDirectionType>(direction_type), level);
			});

			connect(view_frontal, &ViewTmjFrontal::sigMeasureCreated, this, &TMJViewMgr::slotFrontalMeasureCreated);
			connect(view_frontal, &ViewTmjFrontal::sigMeasureDeleted, this, &TMJViewMgr::slotFrontalMeasureDeleted);
			connect(view_frontal, &ViewTmjFrontal::sigMeasureModified, this, &TMJViewMgr::slotFrontalMeasureModified);
			connect(view_frontal, &ViewTmjFrontal::sigRequestSyncViewStatus, this, &TMJViewMgr::slotSetFrontalViewStatus);
		}

		// connections for VR Cut
		const auto& view_frontal = view_frontal_[direction_type][0].get();
		connect(view_frontal, &ViewTmjFrontal::sigCut, this, &TMJViewMgr::slotTMJCutFrontal);
	}

	const auto& view_axial = view_axial_.get();
	connect(view_axial, &ViewTMJaxial::sigTranslateZ, this, &TMJViewMgr::slotAxialPositionChanged);
	connect(view_axial, &ViewTMJaxial::sigRequestInitialize, this, &TMJViewMgr::slotRequestInitializeFromAxial);
	connect(view_axial, &ViewTMJaxial::sigUpdateTMJ, this, &TMJViewMgr::slotUpdateTMJ);
	connect(view_axial, &ViewTMJaxial::sigWindowingDone, this, &TMJViewMgr::slotSyncWindowing);

#ifdef WILL3D_EUROPE
	connect(view_axial_.get(), &ViewTMJaxial::sigShowButtonListDialog, this, &TMJViewMgr::sigShowButtonListDialog);
	for (int i = 0; i < TMJDirectionType::TMJ_TYPE_END; ++i)
	{
		for (int j = 0; j < common::kMaxFrontalCount; ++j)
		{
			connect(view_frontal_[i][j].get(), &ViewTmjFrontal::sigShowButtonListDialog, this, &TMJViewMgr::sigShowButtonListDialog);
		}
		for (int j = 0; j < common::kMaxLateralCount; ++j)
		{
			connect(view_lateral_[i][j].get(), &ViewTmjLateral::sigShowButtonListDialog, this, &TMJViewMgr::sigShowButtonListDialog);
		}
		connect(view_3d_[i].get(), &ViewTMJ3D::sigShowButtonListDialog, this, &TMJViewMgr::sigShowButtonListDialog);
	}

	connect(view_axial_.get(), &ViewTMJaxial::sigSyncControlButton, [=](bool is_on)
	{
		SetAllSyncControlButton(is_on);
	});

	for (int i = 0; i < TMJDirectionType::TMJ_TYPE_END; ++i)
	{
		for (int j = 0; j < common::kMaxFrontalCount; ++j)
		{
			connect(view_frontal_[i][j].get(), &ViewTmjFrontal::sigSyncControlButton, [=](bool is_on)
			{
				SetAllSyncControlButton(is_on);
			});
		}

		for (int j = 0; j < common::kMaxLateralCount; ++j)
		{
			connect(view_lateral_[i][j].get(), &ViewTmjLateral::sigSyncControlButton, [=](bool is_on)
			{
				SetAllSyncControlButton(is_on);
			});
		}

		connect(view_3d_[i].get(), &ViewTMJ3D::sigSyncControlButton, [=](bool is_on)
		{
			SetAllSyncControlButton(is_on); 
		});
	}

#endif // WILL3D_EUROPE
}

void TMJViewMgr::ConnectTMJMenus()
{
	task_tool_->Connections();

	// view와 연결된 signal은 view mgr에서 connection
	const auto& task_tool = task_tool_.get();
	connect(task_tool, &TMJTaskTool::sigTMJOrientRotate, this,
		&TMJViewMgr::slotOrientationFromTaskTool);
	connect(task_tool, &TMJTaskTool::sigTMJDrawRect, view_axial_.get(),
		&ViewTMJaxial::slotROIRectDraw);
	connect(task_tool, &TMJTaskTool::sigTMJCutEnable, this,
		&TMJViewMgr::slotTMJCutEnable);
	connect(task_tool, &TMJTaskTool::sigTMJClipParamsChanged, this,
		&TMJViewMgr::slotTMJClipParamsChange);
	connect(task_tool, &TMJTaskTool::sigTMJRectChanged, view_axial_.get(),
		&ViewTMJaxial::slotTMJRectSizeChanged);
	connect(task_tool, &TMJTaskTool::sigTMJLateralParamChanged, this,
		&TMJViewMgr::slotLateralParamChanged);

	connect(view_axial_.get(), &ViewTMJaxial::sigROIRectCreated, task_tool,
		&TMJTaskTool::DrawRectDone);

	connect(task_tool, &TMJTaskTool::sigTMJCutReset, this,
		&TMJViewMgr::slotTMJCutReset);
	connect(task_tool, &TMJTaskTool::sigTMJCutUndo, this,
		&TMJViewMgr::slotTMJCutUndo);
	connect(task_tool, &TMJTaskTool::sigTMJCutRedo, this,
		&TMJViewMgr::slotTMJCutRedo);
}

void TMJViewMgr::MoveFrontalViewToSelectedMeasure(
	const TMJDirectionType& type,
	const common::measure::VisibilityParams& visibility_param)
{
	TMJDirectionType dir_type = static_cast<TMJDirectionType>(type);

	glm::vec3 pt_frontal = tmj_engine_->GetFrontal(dir_type).center_position();
	TranslateFrontal(dir_type, visibility_param.center, pt_frontal);
}

void TMJViewMgr::MoveLateralViewToSelectedMeasure(
	const TMJDirectionType& type,
	const common::measure::VisibilityParams& visibility_param)
{
	TMJDirectionType dir_type = static_cast<TMJDirectionType>(type);

	glm::vec3 pt_lateral =
		tmj_engine_->GetLateral(dir_type).center_positions().front();
	TranslateLateral(dir_type, visibility_param.center, pt_lateral);
}

void TMJViewMgr::MoveArchViewToSelectedMeasure(
	const common::measure::VisibilityParams& visibility_param)
{
	glm::vec3 curr_center = view_axial_->GetCenterPosition();
	glm::vec3 curr_up_vector = view_axial_->GetUpVector();

	glm::vec3 curr_center_to_moved_center = visibility_param.center - curr_center;
	float distance = glm::dot(curr_center_to_moved_center, curr_up_vector);
	view_axial_->SetSliceInVol(view_axial_->GetSliceInVol() + distance);
}
/**=================================================================================================
Private slots
*===============================================================================================**/
void TMJViewMgr::slotSyncWindowing()
{
	view_axial_->UpdateSlice();

	for (int type = 0; type < TMJDirectionType::TMJ_TYPE_END; ++type)
	{
		for (int i = 0;
			i < GetLateralViewCount(static_cast<TMJDirectionType>(type)); i++)
		{
			view_lateral_[type][i]->UpdateLateral();
		}
		for (int i = 0; i < frontal_count_[type]; i++)
		{
			view_frontal_[type][i]->UpdateFrontal();
		}
	}
}

void TMJViewMgr::slotSyncLateralZoom(const float& scene_scale)
{
	QObject* sender = QObject::sender();
	for (int type = 0; type < TMJDirectionType::TMJ_TYPE_END; ++type)
	{
		for (int view_id = 0; view_id < common::kMaxLateralCount; view_id++)
		{
			if (view_lateral_[type][view_id].get() == sender) continue;

			view_lateral_[type][view_id]->SetZoomScale(scene_scale);
		}
	}
}

void TMJViewMgr::slotSyncLateralPan(const QPointF& trans)
{
	QObject* sender = QObject::sender();
	for (int type = 0; type < TMJDirectionType::TMJ_TYPE_END; ++type)
	{
		for (int view_id = 0; view_id < common::kMaxLateralCount; ++view_id)
		{
			if (view_lateral_[type][view_id].get() == sender) continue;

			view_lateral_[type][view_id]->SetPanTranslate(trans);
		}
	}
}

void TMJViewMgr::slotSyncFrontalZoom(const float& scene_scale)
{
	QObject* sender = QObject::sender();
	for (int type = 0; type < TMJDirectionType::TMJ_TYPE_END; ++type)
	{
		for (int view_id = 0; view_id < common::kMaxFrontalCount; view_id++)
		{
			if (view_frontal_[type][view_id].get() == sender) continue;

			view_frontal_[type][view_id]->SetZoomScale(scene_scale);
		}
	}
}

void TMJViewMgr::slotSyncFrontalPan(const QPointF& trans)
{
	QObject* sender = QObject::sender();
	for (int type = 0; type < TMJDirectionType::TMJ_TYPE_END; ++type)
	{
		for (int view_id = 0; view_id < common::kMaxFrontalCount; ++view_id)
		{
			if (view_frontal_[type][view_id].get() == sender) continue;

			view_frontal_[type][view_id]->SetPanTranslate(trans);
		}
	}
}

void TMJViewMgr::UpdateTMJ()
{
	for (int dir_type = 0; dir_type < TMJDirectionType::TMJ_TYPE_END;
		dir_type++)
	{
		slotUpdateTMJ((TMJDirectionType)dir_type);
	}
}

void TMJViewMgr::UpdateResourceLateral(const TMJDirectionType& type,
	bool* is_update_lateral,
	bool* is_update_cut,
	bool* is_update_rect)
{
	// position
	bool is_update = false;
	std::map<int, glm::vec3> pts_lateral_center_in_vol;
	glm::vec3 up_vector_lateral;
	if (!view_axial_->GetLateralPositionInfo(type, &pts_lateral_center_in_vol,
		&up_vector_lateral))
	{
		if (tmj_engine_->DeleteLateralResource(type))
		{
			*is_update_lateral |= true;
			return;
		}
	}

	const auto& tmj_roi = tmj_engine_->GetTmjROI();
	const float scalar_slice_to_mid =
		tmj_roi.slice - (tmj_roi.top + tmj_roi.bottom) * 0.5f - 0.5f;
	const glm::vec3 up_vector = view_axial_->GetUpVector();
	const glm::vec3 up_dist = scalar_slice_to_mid * up_vector;

	for (auto& elem : pts_lateral_center_in_vol)
	{
		elem.second = (elem.second - up_dist);
	}

	tmj_engine_->ReadyLateralResource(type);

	if (tmj_engine_->SetLateralPositionInfo(type, pts_lateral_center_in_vol,
		up_vector_lateral))
		*is_update_lateral |= true;

	// size
	float width, height;
	view_axial_->GetTMJSizeInfo(type, &width, &height);
	if (tmj_engine_->SetTMJRectHeight(type, height))
	{
		if (tmj_engine_->ResetCutTMJ(type)) *is_update_cut |= true;
		*is_update_lateral |= true;
		*is_update_rect |= true;
	}

	// params(thickness, interval, etc)
	for (int i = 0; i < TMJLateralID::TMJ_LATERAL_END; i++)
	{
		float value;
		view_axial_->GetLateralParam((TMJLateralID)i, &value);

		if (tmj_engine_->SetLateralParam((TMJLateralID)i, value))
		{
			*is_update_lateral |= true;
		}
	}
}

void TMJViewMgr::UpdateResourceFrontal(const TMJDirectionType& type,
	bool* is_update_frontal,
	bool* is_update_cut,
	bool* is_update_rect)
{
	// position
	glm::vec3 pt_frontal_center_in_vol;
	glm::vec3 up_vector_frontal;
	if (!view_axial_->GetFrontalPositionInfo(type, &pt_frontal_center_in_vol,
		&up_vector_frontal))
	{
		if (tmj_engine_->DeleteFrontalResource(type))
		{
			*is_update_frontal |= true;
			return;
		}
	}

	const auto& tmj_roi = tmj_engine_->GetTmjROI();
	const float scalar_slice_to_mid =
		tmj_roi.slice - (tmj_roi.top + tmj_roi.bottom) * 0.5f - 0.5f;
	const glm::vec3 up_vector = view_axial_->GetUpVector();
	const glm::vec3 up_dist = scalar_slice_to_mid * up_vector;

	pt_frontal_center_in_vol -= up_dist;

	tmj_engine_->ReadyFrontalResource(type);

	if (tmj_engine_->SetFrontalPositionInfo(type, pt_frontal_center_in_vol,
		up_vector_frontal))
		*is_update_frontal |= true;

	// size
	float width, height;
	view_axial_->GetTMJSizeInfo(type, &width, &height);
	if (tmj_engine_->SetTMJRectWidth(type, width))
	{
		if (tmj_engine_->ResetCutTMJ(type)) *is_update_cut |= true;
		*is_update_frontal |= true;
		*is_update_rect |= true;
	}

	// rect center position
	glm::vec3 pt_rect_center;
	if (view_axial_->GetTMJRectCenter(type, &pt_rect_center))
	{
		if (tmj_engine_->SetTMJRectCenter(type, pt_rect_center))
		{
			if (tmj_engine_->ResetCutTMJ(type)) *is_update_cut |= true;
			pt_rect_center -= up_dist;
			*is_update_rect |= true;
		}
	}
}

void TMJViewMgr::SyncTMJparamsFromUI()
{
	const float base_pixel_spacing = tmj_engine_->GetBasePixelSpacing();

	for (int i = 0; i < TMJDirectionType::TMJ_TYPE_END; i++)
	{
		tmj_engine_->ReadyFrontalResource((TMJDirectionType)i);
		tmj_engine_->ReadyLateralResource((TMJDirectionType)i);
	}

	for (int i = 0; i < TMJLateralID::TMJ_LATERAL_END; i++)
	{
		tmj_engine_->SetLateralParam(
			(TMJLateralID)i,
			task_tool_->GetLateralParam((TMJLateralID)i) / base_pixel_spacing);
	}
	for (int i = 0; i < TMJRectID::TMJ_RECT_END; i++)
	{
		tmj_engine_->SetTMJRectParam(
			(TMJRectID)i,
			task_tool_->GetTMJRectparam((TMJRectID)i) / base_pixel_spacing);
	}

	const float top = view_orient_[ReorientViewID::ORIENT_R]->GetTopInVol();
	const float bottom = view_orient_[ReorientViewID::ORIENT_R]->GetBottomInVol();

	for (int i = 0; i < TMJDirectionType::TMJ_TYPE_END; i++)
	{
		TMJDirectionType type = static_cast<TMJDirectionType>(i);
		tmj_engine_->SetLatertalCount(type, GetLateralViewCount(type));
	}
}

void TMJViewMgr::SyncTMJRectSizeUI()
{
	const float base_pixel_spacing = tmj_engine_->GetBasePixelSpacing();
	for (int i = 0; i < TMJDirectionType::TMJ_TYPE_END; i++)
	{
		const TMJDirectionType type = (TMJDirectionType)i;
		float width, height;
		const bool res = tmj_engine_->GetTMJRectSize(type, &width, &height);
		if (res)
		{
			width = width * base_pixel_spacing;
			height = height * base_pixel_spacing;

			task_tool_->SetFrontalWidthLowerBound(type, GetLateralViewCount(type));
		}
		else
		{
			width = height = 0.0f;
			task_tool_->SetFrontalWidthLowerBound(type, 0);
		}

		task_tool_->SyncFrontalWidth(type, width);
		task_tool_->SyncLateralWidth(type, height);
	}
}

void TMJViewMgr::UpdateAxialLines()
{
	for (int type = 0; type < TMJDirectionType::TMJ_TYPE_END; ++type)
	{
		for (int view_id = 0; view_id < frontal_count_[type]; view_id++)
		{
			view_frontal_[type][view_id]->UpdateUI();
		}

		TMJDirectionType d_type = static_cast<TMJDirectionType>(type);
		for (int view_id = 0; view_id < GetLateralViewCount(d_type); ++view_id)
		{
			view_lateral_[type][view_id]->UpdateUI();
		}
	}
}

void TMJViewMgr::UpdateFrontalLines() {}

int TMJViewMgr::GetLateralViewCount(const TMJDirectionType& type)
{
	return lateral_layout_shape_[type].row * lateral_layout_shape_[type].col;
}

void TMJViewMgr::DeleteMeasuresInTMJRectViews(const TMJDirectionType& type)
{
	for (int i = 0; i < common::kMaxFrontalCount; ++i)
	{
		view_frontal_[type][i]->SetCommonToolOnce(
			common::CommonToolTypeOnce::M_DEL_ALL, true);
	}

	for (int i = 0; i < common::kMaxLateralCount; ++i)
	{
		view_lateral_[type][i]->SetCommonToolOnce(
			common::CommonToolTypeOnce::M_DEL_ALL, true);
	}
}

void TMJViewMgr::TranslateLateral(const TMJDirectionType& type,
	const glm::vec3& pt_center_translated,
	const glm::vec3& pt_center_current)
{
	TMJDirectionType dir_type = static_cast<TMJDirectionType>(type);
	glm::vec3 up_vector = tmj_engine_->GetLateral(dir_type).up_vector();
	if (type == TMJDirectionType::TMJ_RIGHT) up_vector *= glm::vec3(-1.0f);
	float delta = glm::dot(up_vector, pt_center_translated - pt_center_current);

	view_axial_->TranslateLateral(static_cast<TMJDirectionType>(type), delta);
}
void TMJViewMgr::TranslateFrontal(const TMJDirectionType& type,
	const glm::vec3& pt_center_translated,
	const glm::vec3& pt_center_current)
{
	TMJDirectionType dir_type = static_cast<TMJDirectionType>(type);
	glm::vec3 up_vector = tmj_engine_->GetFrontal(dir_type).up_vector();
	float delta = glm::dot(up_vector, pt_center_translated - pt_center_current);

	view_axial_->TranslateFrontal(static_cast<TMJDirectionType>(type), delta);
}

#ifdef WILL3D_EUROPE
void TMJViewMgr::SetAllSyncControlButton(const bool is_on)
{
	view_axial_.get()->SetSyncControlButton(is_on);

	for (int i = 0; i < TMJDirectionType::TMJ_TYPE_END; ++i)
	{
		for (int j = 0; j < common::kMaxFrontalCount; ++j)
		{
			view_frontal_[i][j].get()->SetSyncControlButton(is_on);
		}
		for (int j = 0; j < common::kMaxLateralCount; ++j)
		{
			view_lateral_[i][j].get()->SetSyncControlButton(is_on);
		}
		view_3d_[i].get()->SetSyncControlButton(is_on);
	}
}
#endif // WILL3D_EUROPE

void TMJViewMgr::slotAxialPositionChanged()
{
	tmj_engine_->SetTMJROISlice(view_axial_->GetSliceInVol());
	tmj_engine_->SetAxialCenterPosition(view_axial_->GetCenterPosition());
	this->UpdateAxialLines();
}

void TMJViewMgr::slotRequestInitializeFromAxial()
{
	SyncTMJparamsFromUI();

	tmj_engine_->SetAxialCenterPosition(view_axial_->GetCenterPosition());
	tmj_engine_->SetBackVector(view_axial_->GetUpVector());

	view_axial_->InitTMJitems();
	this->UpdateAxialLines();

	for (int i = 0; i < TMJDirectionType::TMJ_TYPE_END; ++i)
		slotLateralViewSelected(static_cast<TMJDirectionType>(i), 0);
}

void TMJViewMgr::slotUpdateTMJ(const TMJDirectionType& type)
{
	bool is_update_lateral = false, is_update_frontal = false,
		is_update_cut = false, is_update_rect = false;

	this->UpdateResourceLateral(type, &is_update_lateral, &is_update_cut,
		&is_update_rect);
	this->UpdateResourceFrontal(type, &is_update_frontal, &is_update_cut,
		&is_update_rect);

	int lateral_count = GetLateralViewCount(type);
	if (is_update_lateral)
	{
		for (int i = 0; i < lateral_count; i++)
		{
			view_lateral_[type][i]->UpdateLateral();
		}
		for (int i = 0; i < common::kMaxFrontalCount; i++)
		{
			view_frontal_[type][i]->UpdateUI();
		}
	}

	if (is_update_frontal)
	{
		for (int i = 0; i < common::kMaxFrontalCount; i++)
		{
			view_frontal_[type][i]->UpdateFrontal();
		}

		for (int i = 0; i < lateral_count; i++)
		{
			view_lateral_[type][i]->UpdateUI();
		}
	}

	if (is_update_frontal || is_update_lateral)
	{
		view_3d_[type]->UpdateFrontal();
	}

	if (is_update_rect) DeleteMeasuresInTMJRectViews(type);

	this->SyncTMJRectSizeUI();
	if (is_update_cut)
	{
		view_3d_[type]->UpdateCutting(tmj_engine_->GetCutCurrStep(type));
		view_frontal_[type][0]->ResetTMJCutUI();
	}
}

void TMJViewMgr::slotOrientationViewRotate(const ReorientViewID& view_type,
	const glm::mat4& orien)
{
	for (int curr_id = 0; curr_id < ReorientViewID::REORI_VIEW_END; ++curr_id)
	{
		if (curr_id == view_type)
		{
			int degree_view = view_orient_[view_type]->GetAngleDegree();
			task_tool_->SyncOrientDegreeUIOnly(view_type, degree_view);
		}
		else
		{
			view_orient_[curr_id]->SetRotateMatrix(orien);
		}
	}
	view_axial_->ForceRotateMatrix(orien);
	tmj_engine_->set_reorien(orien);
	tmj_engine_->SetBackVector(view_axial_->GetUpVector());
	tmj_engine_->SetAxialCenterPosition(view_axial_->GetCenterPosition());

	view_axial_->SetCommonToolOnce(common::CommonToolTypeOnce::M_DEL_ALL, true);
}

void TMJViewMgr::slotOrientationViewRenderQuality(
	const ReorientViewID& view_type)
{
	for (int id = 0; id < ReorientViewID::REORI_VIEW_END; ++id)
	{
		if (id == view_type) continue;
		view_orient_[id]->SetRenderQuality();
	}
}

void TMJViewMgr::slotOrientationFromTaskTool(const ReorientViewID& view_type,
	int angle)
{
	view_orient_[view_type]->SetOrientationAngle(angle);
}

void TMJViewMgr::slotChangeROIFromOrienView()
{
	float top = view_orient_[ReorientViewID::ORIENT_R]->GetTopInVol();
	float bot = view_orient_[ReorientViewID::ORIENT_R]->GetBottomInVol();
	tmj_engine_->SetTMJROITop(top);
	tmj_engine_->SetTMJROIBottom(bot);
	view_axial_->SetSliceRange(top, bot);
	UpdateTMJ();
	for (int i = 0; i < TMJDirectionType::TMJ_TYPE_END; ++i)
	{
		DeleteMeasuresInTMJRectViews(static_cast<TMJDirectionType>(i));
	}
}

void TMJViewMgr::slotUpdateDoneROIFromOrienView()
{
	tmj_engine_->SetBackVector(view_axial_->GetUpVector());

	float top = view_orient_[ReorientViewID::ORIENT_R]->GetTopInVol();
	float bot = view_orient_[ReorientViewID::ORIENT_R]->GetBottomInVol();
	tmj_engine_->SetTMJROITop(top);
	tmj_engine_->SetTMJROIBottom(bot);
	view_axial_->SetSliceRange(top, bot);
	UpdateTMJ();
}

void TMJViewMgr::slotChangeSliceFromOrienView()
{
	float slice = view_orient_[ReorientViewID::ORIENT_R]->GetSliceInVol();
	view_axial_->SetSliceInVol(slice);
}

void TMJViewMgr::DeleteTMJROIRect(const TMJDirectionType& type)
{
	view_axial_->DeleteROIRectUI(type);
	view_frontal_[type][0]->ResetTMJCutUI();
	DeleteMeasuresInTMJRectViews(type);
}

void TMJViewMgr::ResetOrientationParams()
{
	task_tool_->ResetOrientDegreesUI();

	for (int id = 0; id < ReorientViewID::REORI_VIEW_END; ++id)
		view_orient_[id]->SetOrientationAngle(0);

	for (int id = 0; id < ReorientViewID::REORI_VIEW_END; ++id)
		view_orient_[id]->SetRotateMatrix(glm::mat4(1.0f));

	for (int id = 0; id < ReorientViewID::REORI_VIEW_END; ++id)
		view_orient_[id]->SetRenderQuality();

	view_axial_->ForceRotateMatrix(glm::mat4(1.0f));
	tmj_engine_->set_reorien(glm::mat4(1.0f));

	tmj_engine_->SetBackVector(view_axial_->GetUpVector());
	tmj_engine_->SetAxialCenterPosition(view_axial_->GetCenterPosition());

	UpdateTMJ();
}

void TMJViewMgr::GridOnOffOrientation(const bool& on)
{
	for (int id = 0; id < ReorientViewID::REORI_VIEW_END; ++id)
		view_orient_[id]->SetGridOnOff(on);
}

void TMJViewMgr::slotTMJCutEnable(const bool& cut_on,
	const VRCutTool& cut_tool)
{
	for (int id = 0; id < TMJDirectionType::TMJ_TYPE_END; ++id)
	{
		view_frontal_[id][0]->SetTMJCutMode(cut_on, cut_tool);
		view_3d_[id]->SetCutting(cut_on);
	}
}

void TMJViewMgr::slotTMJCutReset(const TMJDirectionType& direction_type)
{
	tmj_engine_->ResetCutTMJ(direction_type);
	view_3d_[direction_type]->UpdateCutting(
		tmj_engine_->GetCutCurrStep(direction_type));
	view_frontal_[direction_type][0]->ResetTMJCutUI();
}

void TMJViewMgr::slotTMJCutUndo(const TMJDirectionType& direction_type)
{
	tmj_engine_->UndoCutTMJ(direction_type);
	view_3d_[direction_type]->UpdateCutting(
		tmj_engine_->GetCutCurrStep(direction_type));
	view_frontal_[direction_type][0]->UndoTMJCutUI();
}

void TMJViewMgr::slotTMJCutRedo(const TMJDirectionType& direction_type)
{
	tmj_engine_->RedoCutTMJ(direction_type);
	view_3d_[direction_type]->UpdateCutting(
		tmj_engine_->GetCutCurrStep(direction_type));
	view_frontal_[direction_type][0]->RedoTMJCutUI();
}

void TMJViewMgr::slotTMJCutFrontal(const TMJDirectionType& direction_type,
	const QPolygonF& cut_area,
	const bool& is_inside)
{
	std::function<void(const std::vector<glm::vec3>&, std::vector<QPointF>&)>
		FrontalViewMapVolToScene;
	view_frontal_[direction_type][0]->GetMapVolToSceneFunc(
		FrontalViewMapVolToScene);
	tmj_engine_->CutTMJ(direction_type, cut_area, is_inside,
		FrontalViewMapVolToScene);
	view_3d_[direction_type]->UpdateCutting(
		tmj_engine_->GetCutCurrStep(direction_type));
}

void TMJViewMgr::slotTMJClipParamsChange(const TMJTaskTool::ClipID& clip_id,
	const std::vector<float>& values,
	bool clip_enable)
{
	for (int id = 0; id < TMJDirectionType::TMJ_TYPE_END; ++id)
	{
		glm::vec3 vector;
		switch (clip_id)
		{
		case TMJTaskTool::ClipID::AP:
			vector = tmj_engine_->GetLateral(static_cast<TMJDirectionType>(id))
				.up_vector();
			break;
		case TMJTaskTool::ClipID::LM:
			vector = tmj_engine_->GetFrontal(static_cast<TMJDirectionType>(id))
				.up_vector();
			break;
		case TMJTaskTool::ClipID::TB:
			vector = tmj_engine_->GetTMJBackVector();
			break;
		default:
			assert(false);
			break;
		}

		std::vector<glm::vec4> clipping_planes = { glm::vec4(-vector, values[0]),
												  glm::vec4(vector, values[1]) };

		view_3d_[id]->SetCliping(clipping_planes, clip_enable);
	}
}

void TMJViewMgr::slotLateralParamChanged(const TMJLateralID& id, double value)
{
	const float base_pixel_spacing = tmj_engine_->GetBasePixelSpacing();
	const float value_in_pixel = (float)value / base_pixel_spacing;
	view_axial_->SetLateralParam(id, value_in_pixel);

	switch (id)
	{
	case TMJLateralID::LEFT_INTERVAL:
		task_tool_->SetFrontalWidthLowerBound(
			TMJDirectionType::TMJ_LEFT,
			GetLateralViewCount(TMJDirectionType::TMJ_LEFT));
		break;
	case TMJLateralID::LEFT_THICKNESS:
		for (int k = 0; k < common::kMaxFrontalCount; k++)
		{
			view_frontal_[TMJDirectionType::TMJ_LEFT][k]->UpdateThickness();
		}
		break;
	case TMJLateralID::RIGHT_INTERVAL:
		task_tool_->SetFrontalWidthLowerBound(
			TMJDirectionType::TMJ_RIGHT,
			GetLateralViewCount(TMJDirectionType::TMJ_RIGHT));
		break;
	case TMJLateralID::RIGHT_THICKNESS:
		for (int k = 0; k < common::kMaxFrontalCount; k++)
		{
			view_frontal_[TMJDirectionType::TMJ_RIGHT][k]->UpdateThickness();
		}
		break;
	default:
		common::Logger::instance()->PrintAndAssert(
			common::LogType::ERR,
			"TMJViewMgr::slotLateralParamChanged, Invalid TMJLateralID");
		break;
	}
}
void TMJViewMgr::slotLateralViewSelected(const TMJDirectionType& direction_type,
	const int& lateral_id)
{
	tmj_engine_->SetLateralSelectedID(direction_type, lateral_id);
	for (int view_id = 0; view_id < GetLateralViewCount(direction_type);
		++view_id)
	{
		view_lateral_[direction_type][view_id]->SetSelectedStatus(
			(view_id == lateral_id) ? true : false);
	}
	view_axial_->SetHighlightLateralLine(direction_type, lateral_id);

	for (int view_id = 0; view_id < common::kMaxFrontalCount; ++view_id)
	{
		view_frontal_[direction_type][view_id]->SetLateralSelected(lateral_id);
	}
}

void TMJViewMgr::slotWheelEventFromLateralView(const int& wheel_step)
{
	bool slide_as_set = GlobalPreferences::GetInstance()->preferences_.advanced.cross_section_view.slide_as_set;

	QObject* sender = QObject::sender();
	for (int type = 0; type < TMJ_TYPE_END; type++)
	{
		int lateral_view_count = GetLateralViewCount(static_cast<TMJDirectionType>(type));
		for (int view_id = 0; view_id < lateral_view_count; view_id++)
		{
			if (view_lateral_[type][view_id].get() == sender)
			{
				float step = slide_as_set ? wheel_step * lateral_view_count : wheel_step;
				view_axial_->TranslateLateralFromWheelEvent(static_cast<TMJDirectionType>(type), step);
				return;
			}
		}
	}
}

void TMJViewMgr::slotWheelEventFromFrontalView(const int& wheel_step)
{
	QObject* sender = QObject::sender();
	for (int type = 0; type < TMJ_TYPE_END; type++)
	{
		for (int view_id = 0; view_id < frontal_count_[type]; view_id++)
		{
			if (view_frontal_[type][view_id].get() == sender)
			{
				view_axial_->TranslateFrontalFromWheelEvent(
					static_cast<TMJDirectionType>(type),
					static_cast<float>(wheel_step));
				return;
			}
		}
	}
}

void TMJViewMgr::slotSetLateralSliceFromFrontalView(const glm::vec3& pt_vol)
{
	QObject* sender = QObject::sender();
	for (int type = 0; type < TMJ_TYPE_END; type++)
	{
		for (int view_id = 0; view_id < frontal_count_[type]; view_id++)
		{
			if (view_frontal_[type][view_id].get() == sender)
			{
				TMJDirectionType dir_type = static_cast<TMJDirectionType>(type);
				glm::vec3 pt_lateral =
					tmj_engine_->GetLateral(dir_type).center_positions().at(view_id);
				TranslateLateral(dir_type, pt_vol, pt_lateral);
				return;
			}
		}
	}
}

void TMJViewMgr::slotSetFrontalSliceFromLateralView(const glm::vec3& pt_vol)
{
	QObject* sender = QObject::sender();
	for (int type = 0; type < TMJ_TYPE_END; type++)
	{
		for (int view_id = 0;
			view_id < GetLateralViewCount(static_cast<TMJDirectionType>(type));
			view_id++)
		{
			if (view_lateral_[type][view_id].get() == sender)
			{
				TMJDirectionType dir_type = static_cast<TMJDirectionType>(type);
				glm::vec3 pt_frontal =
					tmj_engine_->GetFrontal(dir_type).center_position();
				TranslateFrontal(dir_type, pt_vol, pt_frontal);
				return;
			}
		}
	}
}

void TMJViewMgr::slotSetAxialZ(const glm::vec3& pt_vol)
{
	float delta = glm::dot(view_axial_->GetUpVector(),
		pt_vol - view_axial_->GetCenterPosition());

	if (abs(delta) > std::numeric_limits<float>::epsilon())
	{
		view_axial_->SetSliceInVol(view_axial_->GetSliceInVol() + delta);
	}
}

void TMJViewMgr::slotTMJLateralSharpen(const TMJDirectionType& type,
	SharpenLevel level)
{
	QObject* sender = QObject::sender();

	for (int view_id = 0; view_id < common::kMaxLateralCount; view_id++)
	{
		if (view_lateral_[type][view_id].get() == sender) continue;

		view_lateral_[type][view_id]->SetSharpen(level);
	}
}

void TMJViewMgr::slotTMJFrontalSharpen(const TMJDirectionType& type,
	SharpenLevel level)
{
	QObject* sender = QObject::sender();

	for (int view_id = 0; view_id < common::kMaxFrontalCount; view_id++)
	{
		if (view_frontal_[type][view_id].get() == sender) continue;

		view_frontal_[type][view_id]->SetSharpen(level);
	}
}

void TMJViewMgr::slotSetLateralViewStatus(float* scene_scale,
	QPointF* gl_translate)
{
	*scene_scale = view_lateral_[TMJDirectionType::TMJ_LEFT][0]->GetSceneScale();
	*gl_translate = view_lateral_[TMJDirectionType::TMJ_LEFT][0]->GetGLTranslate();
}

void TMJViewMgr::slotSetFrontalViewStatus(float* scene_scale,
	QPointF* gl_translate)
{
	*scene_scale = view_frontal_[TMJDirectionType::TMJ_LEFT][0]->GetSceneScale();
	*gl_translate = view_frontal_[TMJDirectionType::TMJ_LEFT][0]->GetGLTranslate();
}

void TMJViewMgr::slotLateralMeasureCreated(const TMJDirectionType& type,
	const int& lateral_id,
	const unsigned int& measure_id)
{
	for (int i = 0; i < common::kMaxLateralCount; i++)
	{
		if (i == lateral_id) continue;

		view_lateral_[type][i]->SyncCreateMeasureUI(measure_id);
	}
}

void TMJViewMgr::slotLateralMeasureDeleted(const TMJDirectionType& type,
	const int& lateral_id,
	const unsigned int& measure_id)
{
	for (int i = 0; i < common::kMaxLateralCount; i++)
	{
		if (i == lateral_id) continue;

		view_lateral_[type][i]->SyncDeleteMeasureUI(measure_id);
	}
}

void TMJViewMgr::slotLateralMeasureModified(const TMJDirectionType& type,
	const int& lateral_id,
	const unsigned int& measure_id)
{
	for (int i = 0; i < common::kMaxLateralCount; i++)
	{
		if (i == lateral_id) continue;

		view_lateral_[type][i]->SyncModifyMeasureUI(measure_id);
	}
}

void TMJViewMgr::slotFrontalMeasureCreated(const TMJDirectionType& type,
	const int& lateral_id,
	const unsigned int& measure_id)
{
	for (int i = 0; i < common::kMaxFrontalCount; i++)
	{
		if (i == lateral_id) continue;

		view_frontal_[type][i]->SyncCreateMeasureUI(measure_id);
	}
}

void TMJViewMgr::slotFrontalMeasureDeleted(const TMJDirectionType& type,
	const int& lateral_id,
	const unsigned int& measure_id)
{
	for (int i = 0; i < common::kMaxFrontalCount; i++)
	{
		if (i == lateral_id) continue;

		view_frontal_[type][i]->SyncDeleteMeasureUI(measure_id);
	}
}

void TMJViewMgr::slotFrontalMeasureModified(const TMJDirectionType& type,
	const int& lateral_id,
	const unsigned int& measure_id)
{
	for (int i = 0; i < common::kMaxFrontalCount; i++)
	{
		if (i == lateral_id) continue;

		view_frontal_[type][i]->SyncModifyMeasureUI(measure_id);
	}
}

bool TMJViewMgr::ShiftLateralView(const TMJDirectionType& direction)
{
	bool slide_as_set = GlobalPreferences::GetInstance()->preferences_.advanced.cross_section_view.slide_as_set;
	int view_count = GetLateralViewCount(direction);
	float step = slide_as_set ? static_cast<float>(view_count) : 1.0f;

	return view_axial_->TranslateLateralFromWheelEvent(direction, step);
}

bool TMJViewMgr::ShiftFrontalView(const TMJDirectionType& direction)
{
	bool slide_as_set = GlobalPreferences::GetInstance()->preferences_.advanced.cross_section_view.slide_as_set;
	int view_count = common::kMaxFrontalCount;
	float step = slide_as_set ? static_cast<float>(view_count) : 1.0f;

	return view_axial_->TranslateFrontalFromWheelEvent(direction, step);
}

void TMJViewMgr::SetFrontalLineIndex(const TMJDirectionType& direction, int index)
{
	view_axial_->SetFrontalLineIndex(direction, index);
}

void TMJViewMgr::SetLateralLineIndex(const TMJDirectionType& direction, int index)
{
	view_axial_->SetLateralLineIndex(direction, index);
}

#ifdef WILL3D_EUROPE
void TMJViewMgr::SetSyncControlButtonOut()
{
	view_axial_.get()->SetSyncControlButton(false);

	for (int i = 0; i < TMJDirectionType::TMJ_TYPE_END; ++i)
	{
		for (int j = 0; j < common::kMaxFrontalCount; ++j)
		{
			view_frontal_[i][j].get()->SetSyncControlButton(false);
		}

		for (int j = 0; j < common::kMaxLateralCount; ++j)
		{
			view_lateral_[i][j].get()->SetSyncControlButton(false);
		}

		view_3d_[i].get()->SetSyncControlButton(false);
	}
}
#endif // WILL3D_EUROPE
