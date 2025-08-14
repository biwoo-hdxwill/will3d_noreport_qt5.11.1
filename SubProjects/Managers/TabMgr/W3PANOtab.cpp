#include "W3PANOtab.h"
/*=========================================================================

File:			class CW3PANOtab
Language:		C++11
Library:		Qt 5.2.0
Author:			Hong Jung
First date:		2015-12-02
Last modify:	2015-12-02

=========================================================================*/

#include <Engine/Common/Common/global_preferences.h>
#include <Engine/Common/Common/event_handle_common.h>
#include <Engine/Common/Common/event_handler.h>
#include <Engine/Common/Common/W3LayoutFunctions.h>
#include <Engine/Common/Common/W3Logger.h>
#include <Engine/Common/Common/W3Memory.h>

#include <Engine/Resource/ResContainer/resource_container.h>
#include <Engine/Resource/Resource/W3Image3D.h>

#include <Engine/Module/Panorama/pano_engine.h>

#include <Engine/UIModule/UITools/pano_task_tool.h>
#include <Engine/UIModule/UITools/tool_mgr.h>

#include <Engine/UIModule/UIFrame/orientation_dlg.h>
#include <Engine/UIModule/UIFrame/window_pano.h>
#include <Engine/UIModule/UIFrame/window_pano_arch.h>
#include <Engine/UIModule/UIFrame/window_pano_cross_section.h>

#include "pano_view_mgr.h"

CW3PANOtab::CW3PANOtab() {
  task_tool_.reset(new PanoTaskTool(this));
  ToolMgr::instance()->SetPanoTaskTool(task_tool_);

  window_pano_cross_section_.reset(new WindowPanoCrossSection());
  window_pano_cross_section_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
  window_pano_cross_section_->setVisible(false);
}

CW3PANOtab::~CW3PANOtab(void) {
  SAFE_DELETE_OBJECT(m_pTopLayout);
  SAFE_DELETE_OBJECT(m_pBottomLayout);
  SAFE_DELETE_OBJECT(m_pLayout);
}
#ifndef WILL3D_VIEWER
void CW3PANOtab::exportProject(ProjectIOPanorama& out) {
  if (pano_view_mgr_) pano_view_mgr_->exportProject(out);
}

void CW3PANOtab::importProject(ProjectIOPanorama& in_pano,
							   const bool& is_counterpart_exists) {
  if (!initialized()) {
	Initialize();
  }
  pano_view_mgr_->importProject(in_pano, is_counterpart_exists);
}
#endif
//20250123 LIN importProjectPanoTool 기능 viewer에 적용
void CW3PANOtab::importProjectPanoTool(ProjectIOPanoEngine& in_pano_engine) 
{
	task_tool_->ImportProject(in_pano_engine);
}
//========
void CW3PANOtab::SetPanoEngine(const std::shared_ptr<PanoEngine>& pano_engine) {
  pano_engine_ = pano_engine;
}

void CW3PANOtab::connections() 
{
  connect(task_tool_.get(), SIGNAL(sigPanoReorient()), this, SLOT(slotOrienAdjust()));
  connect(task_tool_.get(), SIGNAL(sigPanoReorientReset()), this, SLOT(slotResetOrientation()));
  connect(task_tool_.get(), SIGNAL(sigPanoTask(PanoTaskID)), this, SLOT(slotPanoTask(PanoTaskID)));
  connect(task_tool_.get(), SIGNAL(sigTaskResetAutoArchSliceNumber()), this, SLOT(slotTaskResetAutoArchSliceNumber()));

  connect(window_pano_cross_section_.get(), &WindowPanoCrossSection::sigMaximizeOnOff, this, &CW3PANOtab::slotMaximizeOnOff);

  connect(window_pano_.get(), &WindowPano::sigMaximizeOnOff, this, &CW3PANOtab::slotMaximizeOnOff);

  connect(window_pano_cross_section_.get(), &WindowPanoCrossSection::sigSelectLayout, this, &CW3PANOtab::slotSelectLayout);

  connect(orientation_dlg_.get(), SIGNAL(sigGridOnOff(bool)), this, SLOT(slotGridOnOffOrientation(bool)));
  connect(orientation_dlg_.get(), SIGNAL(sigResetOrientation()), this, SLOT(slotResetOrientation()));

  connect(pano_view_mgr_.get(), SIGNAL(sigNerveDrawModeOn()), this, SLOT(slotNerveDrawOn()));

  connect(pano_view_mgr_.get(), &BasePanoViewMgr::sigSyncCrossSectionParams, this, &CW3PANOtab::sigSyncCrossSectionParams);

  connect(pano_view_mgr_.get(), &PanoViewMgr::sigMaximizeSingleCrossSectionView, this, &CW3PANOtab::slotMaximizeSingleCrossSectionView);

  connect(pano_view_mgr_.get(), &PanoViewMgr::sigCreateDCMFiles_ushort, this, &CW3PANOtab::sigCreateDCMFiles_ushort);
  connect(pano_view_mgr_.get(), &PanoViewMgr::sigCreateDCMFiles_uchar, this, &CW3PANOtab::sigCreateDCMFiles_uchar);

#ifdef WILL3D_EUROPE
  connect(pano_view_mgr_.get(), &PanoViewMgr::sigShowButtonListDialog, this, &CW3PANOtab::sigShowButtonListDialog);
#endif // WILL3D_EUROPE
}

void CW3PANOtab::Initialize() {
  if (BaseTab::initialized()) {
	common::Logger::instance()->Print(common::LogType::ERR,
									  "already initialized.");
	assert(false);
  }

  /*window_pano_cross_section_.reset(new WindowPanoCrossSection());
  window_pano_cross_section_->setVisible(false);*/
  window_pano_cross_section_->SetIntervalMinimumValue(
	  ResourceContainer::GetInstance()->GetMainVolume().pixelSpacing());

  window_pano_arch_.reset(new WindowPanoArch());
  window_pano_arch_->setVisible(false);

  window_pano_.reset(new WindowPano());
  window_pano_->setVisible(false);

  orientation_dlg_.reset(new OrientationDlg());
  orientation_dlg_->setVisible(false);

  OrientationTool::OrientUI orient_ui;
  orient_ui.a = orientation_dlg_->GetOrienA();
  orient_ui.r = orientation_dlg_->GetOrienR();
  orient_ui.i = orientation_dlg_->GetOrienI();

  BasePanoTaskTool::CrossSectionUI cs_ui;
  cs_ui.angle = window_pano_cross_section_->GetRotate();
  cs_ui.interval = window_pano_cross_section_->GetInterval();
  cs_ui.thickness = window_pano_cross_section_->GetThickness();

  BasePanoTaskTool::PanoUI pano_ui;
  pano_ui.arch_range = window_pano_arch_->GetRange();
  pano_ui.thickness = window_pano_->GetThickness();

  task_tool_->InitExternUIs(orient_ui, cs_ui, pano_ui);

  pano_view_mgr_.reset(new PanoViewMgr());
  BaseTab::SetCastedViewMgr(
	  std::static_pointer_cast<BaseViewMgr>(pano_view_mgr_));

  pano_view_mgr_->set_init_arch_from_mpr(init_arch_from_mpr_);
  pano_view_mgr_->SetPanoMenu(task_tool_);
  pano_view_mgr_->SetPanoEngine(pano_engine_);

  window_pano_cross_section_->SetViews(pano_view_mgr_->GetViewPanoCrossSectionWidget());
  QSize default_layout = GlobalPreferences::GetInstance()->preferences_.advanced.cross_section_view.default_layout;
  window_pano_cross_section_->SetViewLayoutCount(default_layout.height(), default_layout.width());

  window_pano_arch_->SetView(pano_view_mgr_->GetViewPanoArchWidget());

  window_pano_->SetView(pano_view_mgr_->GetViewPanoWidget());

  orientation_dlg_->SetView(
	  pano_view_mgr_->GetViewPanoOrient(ReorientViewID::ORIENT_A),
	  pano_view_mgr_->GetViewPanoOrient(ReorientViewID::ORIENT_R),
	  pano_view_mgr_->GetViewPanoOrient(ReorientViewID::ORIENT_I));

  SetLayout();

  connections();

  BaseTab::set_initialized(true);
}

void CW3PANOtab::SetLayout() {
  SAFE_DELETE_OBJECT(m_pBottomLayout);
  SAFE_DELETE_OBJECT(m_pTopLayout);
  SAFE_DELETE_OBJECT(m_pLayout);

  m_pBottomLayout = new QHBoxLayout();
  m_pTopLayout = new QVBoxLayout();
  m_pLayout = new QVBoxLayout();

  switch (curr_layout_type_)
  {
  case CW3PANOtab::PanoLayoutType::DEFAULT:
	  top_layout_stretch_ = 1;
	  bottom_layout_stretch_ = 1;

	  m_pBottomLayout->setSpacing(kLayoutSpacing);
	  m_pBottomLayout->addWidget(window_pano_arch_.get());
	  m_pBottomLayout->setStretch(0, 1);
	  m_pBottomLayout->addWidget(window_pano_cross_section_.get());
	  m_pBottomLayout->setStretch(1, 2);

	  m_pTopLayout->setSpacing(kLayoutSpacing);
	  m_pTopLayout->addWidget(window_pano_.get());
	  m_pTopLayout->setStretch(0, 1);

	  m_pLayout->setSpacing(kLayoutSpacing);
	  m_pLayout->addLayout(m_pTopLayout);
	  m_pLayout->setStretch(0, top_layout_stretch_);
	  m_pLayout->addLayout(m_pBottomLayout);
	  m_pLayout->setStretch(1, bottom_layout_stretch_);
	  break;
  case CW3PANOtab::PanoLayoutType::PANORAMA_MAIN:
	  m_pLayout->addWidget(window_pano_.get());
	  break;
  case CW3PANOtab::PanoLayoutType::CROSS_SECTION_MAIN:
	  top_layout_stretch_ = 3;
	  bottom_layout_stretch_ = 2;

	  m_pBottomLayout->setSpacing(kLayoutSpacing);
	  m_pBottomLayout->addWidget(window_pano_arch_.get());
	  m_pBottomLayout->setStretch(0, 1);
	  m_pBottomLayout->addWidget(window_pano_.get());
	  m_pBottomLayout->setStretch(1, 2);

	  m_pTopLayout->setSpacing(kLayoutSpacing);
	  m_pTopLayout->addWidget(window_pano_cross_section_.get());
	  m_pTopLayout->setStretch(0, 1);

	  m_pLayout->setSpacing(kLayoutSpacing);
	  m_pLayout->addLayout(m_pTopLayout);
	  m_pLayout->setStretch(0, top_layout_stretch_);
	  m_pLayout->addLayout(m_pBottomLayout);
	  m_pLayout->setStretch(1, bottom_layout_stretch_);
	  break;
  default:
	  break;
  }

  tab_layout_ = m_pLayout;

}

void CW3PANOtab::SetVisibleWindows(bool isVisible)
{
	if (!initialized()) return;

	if (isVisible)
	{
		if (tab_changed_)
		{
			pano_view_mgr_->SyncMeasureResource();
		}
#if 0
		CW3LayoutFunctions::setVisibleWidgets(tab_layout_, isVisible);
#else
		if (single_cross_maximized_)
		{
			window_pano_cross_section_->setVisible(true);
			pano_view_mgr_->CheckCrossSectionMaximizeAlone();
		}
		else
		{
			CW3LayoutFunctions::setVisibleWidgets(tab_layout_, isVisible);
		}
#endif
		pano_view_mgr_->SyncPanoResource();
		task_tool_->SyncVisibilityResources();
	}
	else
	{
		window_pano_cross_section_->setVisible(isVisible);
		window_pano_arch_->setVisible(isVisible);
		window_pano_->setVisible(isVisible);
		pano_view_mgr_->SetVisibleViews(isVisible);
	}
}

CW3PANOtab::PanoLayoutType CW3PANOtab::GetMaximizedViewType()
{
	PanoLayoutType sender_view_type;
	QObject* sender = QObject::sender();

	if (sender == window_pano_cross_section_.get())
	{
		sender_view_type = PanoLayoutType::CROSS_SECTION_MAIN;
	}
	else if (sender == window_pano_arch_.get())
	{
		sender_view_type = PanoLayoutType::DEFAULT;
	}
	else if (sender == window_pano_.get())
	{
		sender_view_type = PanoLayoutType::PANORAMA_MAIN;
	}
	else
	{
		return PanoLayoutType::DEFAULT;
	}

	if (curr_layout_type_ == sender_view_type)
	{
		return PanoLayoutType::DEFAULT;
	}

	return sender_view_type;
}

void CW3PANOtab::slotMaximizeOnOff(bool max) 
{
#if 0
  curr_layout_type_ =
	  max ? PanoLayoutType::CROSS_SECTION_MAIN : PanoLayoutType::DEFAULT;
#endif

  if (!max)
  {
	  if (curr_layout_type_ == PanoLayoutType::CROSS_SECTION_MAIN)
	  {
		  if (single_cross_maximized_)
		  {
			  single_cross_maximized_ = false;
			  pano_view_mgr_->set_single_cross_section_maximized(single_cross_maximized_);
			  window_pano_cross_section_->setVisible(true);
		  }
	  }
  }
  else
  {
	  if (GetMaximizedViewType() == PanoLayoutType::CROSS_SECTION_MAIN)
	  {
		  if (single_cross_maximized_)
		  {
			  single_cross_maximized_ = false;
			  pano_view_mgr_->set_single_cross_section_maximized(single_cross_maximized_);
			  window_pano_cross_section_->setVisible(true);
		  }
	  }
  }

  curr_layout_type_ = GetMaximizedViewType();
  
  this->SetVisibleWindows(false);
  SetLayout();
  const auto& event_common_handler =
	  EventHandler::GetInstance()->GetCommonEventHandle();
  event_common_handler.EmitSigSetTabSlotLayout(tab_layout_);
  this->SetVisibleWindows(true);
}

void CW3PANOtab::slotSelectLayout(int row, int column) {
  pano_view_mgr_->SelectLayoutCrossSection(row, column);
}

void CW3PANOtab::slotOrienAdjust() { 
  bool grid_on = orientation_dlg_->IsGridOn();
  slotGridOnOffOrientation(grid_on);
  orientation_dlg_->exec();
}

void CW3PANOtab::slotResetOrientation() {
  bool is_pressed = task_tool_->IsTaskManualOn();
  pano_view_mgr_->ResetOrientationAndResetArch(!is_pressed);
}

void CW3PANOtab::slotGridOnOffOrientation(bool on) {
  pano_view_mgr_->GridOnOffOrientation(on);
}

void CW3PANOtab::slotPanoTask(const PanoTaskID& task_id) {
  switch (task_id) {
	case PanoTaskID::AUTO:
	  pano_view_mgr_->slotAutoArch(true);
	  break;
	case PanoTaskID::MANUAL:
	  emit sigCommonToolCancelSelected();
	  pano_view_mgr_->TaskManualArch();
	  break;
	default:
	  break;
  }
}

void CW3PANOtab::slotTaskResetAutoArchSliceNumber()
{
	if (!pano_view_mgr_)
	{
		return;
	}

	slotResetOrientation();
	pano_view_mgr_->slotAutoArch(false);
}

void CW3PANOtab::slotNerveDrawOn() { emit sigCommonToolCancelSelected(); }

void CW3PANOtab::setMIP(bool isMIP) {
  // m_pPANOViewMgr->setMIP(isMIP);
}

QStringList CW3PANOtab::GetViewList() {
  return QStringList{window_pano_.get()->window_title(),
					 window_pano_arch_.get()->window_title(),
					 window_pano_cross_section_.get()->window_title()};
}

QImage CW3PANOtab::GetScreenshot(int view_type) {
	QWidget* source = GetScreenshotSource(view_type);

  return BaseTab::GetScreenshot(source);
}

QWidget* CW3PANOtab::GetScreenshotSource(int view_type)
{
	QWidget* source = nullptr;

	switch (view_type)
	{
	case 1:
		source = window_pano_.get();
		break;
	case 2:
		source = window_pano_arch_.get();
		break;
	case 3:
		source = window_pano_cross_section_.get();
		break;
	}

	return source;
}

void CW3PANOtab::SyncMeasureResource() {
  pano_view_mgr_->SyncMeasureResource();
}

void CW3PANOtab::ApplyPreferences() {
  BaseTab::ApplyPreferences();

  if (window_pano_) window_pano_->ApplyPreferences();

  if (window_pano_cross_section_)
	window_pano_cross_section_->ApplyPreferences();
}

void CW3PANOtab::slotSyncCrossSectionParams(const float interval, const float degree, const float thickness)
{
	if (!window_pano_cross_section_)
	{
		return;
	}

	blockSignals(true);

	window_pano_cross_section_->GetInterval()->setValue(interval);
	window_pano_cross_section_->GetRotate()->setValue(degree);
	window_pano_cross_section_->GetThickness()->setValue(thickness);

	blockSignals(false);
}

#ifndef WILL3D_VIEWER
void CW3PANOtab::slotRequestedGetPanoCrossSectionViewInfo(int& filter_level, int& thickness)
{
	filter_level = pano_view_mgr_->GetCrossSectionFilterLevel();
	thickness = window_pano_cross_section_->GetThicknessValue();
}

void CW3PANOtab::slotRequestedCreateCrossSectionDCMFiles(bool nerve_visible, bool implant_visible, int filter, int thickness)
{
	pano_view_mgr_->RequestedCreateCrossSectionDCMFiles(nerve_visible, implant_visible, filter, thickness);
}
#endif //!WILL3D_VIEWER

void CW3PANOtab::SetInitArchFromMPR(const bool value)
{
	init_arch_from_mpr_ = value;
	if (!pano_view_mgr_)
	{
		return;
	}
	pano_view_mgr_->set_init_arch_from_mpr(value);
}

void CW3PANOtab::SetCurrentArchType(const ArchTypeID& arch_type)
{
	if (!pano_view_mgr_)
	{
		return;
	}
	pano_view_mgr_->SetCurrentArchType(arch_type);
}

void CW3PANOtab::UpdateArchFromMPR(ArchTypeID arch_type, const std::vector<glm::vec3>& points, const glm::mat4& orientation_matrix, const int slice_number)
{
	if (!pano_view_mgr_)
	{
		return;
	}
	pano_view_mgr_->UpdateArchFromMPR(arch_type, points, orientation_matrix, slice_number);
}

void CW3PANOtab::ClearArch(ArchTypeID arch_type)
{
	if (!pano_view_mgr_)
	{
		return;
	}
	pano_view_mgr_->ClearArch(arch_type);
}

#ifdef WILL3D_EUROPE
void CW3PANOtab::SyncControlButtonOut()
{
	pano_view_mgr_->SetSyncControlButtonOut();
}
#endif // WILL3D_EUROPE

void CW3PANOtab::slotMaximizeSingleCrossSectionView(const bool maximized)
{
	single_cross_maximized_ = maximized;
	switch (curr_layout_type_)
	{
	case CW3PANOtab::PanoLayoutType::DEFAULT:
		if (maximized)
		{
			m_pLayout->setStretch(0, 0);
			m_pLayout->setStretch(1, 1);
		}
		else
		{
			m_pLayout->setStretch(0, top_layout_stretch_);
			m_pLayout->setStretch(1, bottom_layout_stretch_);
		}		
		break;
	case CW3PANOtab::PanoLayoutType::CROSS_SECTION_MAIN:
		if (maximized)
		{
			m_pLayout->setStretch(0, 1);
			m_pLayout->setStretch(1, 0);
		}
		else
		{
			m_pLayout->setStretch(0, top_layout_stretch_);
			m_pLayout->setStretch(1, bottom_layout_stretch_);
		}
		break;
	default:
		return;
	}

	window_pano_->blockSignals(maximized);
	window_pano_->setVisible(!maximized);

	window_pano_arch_->blockSignals(maximized);
	window_pano_arch_->setVisible(!maximized);
}
