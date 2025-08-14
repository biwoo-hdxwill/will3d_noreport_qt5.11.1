#include "W3TMJtab.h"
/*=========================================================================

File:			class CW3TMJtab
Language:		C++11
Library:		Qt 5.2.0
Author:			Hong Jung
First date:		2015-12-02
Last modify:	2015-12-02

=========================================================================*/

#include <Engine/Common/Common/global_preferences.h>
#include <Engine/Common/Common/W3LayoutFunctions.h>
#include <Engine/Common/Common/W3Logger.h>
#include <Engine/Common/Common/W3Memory.h>
#include <Engine/Common/Common/language_pack.h>

#include <Engine/Resource/ResContainer/resource_container.h>
#include <Engine/Resource/Resource/W3Image3D.h>
#ifndef WILL3D_VIEWER
#include <Engine/Core/W3ProjectIO/project_io_tmj.h>
#endif
#include <Engine/UIModule/UIFrame/window_plane.h>
#include <Engine/UIModule/UIFrame/window_tmj_frontal.h>
#include <Engine/UIModule/UIFrame/window_tmj_lateral.h>
#include <Engine/UIModule/UITools/tmj_task_tool.h>
#include <Engine/UIModule/UITools/tool_mgr.h>
#include <Engine/Module/TMJ/tmj_engine.h>
#include <Engine/UIModule/UIFrame/orientation_dlg.h>

#include "tmj_view_mgr.h"

CW3TMJtab::CW3TMJtab() : task_tool_(new TMJTaskTool)
{
	ToolMgr::instance()->SetTMJTaskTool(task_tool_);
	slotTMJGetTaskLayout(task_tool_->GetTMJTaskLayout());
}

CW3TMJtab::~CW3TMJtab(void) {}
#ifndef WILL3D_VIEWER
void CW3TMJtab::exportProject(ProjectIOTMJ& out)
{
	if (tmj_view_mgr_)
	{
		tmj_view_mgr_->exportProject(out);
		tmj_engine_->ExportProject(out);
		task_tool_->ExportProject(out);
	}
}

void CW3TMJtab::importProject(ProjectIOTMJ& in)
{
	if (in.IsInit())
	{
		if (!initialized())
		{
			Initialize();
		}
		tmj_engine_->ImportProject(in);
		task_tool_->ImportProject(in);
		tmj_view_mgr_->importProject(in);
	}
}
#endif
void CW3TMJtab::SetTMjengine(const std::shared_ptr<TMJengine>& tmj_engine)
{
	tmj_engine_ = tmj_engine;
}

void CW3TMJtab::Initialize()
{
	if (BaseTab::initialized())
	{
		common::Logger::instance()->PrintAndAssert(common::LogType::ERR,
			"already initialized.");
	}

	tmj_view_mgr_.reset(new TMJViewMgr());
	this->SetCastedViewMgr(std::static_pointer_cast<BaseViewMgr>(tmj_view_mgr_));
	tmj_view_mgr_->SetTMJengine(tmj_engine_);

	orientation_dlg_.reset(new OrientationDlg());
	orientation_dlg_->setVisible(false);

	window_axial_.reset(new WindowPlane(lang::LanguagePack::txt_axial()));
	window_right_3d_.reset(new WindowPlane(lang::LanguagePack::txt_right() +
		lang::LanguagePack::txt_3d()));
	window_right_3d_->SetView(tmj_view_mgr_->GetViewFrontal3DWidget(TMJ_RIGHT));
	window_left_3d_.reset(new WindowPlane(lang::LanguagePack::txt_left() +
		lang::LanguagePack::txt_3d()));
	window_left_3d_->SetView(tmj_view_mgr_->GetViewFrontal3DWidget(TMJ_LEFT));
	window_left_frontal_.reset(new WindowTmjFrontal(TMJ_LEFT));
	window_right_frontal_.reset(new WindowTmjFrontal(TMJ_RIGHT));
	window_left_lateral_.reset(new WindowTmjLateral(TMJ_LEFT));
	window_right_lateral_.reset(new WindowTmjLateral(TMJ_RIGHT));

	window_left_lateral_->SetIntervalMinimumValue(
		ResourceContainer::GetInstance()->GetMainVolume().pixelSpacing());
	window_right_lateral_->SetIntervalMinimumValue(
		ResourceContainer::GetInstance()->GetMainVolume().pixelSpacing());

	QSize default_lateral_layout = GlobalPreferences::GetInstance()->preferences_.advanced.tmj.default_lateral_layout;
	window_left_lateral_->SetViews(tmj_view_mgr_->GetViewLateralWidget(TMJ_LEFT));
	window_left_lateral_->SetViewLayoutCount(default_lateral_layout.height(), default_lateral_layout.width());
	window_right_lateral_->SetViews(tmj_view_mgr_->GetViewLateralWidget(TMJ_RIGHT));
	window_right_lateral_->SetViewLayoutCount(default_lateral_layout.height(), default_lateral_layout.width());

	window_left_frontal_->SetViews(tmj_view_mgr_->GetViewFrontalWidget(TMJ_LEFT));
	window_left_frontal_->SetViewLayoutCount(1, 1);
	window_right_frontal_->SetViews(
		tmj_view_mgr_->GetViewFrontalWidget(TMJ_RIGHT));
	window_right_frontal_->SetViewLayoutCount(1, 1);

	window_axial_->SetView(tmj_view_mgr_->GetViewAxial());

	orientation_dlg_->SetView(tmj_view_mgr_->GetViewOrient(ReorientViewID::ORIENT_A),
		tmj_view_mgr_->GetViewOrient(ReorientViewID::ORIENT_R),
		tmj_view_mgr_->GetViewOrient(ReorientViewID::ORIENT_I));

	OrientationTool::OrientUI orient_ui;
	orient_ui.a = orientation_dlg_->GetOrienA();
	orient_ui.r = orientation_dlg_->GetOrienR();
	orient_ui.i = orientation_dlg_->GetOrienI();

	TMJTaskTool::LateralUI lateral_ui;
	lateral_ui.left_interval = window_left_lateral_->GetInterval();
	lateral_ui.left_thickness = window_left_lateral_->GetThickness();
	lateral_ui.right_interval = window_right_lateral_->GetInterval();
	lateral_ui.right_thickness = window_right_lateral_->GetThickness();

	TMJTaskTool::FrontalUI frontal_ui;
	frontal_ui.left_width = window_left_frontal_->GetWidth();
	frontal_ui.left_height = window_left_frontal_->GetHeight();
	frontal_ui.right_width = window_right_frontal_->GetWidth();
	frontal_ui.right_height = window_right_frontal_->GetHeight();

	TMJTaskTool::CutControlUI cut_right_ui;
	cut_right_ui.reset = window_right_frontal_->GetReset();
	cut_right_ui.undo = window_right_frontal_->GetUndo();
	cut_right_ui.redo = window_right_frontal_->GetRedo();

	TMJTaskTool::CutControlUI cut_left_ui;
	cut_left_ui.reset = window_left_frontal_->GetReset();
	cut_left_ui.undo = window_left_frontal_->GetUndo();
	cut_left_ui.redo = window_left_frontal_->GetRedo();

	task_tool_->InitExternUIs(orient_ui, lateral_ui, frontal_ui, cut_right_ui,
		cut_left_ui);
	tmj_view_mgr_->set_task_tool(task_tool_);

	SetLayout();

	connections();

	BaseTab::set_initialized(true);
}

void CW3TMJtab::SetDefault2Dlayout()
{
	CW3LayoutFunctions::RemoveWidgetsAll(top_layout_.get());
	CW3LayoutFunctions::RemoveWidgetsAll(bottom_layout_.get());

	main_layout_->setStretch(0, 1);
	main_layout_->setStretch(1, 1);

	top_layout_->addWidget(window_right_frontal_.get());
	top_layout_->setStretch(0, 1);
	top_layout_->addWidget(window_axial_.get());
	top_layout_->setStretch(1, 2);
	top_layout_->addWidget(window_left_frontal_.get());
	top_layout_->setStretch(2, 1);

	window_left_frontal_->SetViews(tmj_view_mgr_->GetViewFrontalWidget(TMJ_LEFT),
		1, 1);
	window_right_frontal_->SetViews(
		tmj_view_mgr_->GetViewFrontalWidget(TMJ_RIGHT), 1, 1);

	for (int i = 0; i < TMJDirectionType::TMJ_TYPE_END; i++)
		tmj_view_mgr_->SelectLayoutFrontal((TMJDirectionType)i, 1);

	bottom_layout_->addWidget(window_right_lateral_.get());
	bottom_layout_->setStretch(0, 1);
	bottom_layout_->addItem(new QSpacerItem(10, 0, QSizePolicy::Fixed, QSizePolicy::Preferred));
	bottom_layout_->addWidget(window_left_lateral_.get());
	bottom_layout_->setStretch(2, 1);
}

void CW3TMJtab::SetLateralMain2Dlayout()
{
	SetDefault2Dlayout();
	main_layout_->setStretch(0, 1);
	main_layout_->setStretch(1, 2);
}

void CW3TMJtab::SetFrontalMain2Dlayout()
{
	CW3LayoutFunctions::RemoveWidgetsAll(top_layout_.get());
	CW3LayoutFunctions::RemoveWidgetsAll(bottom_layout_.get());
	main_layout_->setStretch(0, 1);
	main_layout_->setStretch(1, 1);

	top_layout_->addWidget(window_right_frontal_.get());
	top_layout_->setStretch(0, 1);
	top_layout_->addItem(new QSpacerItem(10, 0, QSizePolicy::Fixed, QSizePolicy::Preferred));
	top_layout_->addWidget(window_left_frontal_.get());
	top_layout_->setStretch(2, 1);

	for (int i = 0; i < TMJDirectionType::TMJ_TYPE_END; i++)
	{
		tmj_view_mgr_->SelectLayoutFrontal((TMJDirectionType)i, 3);
	}

	window_left_frontal_->SetViews(tmj_view_mgr_->GetViewFrontalWidget(TMJ_LEFT), 1, 3);
	window_right_frontal_->SetViews(tmj_view_mgr_->GetViewFrontalWidget(TMJ_RIGHT), 1, 3);

	bottom_layout_->addWidget(window_right_lateral_.get());
	bottom_layout_->setStretch(0, 1);
	bottom_layout_->addItem(new QSpacerItem(10, 0, QSizePolicy::Fixed, QSizePolicy::Preferred));
	bottom_layout_->addWidget(window_left_lateral_.get());
	bottom_layout_->setStretch(2, 1);
}

void CW3TMJtab::SetDefault3Dlayout()
{
	CW3LayoutFunctions::RemoveWidgetsAll(top_layout_.get());
	CW3LayoutFunctions::RemoveWidgetsAll(bottom_layout_.get());

	main_layout_->setStretch(0, 1);
	main_layout_->setStretch(1, 1);

	top_layout_->addWidget(window_right_frontal_.get());
	top_layout_->setStretch(0, 1);
	top_layout_->addWidget(window_axial_.get());
	top_layout_->setStretch(1, 2);
	top_layout_->addWidget(window_left_frontal_.get());
	top_layout_->setStretch(2, 1);

	for (int i = 0; i < TMJDirectionType::TMJ_TYPE_END; i++)
		tmj_view_mgr_->SelectLayoutFrontal((TMJDirectionType)i, 1);

	window_left_frontal_->SetViews(tmj_view_mgr_->GetViewFrontalWidget(TMJ_LEFT),
		1, 1);
	window_right_frontal_->SetViews(
		tmj_view_mgr_->GetViewFrontalWidget(TMJ_RIGHT), 1, 1);

	bottom_layout_->addWidget(window_right_3d_.get());
	bottom_layout_->setStretch(0, 1);
	bottom_layout_->addWidget(window_left_3d_.get());
	bottom_layout_->setStretch(1, 1);
}

void CW3TMJtab::SetTMJlayout(const TmjLayoutType& layout_type)
{
	bool cut_on = false;

	switch (layout_type)
	{
	case TmjLayoutType::DEFAULT_2D:
		SetDefault2Dlayout();
		break;
	case TmjLayoutType::LATERAL_MAIN_2D:
		SetLateralMain2Dlayout();
		break;
	case TmjLayoutType::FRONTAL_MAIN_2D:
		SetFrontalMain2Dlayout();
		break;
	case TmjLayoutType::DEFAULT_3D:
		SetDefault3Dlayout();
		cut_on = task_tool_->IsCutEnabled();
		break;
	default:
		assert(false);
		break;
	}

	SetVisibleCutUI(cut_on);
}

void CW3TMJtab::SetVisibleCutUI(const bool visible)
{
	window_left_frontal_->Set3DCutMode(visible);
	window_right_frontal_->Set3DCutMode(visible);
}

void CW3TMJtab::slotTMJCutEnable(const bool& cut_on, const VRCutTool& cut_tool)
{
	SetVisibleCutUI(cut_on);
}

void CW3TMJtab::connections()
{
	const auto& task_tool = task_tool_.get();
	connect(task_tool, &TMJTaskTool::sigTMJGetTaskLayout, this, &CW3TMJtab::slotTMJGetTaskLayout);
	connect(task_tool, &TMJTaskTool::sigTMJReorient, this, &CW3TMJtab::slotOrienAdjust);
	connect(task_tool, &TMJTaskTool::sigTMJReorientReset, this, &CW3TMJtab::slotResetOrientation);
	connect(task_tool, &TMJTaskTool::sigTMJLaoutChanged, this, &CW3TMJtab::slotTMJLaoutChanged);
	connect(task_tool, &TMJTaskTool::sigTMJDeleteRect, this, &CW3TMJtab::slotTMJRectDelete);
	connect(task_tool, &TMJTaskTool::sigTMJCutEnable, this, &CW3TMJtab::slotTMJCutEnable);

	connect(orientation_dlg_.get(), &OrientationDlg::sigGridOnOff, this, &CW3TMJtab::slotGridOnOffOrientation);
	connect(orientation_dlg_.get(), &OrientationDlg::sigResetOrientation, this, &CW3TMJtab::slotResetOrientation);

	connect(window_left_lateral_.get(), &WindowTmjLateral::sigSelectLayout, this, &CW3TMJtab::slotSelectLayout);
	connect(window_right_lateral_.get(), &WindowTmjLateral::sigSelectLayout, this, &CW3TMJtab::slotSelectLayout);

#ifdef WILL3D_EUROPE
	connect(tmj_view_mgr_.get(), &TMJViewMgr::sigShowButtonListDialog, this, &CW3TMJtab::sigShowButtonListDialog);
#endif // WILL3D_EUROPE
}

void CW3TMJtab::SetLayout()
{
	main_layout_.reset(new QVBoxLayout());

	top_layout_.reset(new QHBoxLayout());
	top_layout_->setSpacing(kLayoutSpacing);

	bottom_layout_.reset(new QHBoxLayout());
	bottom_layout_->setSpacing(kLayoutSpacing);

	main_layout_->setSpacing(kLayoutSpacing);
	main_layout_->addLayout(top_layout_.get());
	main_layout_->setStretch(0, 1);
	main_layout_->addLayout(bottom_layout_.get());
	main_layout_->setStretch(1, 1);

	this->SetTMJlayout(tmj_layout_type_);

	tab_layout_ = main_layout_.get();
}

void CW3TMJtab::SetVisibleWindows(bool isVisible)
{
	if (!initialized()) return;

	if (isVisible)
	{
		tmj_view_mgr_->SyncMeasureResource();
		CW3LayoutFunctions::setVisibleWidgets(tab_layout_, isVisible);
		tmj_view_mgr_->SyncTMJItemVisibleUI();
	}
	else
	{
		window_axial_->setVisible(isVisible);
		window_left_lateral_->setVisible(isVisible);
		window_right_lateral_->setVisible(isVisible);
		window_left_frontal_->setVisible(isVisible);
		window_right_frontal_->setVisible(isVisible);
		window_left_3d_->setVisible(isVisible);
		window_right_3d_->setVisible(isVisible);
		tmj_view_mgr_->SetVisibleViews(isVisible);
	}
}

QStringList CW3TMJtab::GetViewList()
{
	switch (tmj_layout_type_)
	{
	case TmjLayoutType::DEFAULT_2D:
	case TmjLayoutType::LATERAL_MAIN_2D:
		return QStringList{
			window_left_frontal_.get()->window_title(),
			window_right_frontal_.get()->window_title(),
			window_axial_.get()->window_title(),
			window_left_lateral_.get()->window_title(),
			window_right_lateral_.get()->window_title()
		};
	case TmjLayoutType::FRONTAL_MAIN_2D:
		return QStringList{
			window_left_frontal_.get()->window_title(),
			window_right_frontal_.get()->window_title(),
			window_left_lateral_.get()->window_title(),
			window_right_lateral_.get()->window_title()
		};
		break;
	case TmjLayoutType::DEFAULT_3D:
		return QStringList{
			window_left_frontal_.get()->window_title(),
			window_right_frontal_.get()->window_title(),
			window_axial_.get()->window_title(),
			window_left_3d_.get()->window_title(),
			window_right_3d_.get()->window_title()
		};
	}
	return QStringList();
}

QImage CW3TMJtab::GetScreenshot(int view_type)
{
	QWidget* source = GetScreenshotSource(view_type);

	return BaseTab::GetScreenshot(source);
}

QWidget* CW3TMJtab::GetScreenshotSource(int view_type)
{
	QWidget* source = nullptr;

	switch (tmj_layout_type_)
	{
	case TmjLayoutType::DEFAULT_2D:
	case TmjLayoutType::LATERAL_MAIN_2D:
		switch (view_type)
		{
		case 1:
			source = window_left_frontal_.get();
			break;
		case 2:
			source = window_right_frontal_.get();
			break;
		case 3:
			source = window_axial_.get();
			break;
		case 4:
			source = window_left_lateral_.get();
			break;
		case 5:
			source = window_right_lateral_.get();
			break;
		}
		break;
	case TmjLayoutType::FRONTAL_MAIN_2D:
		switch (view_type)
		{
		case 1:
			source = window_left_frontal_.get();
			break;
		case 2:
			source = window_right_frontal_.get();
			break;
		case 3:
			source = window_left_lateral_.get();
			break;
		case 4:
			source = window_right_lateral_.get();
			break;
		}
		break;
	case TmjLayoutType::DEFAULT_3D:
		switch (view_type)
		{
		case 1:
			source = window_left_frontal_.get();
			break;
		case 2:
			source = window_right_frontal_.get();
			break;
		case 3:
			source = window_axial_.get();
			break;
		case 4:
			source = window_left_3d_.get();
			break;
		case 5:
			source = window_right_3d_.get();
			break;
		}
		break;
	}

	bool continuous_capture = GlobalPreferences::GetInstance()->preferences_.capture.continuous;
	if (continuous_capture &&
		(source == window_left_frontal_.get() ||
			source == window_right_frontal_.get() ||
			source == window_left_lateral_.get() ||
			source == window_right_lateral_.get()))
	{
		if (source == window_left_frontal_.get())
		{
			tmj_view_mgr_->SetFrontalLineIndex(TMJDirectionType::TMJ_LEFT, 0);
		}
		else if (source == window_right_frontal_.get())
		{
			tmj_view_mgr_->SetFrontalLineIndex(TMJDirectionType::TMJ_RIGHT, 0);
		}
		else if (source == window_left_lateral_.get())
		{
			tmj_view_mgr_->SetLateralLineIndex(TMJDirectionType::TMJ_LEFT, 0);
		}
		else if (source == window_right_lateral_.get())
		{
			tmj_view_mgr_->SetLateralLineIndex(TMJDirectionType::TMJ_RIGHT, 0);
		}

		emit sigContinuousCapture(source);
		source = nullptr;
	}

	return source;
}

void CW3TMJtab::ApplyPreferences()
{
	if (tmj_view_mgr_) tmj_view_mgr_->ApplyPreferences();
}

void CW3TMJtab::slotTMJGetTaskLayout(QVBoxLayout* layout)
{
	const TMJTaskTool::Mode& mode = task_tool_->mode();
	if (mode == TMJTaskTool::Mode::MODE_2D)
	{
		ToolMgr::instance()->GetTMJ2DTaskLayout(layout);
	}
	else if (mode == TMJTaskTool::Mode::MODE_3D)
	{
		ToolMgr::instance()->GetTMJ3DTaskLayout(layout);
	}
}

void CW3TMJtab::slotOrienAdjust()
{
	bool grid_on = orientation_dlg_->IsGridOn();
	slotGridOnOffOrientation(grid_on);
	orientation_dlg_->exec();
}

void CW3TMJtab::slotTMJLaoutChanged(const TmjLayoutType& layout_type)
{
	this->SetVisibleWindows(false);
	this->SetTMJlayout(layout_type);
	tmj_layout_type_ = layout_type;
	this->SetVisibleWindows(true);
}

void CW3TMJtab::slotTMJRectDelete(const TMJDirectionType& type)
{
	tmj_view_mgr_->DeleteTMJROIRect(type);
	emit sigCommonToolCancelSelected();
}

void CW3TMJtab::slotResetOrientation()
{
	tmj_view_mgr_->ResetOrientationParams();
}

void CW3TMJtab::slotGridOnOffOrientation(bool on)
{
	tmj_view_mgr_->GridOnOffOrientation(on);
}

void CW3TMJtab::slotSelectLayout(int row, int col)
{
	if (QObject::sender() == window_left_lateral_.get())
	{
		tmj_view_mgr_->SelectLayoutLateral(TMJ_LEFT, row, col);
	}
	else if (QObject::sender() == window_right_lateral_.get())
	{
		tmj_view_mgr_->SelectLayoutLateral(TMJ_RIGHT, row, col);
	}
	else
	{
		common::Logger::instance()->PrintAndAssert(common::LogType::ERR,
			"CW3TMJtab::slotSelectLayout");
	}
}

bool CW3TMJtab::ShiftContinuousView(QWidget* source)
{
	if (source == window_left_lateral_.get())
	{
		return tmj_view_mgr_->ShiftLateralView(TMJDirectionType::TMJ_LEFT);
	}
	else if (source == window_right_lateral_.get())
	{
		return tmj_view_mgr_->ShiftLateralView(TMJDirectionType::TMJ_RIGHT);
	}
	else if (source == window_left_frontal_.get())
	{
		return tmj_view_mgr_->ShiftFrontalView(TMJDirectionType::TMJ_LEFT);
	}
	else if (source == window_right_frontal_.get())
	{
		return tmj_view_mgr_->ShiftFrontalView(TMJDirectionType::TMJ_RIGHT);
	}
	else
	{
		return false;
	}
}

#ifdef WILL3D_EUROPE
void CW3TMJtab::SyncControlButtonOut()
{
	tmj_view_mgr_->SetSyncControlButtonOut();
}
#endif // WILL3D_EUROPE
