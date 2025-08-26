#include "W3MPRtab.h"
/*=========================================================================

File:			class CW3MPRtab
Language:		C++11
Library:		Qt 5.2.0
Author:			Hong Jung
First date:		2015-11-21
Last modify:	2015-11-21

=========================================================================*/

#include <QDebug>
#include <QKeyEvent>
#include <QSettings>

#include <Engine/Common/Common/global_preferences.h>
#include <Engine/Common/Common/W3Logger.h>
#include <Engine/Common/Common/W3Memory.h>
#include <Engine/Common/Common/event_handle_common.h>
#include <Engine/Common/Common/event_handle_measure.h>
#include <Engine/Common/Common/event_handler.h>

#include <Engine/Resource/ResContainer/W3ResourceContainer.h>
#include <Engine/Resource/ResContainer/resource_container.h>
#include <Engine/Resource/Resource/W3Image3D.h>
#include <Engine/Resource/Resource/lightbox_resource.h>
#ifndef WILL3D_VIEWER
#include <Engine/Core/W3ProjectIO/project_io_mpr.h>
#endif
#include <Engine/Module/MPREngine/W3MPREngine.h>
#include <Engine/Module/MPREngine/mpr_engine.h>
#include <Engine/Module/Panorama/pano_engine.h>

#include <Engine/UIModule/UITools/mpr_task_tool.h>
#include <Engine/UIModule/UITools/tool_mgr.h>

#include <Engine/UIModule/UIFrame/window.h>
#include <Engine/UIModule/UIFrame/window_lightbox.h>
#include <Engine/UIModule/UIFrame/window_mpr.h>
#include <Engine/UIModule/UIFrame/window_mpr_3d.h>
#include <Engine/UIModule/UIFrame/orientation_dlg.h>

#include "../JobMgr/W3Jobmgr.h"
#include "W3MPRViewMgr.h"

namespace
{
	int kMaximizedViewStretch = 3;
	int kNormalViewStretch = 1;
}

CW3MPRtab::CW3MPRtab(CW3VREngine *VREngine, CW3MPREngine *MPRengine,
										 CW3JobMgr *JobMgr, CW3ResourceContainer *Rcontainer)
		: m_pgVREngine(VREngine),
			m_pgMPRengine(MPRengine),
			m_pgJobMgr(JobMgr),
			m_pgRcontainer(Rcontainer)
{
	hide_mpr_views_on_maximized_vr_layout_ = GlobalPreferences::GetInstance()->preferences_.advanced.mpr.hide_mpr_views_on_maximized_vr_layout;

	task_tool_.reset(new MPRTaskTool(this));
	ToolMgr::instance()->SetMPRTaskTool(task_tool_);
}

CW3MPRtab::~CW3MPRtab(void)
{
	SAFE_DELETE_OBJECT(m_pMPRViewMgr);
}

void CW3MPRtab::Initialize()
{
	if (BaseTab::initialized())
	{
		common::Logger::instance()->Print(common::LogType::ERR, "already initialized.");
		assert(false);
	}
	window_axial_.reset(new WindowMPR(MPRViewType::AXIAL));
	window_sagittal_.reset(new WindowMPR(MPRViewType::SAGITTAL));
	window_coronal_.reset(new WindowMPR(MPRViewType::CORONAL));
	window_vr_.reset(new WindowMPR3D(WindowMPR3D::MPR3DMode::VR));
	window_vr_zoom_3d_.reset(new WindowMPR3D(WindowMPR3D::MPR3DMode::ZOOM3D));

	m_pMPRViewMgr =
			new CW3MPRViewMgr(m_pgVREngine, m_pgMPRengine, m_pgRcontainer);
	m_pMPRViewMgr->SetPanoEngine(pano_engine_);

	window_axial_->SetView(m_pMPRViewMgr->GetViewMPR(MPRViewType::AXIAL));
	window_sagittal_->SetView(m_pMPRViewMgr->GetViewMPR(MPRViewType::SAGITTAL));
	window_coronal_->SetView(m_pMPRViewMgr->GetViewMPR(MPRViewType::CORONAL));
	window_vr_->SetView(m_pMPRViewMgr->GetViewVR());
	window_vr_zoom_3d_->SetView(m_pMPRViewMgr->GetViewZoom3D());

	orientation_dlg_.reset(new OrientationDlg());
	orientation_dlg_->setVisible(false);

	OrientationTool::OrientUI orient_ui;
	orient_ui.a = orientation_dlg_->GetOrienA();
	orient_ui.r = orientation_dlg_->GetOrienR();
	orient_ui.i = orientation_dlg_->GetOrienI();

	task_tool_->InitExternUIs(orient_ui);

	m_pMPRViewMgr->set_task_tool(task_tool_);

	orientation_dlg_->SetView(
			m_pMPRViewMgr->GetViewOrientation(ReorientViewID::ORIENT_A),
			m_pMPRViewMgr->GetViewOrientation(ReorientViewID::ORIENT_R),
			m_pMPRViewMgr->GetViewOrientation(ReorientViewID::ORIENT_I));

	SetLayout();
	connections();

	axial_rect_ = window_axial_->rect();
	sagittal_rect_ = window_sagittal_->rect();
	coronal_rect_ = window_coronal_->rect();
	vr_rect_ = window_vr_->rect();
	vr_zoom_3d_rect_ = window_vr_zoom_3d_->rect();

	BaseTab::set_initialized(true);
}

void CW3MPRtab::connections()
{
	const MPRTaskTool *task_tool = task_tool_.get();
	connect(task_tool, &MPRTaskTool::sigMPRAutoReorient, this, &CW3MPRtab::slotAutoReorient);
	connect(task_tool, &MPRTaskTool::sigAdjustOrientationClicked, this, &CW3MPRtab::slotAdjustOrientationClicked);
	connect(task_tool, &MPRTaskTool::sigMPRReorientReset, m_pMPRViewMgr, &CW3MPRViewMgr::slotReorientReset);
	connect(task_tool, &MPRTaskTool::sigRotateOrientation, m_pMPRViewMgr, &CW3MPRViewMgr::slotRotateOrientationFromTaskTool);
	connect(task_tool, &MPRTaskTool::sigMPRTask, this, &CW3MPRtab::slotMPRTask);
	connect(task_tool, &MPRTaskTool::sigMPRVisible, m_pMPRViewMgr, &CW3MPRViewMgr::slotMPRVisible);
	connect(task_tool, &MPRTaskTool::sigMPRChangeFaceTransparency, m_pMPRViewMgr, &CW3MPRViewMgr::slotMPRChangeFaceTransparency);
	connect(task_tool, &MPRTaskTool::sigMPRClipEnable, m_pMPRViewMgr, &CW3MPRViewMgr::slotMPRClipEnable);
	connect(task_tool, &MPRTaskTool::sigMPRClipRangeMove, m_pMPRViewMgr, &CW3MPRViewMgr::slotMPRClipRangeMove);
	connect(task_tool, &MPRTaskTool::sigMPRClipRangeSet, m_pMPRViewMgr, &CW3MPRViewMgr::slotMPRClipRangeSet);
	connect(task_tool, &MPRTaskTool::sigMPRClipPlaneChanged, m_pMPRViewMgr, &CW3MPRViewMgr::slotMPRClipPlaneChanged);
	connect(task_tool, &MPRTaskTool::sigFlipClipping, m_pMPRViewMgr, &CW3MPRViewMgr::slotFlipClipping);
	connect(task_tool, &MPRTaskTool::sigAdjustImplantButtonToggled, m_pMPRViewMgr, &CW3MPRViewMgr::slotAdjustImplantButtonToggled);

	connect(window_axial_.get(), &WindowMPR::sigChangeThickness, this, &CW3MPRtab::slotChangeThickness);
	connect(window_sagittal_.get(), &WindowMPR::sigChangeThickness, this, &CW3MPRtab::slotChangeThickness);
	connect(window_coronal_.get(), &WindowMPR::sigChangeThickness, this, &CW3MPRtab::slotChangeThickness);

	connect(window_axial_.get(), &WindowMPR::sigChangeInterval, this, &CW3MPRtab::slotChangeInterval);
	connect(window_sagittal_.get(), &WindowMPR::sigChangeInterval, this, &CW3MPRtab::slotChangeInterval);
	connect(window_coronal_.get(), &WindowMPR::sigChangeInterval, this, &CW3MPRtab::slotChangeInterval);

	connect(window_axial_.get(), &WindowMPR::sigMaximizeOnOff, this, &CW3MPRtab::slotMaximizeOnOff);
	connect(window_sagittal_.get(), &WindowMPR::sigMaximizeOnOff, this, &CW3MPRtab::slotMaximizeOnOff);
	connect(window_coronal_.get(), &WindowMPR::sigMaximizeOnOff, this, &CW3MPRtab::slotMaximizeOnOff);
	connect(window_vr_.get(), &WindowMPR3D::sigMaximizeOnOff, this, &CW3MPRtab::slotMaximizeOnOff);

	connect(window_axial_.get(), &WindowMPR::sigLightboxOn,
					[=](int row, int col, float interval, float thickness)
					{
						SetLightboxOn(MPRViewType::AXIAL, row, col, interval, thickness);
					});
	connect(window_sagittal_.get(), &WindowMPR::sigLightboxOn,
					[=](int row, int col, float interval, float thickness)
					{
						SetLightboxOn(MPRViewType::SAGITTAL, row, col, interval, thickness);
					});
	connect(window_coronal_.get(), &WindowMPR::sigLightboxOn,
					[=](int row, int col, float interval, float thickness)
					{
						SetLightboxOn(MPRViewType::CORONAL, row, col, interval, thickness);
					});

	connect(window_vr_.get(), &WindowMPR3D::sig3DCutReset, m_pMPRViewMgr, &CW3MPRViewMgr::slot3DCutReset);
	connect(window_vr_.get(), &WindowMPR3D::sig3DCutUndo, m_pMPRViewMgr, &CW3MPRViewMgr::slot3DCutUndo);
	connect(window_vr_.get(), &WindowMPR3D::sig3DCutRedo, m_pMPRViewMgr, &CW3MPRViewMgr::slot3DCutRedo);

	connect(m_pMPRViewMgr, &CW3MPRViewMgr::sigThicknessChanged, this, &CW3MPRtab::slotSyncThicknessChanged);

	connect(m_pgRcontainer, &CW3ResourceContainer::sigInitFacePhoto3D, this, &CW3MPRtab::slotFaceEnabled);

	connect(m_pMPRViewMgr, &CW3MPRViewMgr::sigMPROverlayOn, this, &CW3MPRtab::slotMPROverlayOn);

	connect(m_pMPRViewMgr, SIGNAL(sigSave3DFaceToPLYFile()), this, SIGNAL(sigSave3DFaceToPLYFile()));
	connect(m_pMPRViewMgr, SIGNAL(sigSave3DFaceToOBJFile()), this, SIGNAL(sigSave3DFaceToOBJFile()));

#if 0
	connect(m_pMPRViewMgr->GetViewMPR(MPRViewType::AXIAL), SIGNAL(sigKeyPressEvent(QKeyEvent*)), this, SLOT(slotKeyPressEventFromView(QKeyEvent*)));
	connect(m_pMPRViewMgr->GetViewMPR(MPRViewType::SAGITTAL), SIGNAL(sigKeyPressEvent(QKeyEvent*)), this, SLOT(slotKeyPressEventFromView(QKeyEvent*)));
	connect(m_pMPRViewMgr->GetViewMPR(MPRViewType::CORONAL), SIGNAL(sigKeyPressEvent(QKeyEvent*)), this, SLOT(slotKeyPressEventFromView(QKeyEvent*)));
#endif
	connect(m_pMPRViewMgr->GetViewVR(), SIGNAL(sigKeyPressEvent(QKeyEvent *)), this, SLOT(slotKeyPressEventFromView(QKeyEvent *)));

	connect(m_pMPRViewMgr,
					SIGNAL(sigUpdateArch(ArchTypeID, std::vector<glm::vec3>, glm::mat4, int)),
					this,
					SIGNAL(sigUpdateArch(ArchTypeID, std::vector<glm::vec3>, glm::mat4, int)));

	connect(orientation_dlg_.get(), SIGNAL(sigGridOnOff(bool)), m_pMPRViewMgr, SLOT(slotGridOnOffOrientation(bool)));
	connect(orientation_dlg_.get(), SIGNAL(sigResetOrientation()), m_pMPRViewMgr, SLOT(slotReorientReset()));

	connect(m_pMPRViewMgr, &CW3MPRViewMgr::sigSendMPRPlaneInfo, this, &CW3MPRtab::sigSendMPRPlaneInfo);
	connect(m_pMPRViewMgr, &CW3MPRViewMgr::sigCreateDCMFiles_ushort, this, &CW3MPRtab::sigCreateDCMFiles_ushort);
	connect(m_pMPRViewMgr, &CW3MPRViewMgr::sigCreateDCMFiles_uchar, this, &CW3MPRtab::sigCreateDCMFiles_uchar);

#ifdef WILL3D_EUROPE
	connect(m_pMPRViewMgr, &CW3MPRViewMgr::sigShowButtonListDialog, this, &CW3MPRtab::sigShowButtonListDialog);
#endif // WILL3D_EUROPE
}

CW3MPRtab::MaximizedViewType CW3MPRtab::GetMaximizedViewType()
{
	MaximizedViewType sender_view_type;
	QObject *sender = QObject::sender();
	if (!sender)
		return MaximizedViewType::VR;

	if (sender == window_axial_.get())
		sender_view_type = MaximizedViewType::AXIAL;
	else if (sender == window_sagittal_.get())
		sender_view_type = MaximizedViewType::SAGITTAL;
	else if (sender == window_coronal_.get())
		sender_view_type = MaximizedViewType::CORONAL;
	else if (sender == window_vr_.get() || sender == window_vr_zoom_3d_.get())
		sender_view_type = MaximizedViewType::VR;
	else
		sender_view_type =
				MaximizedViewType::VR; // vr cut 占쏙옙튼占쏙옙 占쏙옙占쏙옙占쏙옙 占쏙옙占?占쏙옙占쏙옙占?占쏙옙占승댐옙.

	if (maximized_view_type_ == sender_view_type)
		return MaximizedViewType::NONE;

	return sender_view_type;
}

void CW3MPRtab::SetLayout()
{
	SAFE_DELETE_OBJECT(view_layout_);

	switch (layout_mode_)
	{
	case LayoutMode::DEFAULT:
		SetLayoutDefault();
		break;
	case LayoutMode::MAXIMIZE:
		SetLayoutMaximize();
		break;
	case LayoutMode::ONLY_TRD:
		SetLayoutTRD();
		break;
	case LayoutMode::LIGHTBOX:
		SetLayoutLightbox();
		break;
	}

	tab_layout_ = view_layout_;
}

void CW3MPRtab::SetLayoutTRD()
{
	view_layout_ = new QHBoxLayout;
	view_layout_->addWidget(window_vr_.get());

	m_pMPRViewMgr->setOnlyTRDMode();
}

void CW3MPRtab::SetLayoutMaximize()
{
	Window *pgChangeWindow = nullptr;
	if (task_tool_->IsZoom3DOn())
		pgChangeWindow = window_vr_zoom_3d_.get();
	else
		pgChangeWindow = window_coronal_.get();

	Window *pgMaximizedWindow = nullptr;
	Window *pgNormalWindow1 = nullptr;
	Window *pgNormalWindow2 = nullptr;
	Window *pgNormalWindow3 = nullptr;

	switch (maximized_view_type_)
	{
	case MaximizedViewType::VR:
		pgMaximizedWindow = window_vr_.get();
		pgNormalWindow1 = window_axial_.get();
		pgNormalWindow2 = pgChangeWindow;
		pgNormalWindow3 = window_sagittal_.get();
		break;
	case MaximizedViewType::AXIAL:
		pgMaximizedWindow = window_axial_.get();
		pgNormalWindow1 = pgChangeWindow;
		pgNormalWindow2 = window_sagittal_.get();
		pgNormalWindow3 = window_vr_.get();
		break;
	case MaximizedViewType::CORONAL:
		pgMaximizedWindow = pgChangeWindow;
		pgNormalWindow1 = window_axial_.get();
		pgNormalWindow2 = window_sagittal_.get();
		pgNormalWindow3 = window_vr_.get();
		break;
	case MaximizedViewType::SAGITTAL:
		pgMaximizedWindow = window_sagittal_.get();
		pgNormalWindow1 = window_axial_.get();
		pgNormalWindow2 = pgChangeWindow;
		pgNormalWindow3 = window_vr_.get();
		break;
	default:
		break;
	}

	pgMaximizedWindow->SetMaximize(true);
	pgNormalWindow1->SetMaximize(false);
	pgNormalWindow2->SetMaximize(false);
	pgNormalWindow3->SetMaximize(false);

	QVBoxLayout *left_layout = new QVBoxLayout;
	left_layout->setSpacing(kLayoutSpacing);
	left_layout->addWidget(pgMaximizedWindow);
	left_layout->setStretch(0, 1);

	QVBoxLayout *right_layout = new QVBoxLayout;
	right_layout->setSpacing(kLayoutSpacing);
	right_layout->addWidget(pgNormalWindow1);
	right_layout->setStretch(0, 1);
	right_layout->addWidget(pgNormalWindow2);
	right_layout->setStretch(1, 1);
	right_layout->addWidget(pgNormalWindow3);
	right_layout->setStretch(2, 1);

	view_layout_ = new QHBoxLayout();
	view_layout_->setSpacing(kLayoutSpacing);

	view_layout_->addLayout(left_layout);
	view_layout_->addLayout(right_layout);
	if (maximized_view_type_ == MaximizedViewType::VR && hide_mpr_views_on_maximized_vr_layout_)
	{
		view_layout_->setStretch(0, 0);
		view_layout_->setStretch(1, 0);
	}
	else
	{
		view_layout_->setStretch(0, kMaximizedViewStretch);
		view_layout_->setStretch(1, kNormalViewStretch);
	}
}

void CW3MPRtab::SetLayoutDefault()
{
	Window *pgChangeWindow = nullptr;
	if (task_tool_->IsZoom3DOn())
		pgChangeWindow = window_vr_zoom_3d_.get();
	else
		pgChangeWindow = window_coronal_.get();

	QVBoxLayout *left_layout = new QVBoxLayout;
	left_layout->setSpacing(kLayoutSpacing);
	left_layout->addWidget(window_axial_.get());
	left_layout->setStretch(0, 1);
	left_layout->addWidget(pgChangeWindow);
	left_layout->setStretch(1, 1);

	QVBoxLayout *right_layout = new QVBoxLayout;
	right_layout->setSpacing(kLayoutSpacing);
	right_layout->addWidget(window_sagittal_.get());
	right_layout->setStretch(0, 1);
	right_layout->addWidget(window_vr_.get());
	right_layout->setStretch(1, 1);

	view_layout_ = new QHBoxLayout;
	view_layout_->setSpacing(kLayoutSpacing);
	view_layout_->addLayout(left_layout);
	view_layout_->setStretch(0, 1);
	view_layout_->addLayout(right_layout);
	view_layout_->setStretch(1, 1);
}

void CW3MPRtab::SetLayoutLightbox()
{
	view_layout_ = new QHBoxLayout;
	view_layout_->setSpacing(kLayoutSpacing);
	view_layout_->addWidget(window_lightbox_);
}

void CW3MPRtab::SetVisibleWindowsTRD(bool visible)
{
	window_vr_->setVisible(visible);
	m_pMPRViewMgr->GetViewVR()->setVisible(visible);
}

void CW3MPRtab::SetVisibleWindowsDefaultAndMaximize(bool visible)
{
  bool is_zoom_3d_on = task_tool_->IsZoom3DOn();
  const QPoint offScreen(-10000, -10000);

  if (visible)
  {
    window_axial_->move(axial_rect_.topLeft());
    window_axial_->resize(axial_rect_.size());
    window_axial_->raise();

    window_sagittal_->move(sagittal_rect_.topLeft());
    window_sagittal_->resize(sagittal_rect_.size());
    window_sagittal_->raise();

    if (is_zoom_3d_on)
    {
      window_coronal_->move(offScreen);

      window_vr_zoom_3d_->move(vr_zoom_3d_rect_.topLeft());
      window_vr_zoom_3d_->resize(vr_zoom_3d_rect_.size());
      window_vr_zoom_3d_->raise();
    }
    else
    {
      window_coronal_->move(coronal_rect_.topLeft());
      window_coronal_->resize(coronal_rect_.size());
      window_coronal_->raise();

      window_vr_zoom_3d_->move(offScreen);
    }

    window_vr_->move(vr_rect_.topLeft());
    window_vr_->resize(vr_rect_.size());
    window_vr_->raise();
  }
  else
  {
    window_axial_->move(offScreen);
    window_sagittal_->move(offScreen);
    window_coronal_->move(offScreen);
    window_vr_->move(offScreen);
    window_vr_zoom_3d_->move(offScreen);
  }
	/*bool visible_at_maximized_vr = !(maximized_view_type_ == MaximizedViewType::VR && hide_mpr_views_on_maximized_vr_layout_);

	bool coronal_visible = !task_tool_->IsZoom3DOn() && visible && visible_at_maximized_vr;
	if (window_coronal_->isVisible() != coronal_visible)
	{
		window_coronal_->setVisible(coronal_visible);
		m_pMPRViewMgr->setVisibleMPRView(MPRViewType::CORONAL, coronal_visible);
	}

	bool zoom_3d_visible = task_tool_->IsZoom3DOn() && visible;
	if (window_vr_zoom_3d_->isVisible() != zoom_3d_visible)
	{
		window_vr_zoom_3d_->setVisible(zoom_3d_visible);
		m_pMPRViewMgr->GetViewZoom3D()->setVisible(zoom_3d_visible);
	}

	bool axial_visible = visible && visible_at_maximized_vr;
	if (window_axial_->isVisible() != axial_visible)
	{
		window_axial_->setVisible(axial_visible);
		m_pMPRViewMgr->setVisibleMPRView(MPRViewType::AXIAL, axial_visible);
	}

	bool sagittal_visible = visible && visible_at_maximized_vr;
	if (window_sagittal_->isVisible() != sagittal_visible)
	{
		window_sagittal_->setVisible(sagittal_visible);
		m_pMPRViewMgr->setVisibleMPRView(MPRViewType::SAGITTAL, sagittal_visible);
	}

	if (window_vr_->isVisible() != visible)
	{
		window_vr_->setVisible(visible);
		m_pMPRViewMgr->GetViewVR()->setVisible(visible);
	}*/
}

void CW3MPRtab::SetVisibleWindowsLightbox(bool visible)
{
	window_lightbox_->setVisible(visible);
	m_pMPRViewMgr->setVisibleLightboxViews(visible);
}

void CW3MPRtab::SyncStatusMPRmenus()
{
	task_tool_->SyncVisibilityResources();
}

void CW3MPRtab::SetVisibleWindows(bool visible)
{
	if (!initialized())
		return;

	if (visible)
		this->SyncStatusMPRmenus();

	is_visible_ = visible;

	switch (layout_mode_)
	{
	case LayoutMode::DEFAULT:
	case LayoutMode::MAXIMIZE:
		SetVisibleWindowsDefaultAndMaximize(visible);
		break;
	case LayoutMode::ONLY_TRD:
		SetVisibleWindowsTRD(visible);
		break;
	case LayoutMode::LIGHTBOX:
		SetVisibleWindowsLightbox(visible);
		break;
	}
}

void CW3MPRtab::activate(const float &pixel_spacing, const float &slice_spacing)
{
	if (m_pMPRViewMgr)
	{
		m_pMPRViewMgr->activate();
	}

#if 0
	window_axial_->InitInterval(slice_spacing);		//slice_spacing???
	window_coronal_->InitInterval(pixel_spacing);
	window_sagittal_->InitInterval(pixel_spacing);
#else
	double default_interval = GlobalPreferences::GetInstance()->preferences_.advanced.mpr.default_interval;
	float interval = 0.f;
	if (static_cast<int>(default_interval) == 0)
	{
		interval = pixel_spacing;
	}
	else
	{
		interval = static_cast<float>(default_interval);
	}

	window_axial_->InitInterval(interval, pixel_spacing);
	window_coronal_->InitInterval(interval, pixel_spacing);
	window_sagittal_->InitInterval(interval, pixel_spacing);
#endif
}

#ifndef WILL3D_VIEWER
void CW3MPRtab::exportProject(ProjectIOMPR &out)
{
	if (m_pMPRViewMgr)
		m_pMPRViewMgr->exportProject(out);
}

void CW3MPRtab::importProject(ProjectIOMPR &in)
{
	if (in.IsInit())
	{
		if (!initialized())
		{
			Initialize();
		}
		m_pMPRViewMgr->importProject(in);
	}
}
#endif
void CW3MPRtab::SetCommonToolOnce(const common::CommonToolTypeOnce &type,
																	bool on)
{
	m_pMPRViewMgr->SetCommonToolOnce(type, on);
}

void CW3MPRtab::SetCommonToolOnOff(const common::CommonToolTypeOnOff &type)
{
	m_pMPRViewMgr->SetCommonToolOnOff(type);
}

void CW3MPRtab::UpdateVRview(bool is_high_quality)
{
	m_pMPRViewMgr->UpdateVRview(is_high_quality);
}

void CW3MPRtab::slotMaximizeOnOff(bool max)
{
	if (layout_mode_ == LayoutMode::ONLY_TRD)
	{
		return;
	}

	maximized_view_type_ = GetMaximizedViewType();

	if (maximized_view_type_ != MaximizedViewType::NONE)
	{
		layout_mode_ = LayoutMode::MAXIMIZE;
	}
	else
	{
		layout_mode_ = LayoutMode::DEFAULT;
	}

	SetLayout();
	SetVisibleWindowsDefaultAndMaximize(is_visible_);

	const auto &event_common_handler =
			EventHandler::GetInstance()->GetCommonEventHandle();
	event_common_handler.EmitSigSetTabSlotLayout(tab_layout_);

	if (maximized_view_type_ != MaximizedViewType::VR)
	{
#ifndef WILL3D_LIGHT
		task_tool_->SyncTaskUI(MPRTaskID::CUT3D, false);
#endif
		m_pMPRViewMgr->setMaximizeVRView(false);
	}
}

void CW3MPRtab::TaskCut3D(bool on)
{
	if (on)
	{
		if (maximized_view_type_ != MaximizedViewType::VR)
			slotMaximizeOnOff(on);
	}
	window_vr_->Set3DCutMode(on);
	m_pMPRViewMgr->Task3DCut(on, task_tool_->cut_tool());
}

void CW3MPRtab::TaskOblique(bool on)
{
	if (on)
	{
		window_vr_->Set3DCutMode(false);
	}
	m_pMPRViewMgr->TaskOblique(on);
}

void CW3MPRtab::TaskSTLExport()
{
	m_pMPRViewMgr->TaskSTLExport(task_tool_->IsClipOn());
}

void CW3MPRtab::TaskDrawArch()
{
	m_pMPRViewMgr->TaskDrawArch();
}

void CW3MPRtab::SetLightboxOn(const MPRViewType &view_type, int row, int col,
															float interval, float thickness)
{
	// initalize parameters
	lightbox_resource::PlaneParams plane_params;
	m_pMPRViewMgr->GetLightboxPlaneParams(view_type, plane_params);

	lightbox_resource::LightboxParams lightbox_params;
	lightbox_params.count_row = row;
	lightbox_params.count_col = col;
	lightbox_params.interval = interval;
	lightbox_params.thickness = thickness;
	switch (view_type)
	{
	case MPRViewType::AXIAL:
		lightbox_params.view_type = LightboxViewType::AXIAL;
		break;
	case MPRViewType::SAGITTAL:
		lightbox_params.view_type = LightboxViewType::SAGITTAL;
		break;
	case MPRViewType::CORONAL:
		lightbox_params.view_type = LightboxViewType::CORONAL;
		break;
	default:
		break;
	}

	// Create MPR Module for light box
	mpr_engine_.reset(new MPREngine);
	mpr_engine_->Initialize(*m_pgMPRengine->getMPRrotCenterInVol(0));
	mpr_engine_->InitLightboxResource(
			m_pMPRViewMgr->GetLightboxPlaneCenter(view_type), lightbox_params,
			plane_params);

	// Create Light box views
	m_pMPRViewMgr->CreateLightboxViews(lightbox_params.view_type);
	std::vector<int> slider_positions;
	mpr_engine_->GetLightboxSliderPositions(slider_positions);
	m_pMPRViewMgr->InitLightboxSlider(row, col, slider_positions,
																		plane_params.available_depth);

	// Create Lightbox UI
	window_lightbox_ = new WindowLightbox(view_type, interval, thickness);
	window_lightbox_->SetViews(m_pMPRViewMgr->GetViewLightbox());
	window_lightbox_->SetViewLayoutCount(row, col);
	window_lightbox_->InitInterval(
			ResourceContainer::GetInstance()->GetMainVolume().pixelSpacing());

	this->ConnectLightbox();

	// layout mode 占쏙옙占쏙옙 占쏙옙占쏙옙 占쏙옙占쏙옙占쏙옙 占쏙옙占싱아울옙占쏙옙 window 占쏙옙 view占쏙옙占쏙옙 invisible
	SetVisibleWindows(false);
	layout_mode_ = LayoutMode::LIGHTBOX;
	SetLayout();

	// Hide MPR Tool & Set Lightbox Layout
	task_tool_->SetLightboxOn(true);
	emit sigMPRLightboxOn();

	// Render and Display Light box images
	m_pMPRViewMgr->UpdateLightboxViews();
}

void CW3MPRtab::SyncLightboxSliderPositions()
{
	std::vector<int> slider_positions;
	mpr_engine_->GetLightboxSliderPositions(slider_positions);
	m_pMPRViewMgr->SyncLightboxSliderPositions(slider_positions);
}

void CW3MPRtab::slotSetLightboxOff()
{
	SetVisibleWindowsLightbox(false);
	layout_mode_ = LayoutMode::DEFAULT;
	SetLayout();

	this->DisconnectLightbox();
	mpr_engine_.reset();
	m_pMPRViewMgr->DeleteLightboxViews();
	SAFE_DELETE_OBJECT(window_lightbox_);

	EventHandler::GetInstance()->GetMeasureEventHandle().EmitSigMeasureDeleteAll(
			common::ViewTypeID::LIGHTBOX);

	// Show MPR Tool & Set Lightbox Layout
	task_tool_->SetLightboxOn(false);
	emit sigMPRLightboxOn();
}

void CW3MPRtab::slotChangeLightboxInterval(double real_interval)
{
	mpr_engine_->SetLightboxInterval(real_interval);
	SyncLightboxSliderPositions();
	m_pMPRViewMgr->UpdateLightboxViews();
}

void CW3MPRtab::slotChangeLightboxThickness(double real_thickness)
{
	mpr_engine_->SetLightboxThickness(real_thickness);
	m_pMPRViewMgr->UpdateLightboxViews();
}

void CW3MPRtab::ConnectLightbox()
{
	connect(m_pMPRViewMgr,
					static_cast<void (CW3MPRViewMgr::*)(const int &, const int &)>(
							&CW3MPRViewMgr::sigLightboxTranslate),
					this,
					static_cast<void (CW3MPRtab::*)(const int &, const int &)>(
							&CW3MPRtab::slotLightboxTransltate));
	connect(m_pMPRViewMgr,
					static_cast<void (CW3MPRViewMgr::*)(const float &)>(
							&CW3MPRViewMgr::sigLightboxTranslate),
					this,
					static_cast<void (CW3MPRtab::*)(const float &)>(
							&CW3MPRtab::slotLightboxTransltate));
	connect(m_pMPRViewMgr, &CW3MPRViewMgr::sigGetLightboxDICOMInfo, this,
					&CW3MPRtab::slotGetLightboxDICOMInfo);
	connect(m_pMPRViewMgr, &CW3MPRViewMgr::sigGetLightboxProfileData, this,
					&CW3MPRtab::slotGetLightboxProfileData);
	connect(m_pMPRViewMgr, &CW3MPRViewMgr::sigGetLightboxROIData, this,
					&CW3MPRtab::slotGetLightboxROIData);
	connect(m_pMPRViewMgr, &CW3MPRViewMgr::sigLightboxMaximize, this,
					&CW3MPRtab::slotLightboxMaximize);

	connect(window_lightbox_, &WindowLightbox::sigChangeLightboxLayout,
					this, &CW3MPRtab::slotSetLightboxLayout);
	connect(window_lightbox_, &WindowLightbox::sigWindowClose, this,
					&CW3MPRtab::slotSetLightboxOff);
	connect(window_lightbox_, &WindowLightbox::sigChangeInterval, this,
					&CW3MPRtab::slotChangeLightboxInterval);
	connect(window_lightbox_, &WindowLightbox::sigChangeThickness, this,
					&CW3MPRtab::slotChangeLightboxThickness);
}

void CW3MPRtab::DisconnectLightbox()
{
	disconnect(m_pMPRViewMgr,
						 static_cast<void (CW3MPRViewMgr::*)(const int &, const int &)>(
								 &CW3MPRViewMgr::sigLightboxTranslate),
						 this,
						 static_cast<void (CW3MPRtab::*)(const int &, const int &)>(
								 &CW3MPRtab::slotLightboxTransltate));
	disconnect(m_pMPRViewMgr,
						 static_cast<void (CW3MPRViewMgr::*)(const float &)>(
								 &CW3MPRViewMgr::sigLightboxTranslate),
						 this,
						 static_cast<void (CW3MPRtab::*)(const float &)>(
								 &CW3MPRtab::slotLightboxTransltate));
	disconnect(m_pMPRViewMgr, &CW3MPRViewMgr::sigGetLightboxDICOMInfo, this,
						 &CW3MPRtab::slotGetLightboxDICOMInfo);
	disconnect(m_pMPRViewMgr, &CW3MPRViewMgr::sigGetLightboxProfileData, this,
						 &CW3MPRtab::slotGetLightboxProfileData);
	disconnect(m_pMPRViewMgr, &CW3MPRViewMgr::sigGetLightboxROIData, this,
						 &CW3MPRtab::slotGetLightboxROIData);
	disconnect(m_pMPRViewMgr, &CW3MPRViewMgr::sigLightboxMaximize, this,
						 &CW3MPRtab::slotLightboxMaximize);

	disconnect(window_lightbox_, &WindowLightbox::sigChangeLightboxLayout,
						 this, &CW3MPRtab::slotSetLightboxLayout);
	disconnect(window_lightbox_, &WindowLightbox::sigWindowClose, this,
						 &CW3MPRtab::slotSetLightboxOff);
	disconnect(window_lightbox_, &WindowLightbox::sigChangeInterval, this,
						 &CW3MPRtab::slotChangeLightboxInterval);
	disconnect(window_lightbox_, &WindowLightbox::sigChangeThickness, this,
						 &CW3MPRtab::slotChangeLightboxThickness);
}

#ifdef WILL3D_EUROPE
void CW3MPRtab::SyncControlButtonOut()
{
	m_pMPRViewMgr->SetSyncControlButtonOut();
}
#endif // WILL3D_EUROPE

void CW3MPRtab::TaskZoom3D(bool bToggled)
{
	if (bToggled)
	{
		emit sigCommonToolCancelSelected();
		window_vr_->Set3DCutMode(false);
		hide_mpr_views_on_maximized_vr_layout_ = false;
	}

	SetLayout();
	const auto &event_common_handler = EventHandler::GetInstance()->GetCommonEventHandle();
	event_common_handler.EmitSigSetTabSlotLayout(tab_layout_);

  if (is_visible_)
  {
    bool is_zoom_3d_on = task_tool_->IsZoom3DOn();

    // window_coronal_->setVisible(!is_zoom_3d_on);
    // m_pMPRViewMgr->setVisibleMPRView(MPRViewType::CORONAL, !is_zoom_3d_on);
    const QPoint offScreen(-10000, -10000);
    if (is_zoom_3d_on)
    {
      window_coronal_->move(offScreen);
    }
    else
    {
      window_coronal_->move(coronal_rect_.topLeft());
      window_coronal_->resize(coronal_rect_.size());
      window_coronal_->raise();
    }

    if (!is_zoom_3d_on)
    {
      m_pMPRViewMgr->updateImageOnViewPlane(MPRViewType::CORONAL);
    }

    // window_vr_zoom_3d_->setVisible(is_zoom_3d_on);
    // m_pMPRViewMgr->setVisibleZoomView(is_zoom_3d_on);
    if (is_zoom_3d_on)
    {
      window_vr_zoom_3d_->move(vr_zoom_3d_rect_.topLeft());
      window_vr_zoom_3d_->resize(vr_zoom_3d_rect_.size());
      window_vr_zoom_3d_->raise();
    }
    else
    {
      window_vr_zoom_3d_->move(offScreen);
    }
  }

	m_pMPRViewMgr->TaskZoom3D(bToggled);

	if (bToggled)
	{
		SetVisibleWindowsDefaultAndMaximize(is_visible_);
	}
}

void CW3MPRtab::slotChangeThickness(double thickness)
{
	MPRViewType sender_view_type;
	QObject *sender = QObject::sender();
	if (sender == window_axial_.get())
		sender_view_type = MPRViewType::AXIAL;
	else if (sender == window_sagittal_.get())
		sender_view_type = MPRViewType::SAGITTAL;
	else if (sender == window_coronal_.get())
		sender_view_type = MPRViewType::CORONAL;
	else
		return;

	m_pMPRViewMgr->ChangeThicknessDirect(sender_view_type, (float)thickness);
}

void CW3MPRtab::slotChangeInterval(double interval)
{
	MPRViewType sender_view_type;
	QObject *sender = QObject::sender();
	if (sender == window_axial_.get())
		sender_view_type = MPRViewType::AXIAL;
	else if (sender == window_sagittal_.get())
		sender_view_type = MPRViewType::SAGITTAL;
	else if (sender == window_coronal_.get())
		sender_view_type = MPRViewType::CORONAL;
	else
		return;

	m_pMPRViewMgr->ChangeIntervalDirect(sender_view_type, (float)interval);
}

void CW3MPRtab::slotSyncThicknessChanged(const MPRViewType &view_type,
																				 float thickness)
{
	switch (view_type)
	{
	case MPRViewType::AXIAL:
		window_axial_->blockSignals(true);
		window_axial_->SyncThicknessValue(thickness);
		window_axial_->blockSignals(false);
		break;
	case MPRViewType::SAGITTAL:
		window_sagittal_->blockSignals(true);
		window_sagittal_->SyncThicknessValue(thickness);
		window_sagittal_->blockSignals(false);
		break;
	case MPRViewType::CORONAL:
		window_coronal_->blockSignals(true);
		window_coronal_->SyncThicknessValue(thickness);
		window_coronal_->blockSignals(false);
		break;
	default:
		break;
	}
}

void CW3MPRtab::slotAutoReorient()
{
#ifndef WILL3D_LIGHT
	m_pgJobMgr->runReorientation();
#endif
}

void CW3MPRtab::slotAdjustOrientationClicked()
{
	bool grid_on = orientation_dlg_->IsGridOn();
	m_pMPRViewMgr->slotGridOnOffOrientation(grid_on);
	orientation_dlg_->exec();
}

void CW3MPRtab::slotMPRTask(const MPRTaskID &task_id, bool on)
{
	switch (task_id)
	{
	case MPRTaskID::ZOOM3D:
		emit sigCommonToolCancelSelected();
		TaskZoom3D(on);
		break;
	case MPRTaskID::OBLIQUE:
		TaskOblique(on);
		break;
#ifndef WILL3D_LIGHT
	case MPRTaskID::CUT3D:
		emit sigCommonToolCancelSelected();
		TaskCut3D(on);
		break;
#endif
	case MPRTaskID::STLEXPORT:
		TaskSTLExport();
		break;
	case MPRTaskID::DRAW_ARCH:
		TaskDrawArch();
		break;
	default:
		break;
	}
}

void CW3MPRtab::slotLightboxTransltate(const int &lightbox_id,
																			 const int &slider_value)
{
	mpr_engine_->TranslateLightbox(lightbox_id, slider_value);
	SyncLightboxSliderPositions();
}

void CW3MPRtab::slotLightboxTransltate(const float &displacement)
{
	mpr_engine_->TranslateLightbox(displacement);
	SyncLightboxSliderPositions();
}

void CW3MPRtab::slotGetLightboxDICOMInfo(const int &lightbox_id,
																				 const QPointF &pt_lightbox_plane,
																				 glm::vec4 &vol_info)
{
	vol_info = mpr_engine_->MapLightboxPlaneToVol(lightbox_id, pt_lightbox_plane);
}

void CW3MPRtab::slotGetLightboxProfileData(const int &lightbox_id,
																					 const QPointF &start_pt_plane,
																					 const QPointF &end_pt_plane,
																					 std::vector<short> &data)
{
	mpr_engine_->GetProfileDataInLightboxPlane(lightbox_id, start_pt_plane,
																						 end_pt_plane, data);
}

void CW3MPRtab::slotGetLightboxROIData(const int &lightbox_id,
																			 const QPointF &start_pt_plane,
																			 const QPointF &end_pt_plane,
																			 std::vector<short> &data)
{
	mpr_engine_->GetROIDataInLightboxPlane(lightbox_id, start_pt_plane,
																				 end_pt_plane, data);
}

void CW3MPRtab::slotLightboxMaximize(const int &lightbox_id)
{
	if (!mpr_engine_->IsLightboxAvailableMaximize())
		return;

	bool is_maximize = mpr_engine_->SetLightboxToMaximizeMode(lightbox_id);

	int lightbox_cnt_row, lightbox_cnt_col;
	mpr_engine_->GetLightboxCount(lightbox_cnt_row, lightbox_cnt_col);

	if (is_maximize)
	{
		auto view = m_pMPRViewMgr->GetViewLightboxMaximize(lightbox_id);
		window_lightbox_->SetViews({view}, 1, 1);
	}
	else
	{
		window_lightbox_->SetViews(m_pMPRViewMgr->GetViewLightbox(),
															 lightbox_cnt_row, lightbox_cnt_col);
	}

	std::vector<int> slider_positions;
	mpr_engine_->GetLightboxSliderPositions(slider_positions);
	m_pMPRViewMgr->InitLightboxSlider(lightbox_cnt_row, lightbox_cnt_col,
																		slider_positions,
																		mpr_engine_->GetLightboxAvailableDepth());

	m_pMPRViewMgr->UpdateLightboxViews();
}

void CW3MPRtab::slotSetLightboxLayout(const int &row, const int &col)
{
	mpr_engine_->ChangeLightboxCount(row, col);

	std::vector<int> slider_positions;
	mpr_engine_->GetLightboxSliderPositions(slider_positions);
	m_pMPRViewMgr->InitLightboxSlider(row, col, slider_positions,
																		mpr_engine_->GetLightboxAvailableDepth());

	// Render and Display Light box images
	m_pMPRViewMgr->UpdateLightboxViews();
	m_pMPRViewMgr->SyncLightboxMeasureResource();
}

void CW3MPRtab::slotSecondDisabled(bool state, float *param)
{
#ifndef WILL3D_LIGHT
	task_tool_->SyncVisibilityEnable(VisibleID::SECONDVOLUME, state);
	m_pMPRViewMgr->secondDisabled(state, param);
#endif
}

void CW3MPRtab::slotSetTranslateMatSecondVolume(glm::mat4 *mat)
{
#ifndef WILL3D_LIGHT
	m_pMPRViewMgr->setTranslateMatSecondVolume(mat);
#endif
}

void CW3MPRtab::slotSetRotateMatSecondVolume(glm::mat4 *mat)
{
#ifndef WILL3D_LIGHT
	m_pMPRViewMgr->setRotateMatSecondVolume(mat);
#endif
}

void CW3MPRtab::slotSetAirway(std::vector<tri_STL> &tries)
{
#ifndef WILL3D_LIGHT
	m_pMPRViewMgr->SetAirway(tries);
#endif
}

void CW3MPRtab::slotAirwayEnable(bool b)
{
#ifndef WILL3D_LIGHT
	task_tool_->SyncVisibilityEnable(VisibleID::AIRWAY, b);
	if (!b)
		task_tool_->SetAirwayEnable(b);
#endif
}

void CW3MPRtab::slotSetAirwaySize(double sz)
{
#ifndef WILL3D_LIGHT
	task_tool_->SetAirwaySize(sz);
#endif
}

void CW3MPRtab::slotFaceEnabled()
{
#ifndef WILL3D_LIGHT
	task_tool_->SyncVisibilityEnable(VisibleID::FACEPHOTO, true);
	m_pMPRViewMgr->slotUpdateMPRPhoto();
#endif
}

void CW3MPRtab::slotPointModel(glm::mat4 *mat)
{
#ifndef WILL3D_LIGHT
	if (!initialized())
		Initialize();
	m_pMPRViewMgr->pointModel(mat);
#endif
}

#ifndef WILL3D_VIEWER
void CW3MPRtab::slotRequestedCreateLightBoxDCMFiles(const bool nerve_visible, const bool implant_visible, const int filter, const int thickness)
{
	m_pMPRViewMgr->RequestedCreateLightBoxDCMFiles(nerve_visible, implant_visible, filter, thickness);
}
#endif // !WILL3D_VIEWER

void CW3MPRtab::setOnlyTRDMode()
{
#ifndef WILL3D_LIGHT
	layout_mode_ = LayoutMode::ONLY_TRD;
#endif
}

void CW3MPRtab::setMIP(bool isMIP) { m_pMPRViewMgr->setMIP(isMIP); }

// Add Lightbox capture
QStringList CW3MPRtab::GetViewList()
{
	if (layout_mode_ == LayoutMode::LIGHTBOX)
	{
		return QStringList("Lightbox");
	}
	else if (task_tool_->IsZoom3DOn())
	{
		return QStringList{"Axial", "Sagittal", "3D Zoom", "3D"};
	}
	else
	{
		return QStringList{"Axial", "Sagittal", "Coronal", "3D"};
	}
}

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QSurfaceFormat>
#include <QWindow>
#include <QOpenGLContext>
#include <QOpenGLFramebufferObjectFormat>
#include <QOpenGLFramebufferObject>
#include <QOpenGLPaintDevice>
#include <QImageWriter>
#include <QOpenGLWidget>
QImage CW3MPRtab::GetScreenshot(int view_type)
{
	QWidget *source = GetScreenshotSource(view_type);
	return BaseTab::GetScreenshot(source);
}

QWidget *CW3MPRtab::GetScreenshotSource(int view_type)
{
#if 0
	QGraphicsView* view = (QGraphicsView*)m_pMPRViewMgr->GetViewVR();
	QGraphicsScene* scene = view->scene();

	QSurfaceFormat format;
	format.setMajorVersion(4);
	format.setMinorVersion(0);

	QWindow window;
	window.setSurfaceType(QWindow::OpenGLSurface);
	window.setFormat(format);
	window.create();

	QOpenGLContext context;
	context.setFormat(format);
	if (!context.create())
		qFatal("Cannot create the requested OpenGL context!");
	context.makeCurrent(&window);

	const QSize drawRectSize = view->rect().size();

	QOpenGLFramebufferObjectFormat fboFormat;
	fboFormat.setSamples(16);
	fboFormat.setAttachment(QOpenGLFramebufferObject::Depth);

	QOpenGLFramebufferObject fbo(drawRectSize, fboFormat);
	fbo.bind();

	QOpenGLPaintDevice device(view->rect().size());
	QPainter painter;
	painter.begin(&device);
	painter.setRenderHints(QPainter::Antialiasing | QPainter::HighQualityAntialiasing);

	painter.beginNativePainting();
	scene->render(&painter);
	painter.endNativePainting();

	painter.end();

	fbo.release();

	QImageWriter test_writer_1("./screenshot/test_1.bmp", "bmp");
	test_writer_1.write(fbo.toImage());

#if 0
	QImage img = ((QOpenGLWidget*)view->viewport())->grabFramebuffer();

	QImageWriter test_writer_2("./screenshot/test_2.png", "png");
	test_writer_2.write(img);
#endif
#endif

	QWidget *source = nullptr;

	if (layout_mode_ == LayoutMode::LIGHTBOX)
	{
		source = window_lightbox_;
	}
	else
	{
		switch (view_type)
		{
		case 1:
			source = window_axial_.get();
			break;
		case 2:
			source = window_sagittal_.get();
			break;
		case 3:
			if (task_tool_->IsZoom3DOn())
				source = window_vr_zoom_3d_.get();
			else
				source = window_coronal_.get();
			break;
		case 4:
			source = window_vr_.get();
			break;
		}
	}

	return source;
}

void CW3MPRtab::DeleteMeasureUI(const common::ViewTypeID &view_type,
																const unsigned int &measure_id)
{
	m_pMPRViewMgr->DeleteMeasureUI(view_type, measure_id);
}

void CW3MPRtab::MoveViewsToSelectedMeasure(const common::ViewTypeID &view_type,
																					 const unsigned int &measure_id)
{
	m_pMPRViewMgr->MoveViewsToSelectedMeasure(view_type, measure_id);
}

void CW3MPRtab::ApplyPreferences()
{
	if (m_pMPRViewMgr)
		m_pMPRViewMgr->ApplyPreferences();
}

void CW3MPRtab::slotMPROverlayOn()
{
#ifndef WILL3D_LIGHT
	task_tool_->SyncVisibilityChecked(VisibleID::FACEPHOTO, false);
#endif
}

void CW3MPRtab::slotKeyPressEventFromView(QKeyEvent *event)
{
	if (event->key() == Qt::Key_Space)
	{
		// hide mpr views when vr is maximized
		if (layout_mode_ != LayoutMode::MAXIMIZE ||
				maximized_view_type_ != MaximizedViewType::VR ||
				task_tool_->IsZoom3DOn())
		{
			return;
		}

		hide_mpr_views_on_maximized_vr_layout_ = !hide_mpr_views_on_maximized_vr_layout_;

		GlobalPreferences::GetInstance()->preferences_.advanced.mpr.hide_mpr_views_on_maximized_vr_layout = hide_mpr_views_on_maximized_vr_layout_;

		if (hide_mpr_views_on_maximized_vr_layout_)
		{
			view_layout_->setStretch(0, 0);
			view_layout_->setStretch(1, 0);
		}
		else
		{
			view_layout_->setStretch(0, kMaximizedViewStretch);
			view_layout_->setStretch(1, kNormalViewStretch);
		}

		SetVisibleWindowsDefaultAndMaximize(is_visible_);
	}
}

void CW3MPRtab::SetPanoEngine(const std::shared_ptr<PanoEngine> &pano_engine)
{
	pano_engine_ = pano_engine;
}

void CW3MPRtab::EmitSendMPRPlaneInfo(const MPRViewType mpr_view_type)
{
	m_pMPRViewMgr->EmitSendMPRPlaneInfo(mpr_view_type);
}

const lightbox_resource::PlaneParams CW3MPRtab::GetMPRPlaneParams(MPRViewType mpr_view_type)
{
	return m_pMPRViewMgr->GetMPRPlaneParams(mpr_view_type);
}

#ifndef WILL3D_VIEWER
void CW3MPRtab::RequestedGetLightBoxViewInfo(int &filter, int &thickness) const
{
	filter = m_pMPRViewMgr->GetLightBoxFilterValue();
	thickness = window_lightbox_->GetThicknessValue();
}
#endif // !WILL3D_VIEWER

#ifdef WILL3D_EUROPE
void CW3MPRtab::TaskSTLExportDialogOn()
{
	TaskSTLExport();
}

void CW3MPRtab::LightboxOn()
{
	MPRViewType mpr_type = m_pMPRViewMgr->GetMouseOverMPR();
	if (mpr_type == MPRViewType::MPR_UNKNOWN || mpr_type == MPRViewType::MPR_END)
	{
		return;
	}

	if (mpr_type == MPRViewType::AXIAL)
	{
		window_axial_.get()->ShowLayoutSelectionView(QCursor::pos());
	}
	else if (mpr_type == MPRViewType::SAGITTAL)
	{
		window_sagittal_.get()->ShowLayoutSelectionView(QCursor::pos());
	}
	else if (mpr_type == MPRViewType::CORONAL)
	{
		window_coronal_.get()->ShowLayoutSelectionView(QCursor::pos());
	}
}
#endif // WILL3D_EUROPE
