#include "W3ENDOtab.h"
/*=========================================================================

File:			class CW3ENDOtab
Language:		C++11
Library:		Qt 5.4.0
Author:			Jung Dae Gun, Hong Jung
First date:		2016-01-13
Last modify:	2016-04-07

=========================================================================*/

#include <Engine/Common/Common/W3Memory.h>
#include <Engine/Common/Common/W3Logger.h>
#include <Engine/Common/Common/language_pack.h>

#ifndef WILL3D_VIEWER
#include <Engine/Core/W3ProjectIO/project_io_endo.h>
#endif

#include <Engine/UIModule/UITools/tool_mgr.h>
#include <Engine/UIModule/UITools/endo_task_tool.h>

#include <Engine/UIModule/UIFrame/window_plane.h>

#include "W3ENDOViewMgr.h"

CW3ENDOtab::CW3ENDOtab(CW3VREngine *VREngine, CW3MPREngine *MPRengine, CW3JobMgr *JobMgr, CW3ResourceContainer *Rcontainer)
	: m_pgVREngine(VREngine),
	m_pgMPRengine(MPRengine), m_pgJobMgr(JobMgr),
	m_pgRcontainer(Rcontainer)
{
	task_tool_.reset(new EndoTaskTool(this));
	ToolMgr::instance()->SetEndoTaskTool(task_tool_);
}

CW3ENDOtab::~CW3ENDOtab(void)
{
	SAFE_DELETE_OBJECT(m_pENDOViewMgr);
	SAFE_DELETE_OBJECT(main_layout_);
}

#ifndef WILL3D_VIEWER
void CW3ENDOtab::exportProject(ProjectIOEndo& out)
{
	if (m_pENDOViewMgr)
		m_pENDOViewMgr->exportProject(out);
}

void CW3ENDOtab::importProject(ProjectIOEndo& in)
{
	if (in.IsInit())
	{
		if (!initialized())
		{
			Initialize();
		}
		m_pENDOViewMgr->importProject(in);
	}
}
#endif

void CW3ENDOtab::UpdateVRview(bool is_high_quality)
{
	m_pENDOViewMgr->UpdateVRview(is_high_quality);
}

#ifdef WILL3D_EUROPE
void CW3ENDOtab::slotMaximizeOnOff(bool maximize)
{
	window_slice_->setVisible(false);
	window_vr_->setVisible(false);
	window_modify_->setVisible(false);

	if (maximize == false)
	{
		window_slice_->setVisible(true);
		window_vr_->setVisible(true);
		window_modify_->setVisible(true);
	}
}
#endif // WILL3D_EUROPE

void CW3ENDOtab::SetLayout()
{
#if 0
	SAFE_DELETE_OBJECT(main_layout_);

	QVBoxLayout* left_layout = new QVBoxLayout;
	QVBoxLayout* right_layout = new QVBoxLayout;
	QHBoxLayout* endo_layout = new QHBoxLayout;
	main_layout_ = new QVBoxLayout;

	left_layout->setSpacing(kLayoutSpacing);
	left_layout->addWidget(window_slice_.get());
	left_layout->setStretch(0, 1);
	left_layout->addWidget(window_sagittal_.get());
	left_layout->setStretch(1, 1);

	right_layout->setSpacing(kLayoutSpacing);
	right_layout->addWidget(window_vr_.get());
	right_layout->setStretch(0, 1);
	right_layout->addWidget(window_modify_.get());
	right_layout->setStretch(1, 1);

	endo_layout->setSpacing(kLayoutSpacing);
	endo_layout->addLayout(left_layout);
	endo_layout->setStretch(0, 1);
	endo_layout->addLayout(right_layout);
	endo_layout->setStretch(1, 1);

	main_layout_->setSpacing(kLayoutSpacing);
	main_layout_->addLayout(endo_layout);

	tab_layout_ = main_layout_;
#else
	if (main_layout_ == nullptr)
	{
		main_layout_ = new QVBoxLayout();
		QGridLayout* grid_layout = new QGridLayout();

		grid_layout->addWidget(window_slice_.get(), 0, 0);
		grid_layout->addWidget(window_sagittal_.get(), 1, 0);
		grid_layout->addWidget(window_vr_.get(), 0, 1);
		grid_layout->addWidget(window_modify_.get(), 1, 1);

		main_layout_->setSpacing(kLayoutSpacing);
		main_layout_->addLayout(grid_layout);

		tab_layout_ = main_layout_;
	}
#endif	
}

void CW3ENDOtab::Initialize()
{
	if (BaseTab::initialized())
	{
		common::Logger::instance()->Print(common::LogType::ERR, "already initialized.");
		assert(false);
	}

	window_slice_.reset(new WindowPlane(lang::LanguagePack::txt_endoscopy_slice()));
	window_sagittal_.reset(new WindowPlane(lang::LanguagePack::txt_draw_path()));
	window_vr_.reset(new WindowPlane(lang::LanguagePack::txt_endoscopy()));
	window_modify_.reset(new WindowPlane(lang::LanguagePack::txt_modify()));

	m_pENDOViewMgr = new CW3ENDOViewMgr(m_pgVREngine, m_pgMPRengine, m_pgJobMgr, m_pgRcontainer);
	m_pENDOViewMgr->set_task_tool(task_tool_);

	window_slice_->SetView(m_pENDOViewMgr->getEndoSliceView());
	window_sagittal_->SetView(m_pENDOViewMgr->getEndoSagittalView());
	window_vr_->SetView(m_pENDOViewMgr->getEndoVRview());
	window_modify_->SetView(m_pENDOViewMgr->getEndoModifyVRview());

#ifdef WILL3D_EUROPE
	window_sagittal_->AddMaximizeButton();
	connect(window_sagittal_.get(), &WindowPlane::sigMaximizeOnOff, this, &CW3ENDOtab::slotMaximizeOnOff);
	connect(m_pENDOViewMgr, &CW3ENDOViewMgr::sigSpline3DEnd, [=]()
	{
		window_sagittal_->SetMaximize(false);
		slotMaximizeOnOff(false);
	});
#endif // WILL3D_EUROPE

	SetLayout();

	connections();
	BaseTab::set_initialized(true);
}

void CW3ENDOtab::connections()
{
	connect(m_pENDOViewMgr, &CW3ENDOViewMgr::sigSetAirway, this, &CW3ENDOtab::sigSetAirway);
	connect(m_pENDOViewMgr, &CW3ENDOViewMgr::sigAirwayEnable, this, &CW3ENDOtab::sigAirwayEnable);
	connect(m_pENDOViewMgr, SIGNAL(sigSetAirwaySize(double)), this, SIGNAL(sigSetAirwaySize(double)));

#ifdef WILL3D_EUROPE
	connect(m_pENDOViewMgr, &CW3ENDOViewMgr::sigShowButtonListDialog, this, &CW3ENDOtab::sigShowButtonListDialog);
#endif // WILL3D_EUROPE
}

void CW3ENDOtab::SetVisibleWindows(bool isVisible)
{
	if (!initialized())
		return;

	window_slice_->setVisible(isVisible);
	window_sagittal_->setVisible(isVisible);
	window_vr_->setVisible(isVisible);
	window_modify_->setVisible(isVisible);

	m_pENDOViewMgr->setVisibleViews(isVisible);
}

void CW3ENDOtab::SetCommonToolOnce(const common::CommonToolTypeOnce & type, bool on)
{
	m_pENDOViewMgr->SetCommonToolOnce(type, on);
}

void CW3ENDOtab::SetCommonToolOnOff(const common::CommonToolTypeOnOff & type)
{
	m_pENDOViewMgr->SetCommonToolOnOff(type);
}

QStringList CW3ENDOtab::GetViewList()
{
	return QStringList{
		window_slice_.get()->window_title(),
		window_vr_.get()->window_title(),
		window_sagittal_.get()->window_title(),
		window_modify_.get()->window_title()
	};
}

QImage CW3ENDOtab::GetScreenshot(int view_type)
{
	QWidget* source = GetScreenshotSource(view_type);

	return BaseTab::GetScreenshot(source);
}

QWidget* CW3ENDOtab::GetScreenshotSource(int view_type)
{
	QWidget* source = nullptr;

	switch (view_type)
	{
	case 1:
		source = window_slice_.get();
		break;
	case 2:
		source = window_vr_.get();
		break;
	case 3:
		source = window_sagittal_.get();
		break;
	case 4:
		source = window_modify_.get();
		break;
	}

	return source;
}

void CW3ENDOtab::DeleteMeasureUI(const common::ViewTypeID& view_type, const unsigned int& measure_id)
{
	m_pENDOViewMgr->DeleteMeasureUI(view_type, measure_id);
}

void CW3ENDOtab::ApplyPreferences()
{
	if (m_pENDOViewMgr)
		m_pENDOViewMgr->ApplyPreferences();
}

#ifdef WILL3D_EUROPE
void CW3ENDOtab::SyncControlButtonOut()
{
	m_pENDOViewMgr->SetSyncControlButtonOut();
}
#endif // WILL3D_EUROPE
