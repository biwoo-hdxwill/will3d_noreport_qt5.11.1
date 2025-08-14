#include "W3IMPLANTtab.h"

#include <QApplication>
#include <QtConcurrent/QtConcurrent>

#include <Engine/Common/Common/global_preferences.h>
#include <Engine/Common/Common/W3ProgressDialog.h>
#include <Engine/Common/Common/W3Memory.h>
#include <Engine/Common/Common/W3MessageBox.h>
#include <Engine/Common/Common/W3Logger.h>
#include <Engine/Common/Common/W3LayoutFunctions.h>
#include <Engine/Common/Common/event_handler.h>
#include <Engine/Common/Common/event_handle_common.h>

#include <Engine/Resource/Resource/implant_resource.h>
#include <Engine/Resource/ResContainer/resource_container.h>
#include <Engine/Resource/Resource/W3Image3D.h>

#include <Engine/Module/Panorama/pano_engine.h>

#include <Engine/UIModule/UITools/implant_task_tool.h>
#include <Engine/UIModule/UITools/tool_mgr.h>

#include <Engine/UIModule/UIFrame/implant_panel.h>
#include <Engine/UIModule/UIFrame/window_implant_3d.h>
#include <Engine/UIModule/UIFrame/window_implant_sagittal.h>
#include <Engine/UIModule/UIFrame/window_pano_cross_section.h>
#include <Engine/UIModule/UIFrame/window_pano_arch.h>
#include <Engine/UIModule/UIFrame/window_pano.h>

#include "implant_view_mgr.h"

CW3IMPLANTtab::CW3IMPLANTtab(CW3VREngine *VREngine, CW3MPREngine *MPRengine,
							 CW3ResourceContainer *Rcontainer) :
	m_pgVREngine(VREngine),
	m_pgMPRengine(MPRengine), m_pgRcontainer(Rcontainer) {

	task_tool_.reset(new ImplantTaskTool(this));
	ToolMgr::instance()->SetImplantTaskTool(task_tool_);

	window_cross_section_.reset(new WindowPanoCrossSection());
	window_cross_section_->setVisible(false);
}
//20250210 LIN import기능 viewer에 유지
#ifndef WILL3D_VIEWER
void CW3IMPLANTtab::exportProject(ProjectIOImplant & out) {
	if (implant_view_mgr_)
		implant_view_mgr_->exportProject(out);
}
#endif

void CW3IMPLANTtab::importProject(ProjectIOImplant & in, const bool& is_counterpart_exists) {
	if (!initialized()) {
		Initialize();
	}
#ifndef WILL3D_VIEWER
	implant_view_mgr_->importProject(in, is_counterpart_exists);
#endif
	implant_panel_->importProject();
}
//#endif
CW3IMPLANTtab::~CW3IMPLANTtab(void) {
	SAFE_DELETE_OBJECT(implant_panel_);
}

void CW3IMPLANTtab::SetPanoEngine(const std::shared_ptr<PanoEngine>& pano_engine) {
	pano_engine_ = pano_engine;
}

void CW3IMPLANTtab::connections() 
{
	connect(implant_panel_, &ImplantPanel::sigAddImplant, this, &CW3IMPLANTtab::slotAddImplant);
	connect(implant_panel_, &ImplantPanel::sigChangeImplant, this, &CW3IMPLANTtab::slotChangeImplant);
	connect(implant_panel_, &ImplantPanel::sigSelectImplant, this, &CW3IMPLANTtab::slotSelectImplant);
	connect(implant_panel_, &ImplantPanel::sigDeleteImplant, this, &CW3IMPLANTtab::slotDeleteImplant);
	connect(implant_panel_, &ImplantPanel::sigDeleteAllImplants, this, &CW3IMPLANTtab::slotDeleteAllImplants);
	connect(implant_panel_, &ImplantPanel::sigArchSelectionChanged, implant_view_mgr_.get(), &ImplantViewMgr::slotChangedArchTypeFromPanoArch);
	connect(implant_panel_, &ImplantPanel::sigCancelAddImplant, this, &CW3IMPLANTtab::slotCancelAddImplant);

	connect(window_3d_implant_.get(), &WindowImplant3D::sigClip3DOnOff, this, &CW3IMPLANTtab::slotClip3DOnOff);
	connect(window_3d_implant_.get(), &WindowImplant3D::sigMaximizeOnOff, [=](bool on) { ChangeLayout(ImplantLayoutType::VR_MAIN, on); });
	
	connect(window_arch_.get(), &WindowPanoArch::sigMaximizeOnOff, [=](bool on) { ChangeLayout(ImplantLayoutType::ARCH_MAIN, on); });

	connect(window_pano_.get(), &WindowPanoArch::sigMaximizeOnOff, [=](bool on) { ChangeLayout(ImplantLayoutType::PANORAMA_MAIN, on); });

	connect(window_cross_section_.get(), &WindowPanoCrossSection::sigSelectLayout, this, &CW3IMPLANTtab::slotSelectLayout);
	connect(window_cross_section_.get(), &WindowPanoCrossSection::sigMaximizeOnOff, [=](bool on) { ChangeLayout(ImplantLayoutType::CROSS_SECTION_MAIN, on); });

	connect(implant_view_mgr_.get(), &ImplantViewMgr::sigImplantSelectionChanged, this, &CW3IMPLANTtab::slotImplantSelectionChanged);
	connect(implant_view_mgr_.get(), &ImplantViewMgr::sigDeleteImplantFromView, this, &CW3IMPLANTtab::slotDeleteImplantFromView);
	connect(implant_view_mgr_.get(), &ImplantViewMgr::sigImplantPlaced, this, &CW3IMPLANTtab::slotPlacedImplant);
	connect(implant_view_mgr_.get(), &ImplantViewMgr::sigSyncBDViewStatus, this, &CW3IMPLANTtab::sigSyncBDViewStatus);
	connect(implant_view_mgr_.get(), &ImplantViewMgr::sigSyncArchType, this, &CW3IMPLANTtab::slotSyncArchType);

	connect(task_tool_.get(), &ImplantTaskTool::sigImplantMemoChanged, this, &CW3IMPLANTtab::slotImplantMemoChanged);

	connect(implant_view_mgr_.get(), &BasePanoViewMgr::sigSyncCrossSectionParams, this, &CW3IMPLANTtab::sigSyncCrossSectionParams);
	connect(implant_view_mgr_.get(), &ImplantViewMgr::sigMaximizeSingleCrossSectionView, this, &CW3IMPLANTtab::slotMaximizeSingleCrossSectionView);

#ifdef WILL3D_EUROPE
	connect(implant_view_mgr_.get(), &ImplantViewMgr::sigShowButtonListDialog, this, &CW3IMPLANTtab::sigShowButtonListDialog);
#endif // WILL3D_EUROPE
}

void CW3IMPLANTtab::Initialize() {
	if (BaseTab::initialized()) {
		common::Logger::instance()->Print(common::LogType::ERR, "already initialized.");
		assert(false);
	}

	/*window_cross_section_.reset(new WindowPanoCrossSection());
	window_cross_section_->setVisible(false);*/
	window_cross_section_->SetIntervalMinimumValue(ResourceContainer::GetInstance()->GetMainVolume().pixelSpacing());

	window_3d_implant_.reset(new WindowImplant3D());
	window_3d_implant_->setVisible(false);

	window_sagittal_.reset(new WindowImplantSagittal());	
	window_sagittal_->setVisible(false);

	window_arch_.reset(new WindowPanoArch(true));
	window_arch_->setVisible(false);

	window_pano_.reset(new WindowPano());
	window_pano_->setVisible(false);

	implant_panel_ = new ImplantPanel(m_pgVREngine, m_pgMPRengine, m_pgRcontainer);
	implant_panel_->setVisible(false);

	BasePanoTaskTool::CrossSectionUI cs_ui;
	cs_ui.angle = window_cross_section_->GetRotate();
	cs_ui.interval = window_cross_section_->GetInterval();
	cs_ui.thickness = window_cross_section_->GetThickness();

	BasePanoTaskTool::PanoUI pano_ui;
	pano_ui.arch_range = window_arch_->GetRange();
	pano_ui.thickness = window_pano_->GetThickness();

	implant_view_mgr_.reset(new ImplantViewMgr());
	BaseTab::SetCastedViewMgr(std::static_pointer_cast<BaseViewMgr>(implant_view_mgr_));
	implant_view_mgr_->SetPanoEngine(pano_engine_);

	task_tool_->InitExternUIs(implant_view_mgr_->GetViewBoneDensity(),
							  window_sagittal_->GetRotate(), cs_ui, pano_ui);
	// Bone Density view 때문에 순서가 이렇게 되어야 함
	implant_view_mgr_->SetImplantMenu(task_tool_);

	window_pano_->SetView(implant_view_mgr_->GetViewPanoWidget());
	window_arch_->SetView(implant_view_mgr_->GetViewPanoArchWidget());
	window_sagittal_->SetView(implant_view_mgr_->GetViewSagittalWidget());
	window_3d_implant_->SetView(implant_view_mgr_->GetView3DWidget());

	window_cross_section_->SetViews(implant_view_mgr_->GetViewPanoCrossSectionWidget());
	QSize default_layout = GlobalPreferences::GetInstance()->preferences_.advanced.cross_section_view.default_layout;
	window_cross_section_->SetViewLayoutCount(default_layout.height(), default_layout.width());

	SetLayout();

	connections();

	BaseTab::set_initialized(true);
}

void CW3IMPLANTtab::SetLayout() {
	main_layout_.reset(new QVBoxLayout());
	main_layout_->setSpacing(1);

	switch (curr_layout_type_) {
	case CW3IMPLANTtab::ImplantLayoutType::DEFAULT:
		SetDefaultLayout();
		break;
	case CW3IMPLANTtab::ImplantLayoutType::VR_MAIN:
		SetVRMainLayout();
		break;
	case CW3IMPLANTtab::ImplantLayoutType::ARCH_MAIN:
		SetArchMainLayout();
		break;
	case CW3IMPLANTtab::ImplantLayoutType::PANORAMA_MAIN:
		SetPanoramaMainLayout();
		break;
	case CW3IMPLANTtab::ImplantLayoutType::CROSS_SECTION_MAIN:
		SetCrossSectionMainLayout();
		break;
	}

	if (curr_layout_type_ == CW3IMPLANTtab::ImplantLayoutType::DEFAULT)
	{
		top_layout_stretch_ = 3;
		middle_layout_stretch_ = 1;
		bottom_layout_stretch_ = 3;
	}
	else
	{
		top_layout_stretch_ = 1;
		bottom_layout_stretch_ = 6;
	}

	tab_layout_ = main_layout_.get();
}

void CW3IMPLANTtab::SetDefaultLayout() 
{
	QHBoxLayout* top_layout = new QHBoxLayout();
	QHBoxLayout* bottom_layout = new QHBoxLayout();

	top_layout->setSpacing(kLayoutSpacing);
	top_layout->addWidget(window_arch_.get());
	top_layout->setStretch(0, 1);
	top_layout->addWidget(window_sagittal_.get());
	top_layout->setStretch(1, 1);

#ifndef WILL3D_EUROPE
	top_layout->addWidget(window_cross_section_.get());
	top_layout->setStretch(2, 1);
#else
	bool cross_section_view_enable = GlobalPreferences::GetInstance()->preferences_.advanced.implant_view.cross_section_view_enable;
	if (cross_section_view_enable)
	{
		top_layout->addWidget(window_cross_section_.get());
		top_layout->setStretch(2, 1);
	}
#endif // !WILL3D_EUROPE

	bottom_layout->setSpacing(kLayoutSpacing);
	bottom_layout->addWidget(window_pano_.get());

	bottom_layout->setStretch(0, 2);
	bottom_layout->addWidget(window_3d_implant_.get());
	bottom_layout->setStretch(1, 1);

	main_layout_->addLayout(top_layout);
	main_layout_->setStretch(0, 3);
	main_layout_->addWidget(implant_panel_);
	main_layout_->setStretch(1, 1);
	main_layout_->addLayout(bottom_layout);
	main_layout_->setStretch(2, 3);
}

void CW3IMPLANTtab::SetVRMainLayout() 
{
	QHBoxLayout* bottom_layout = new QHBoxLayout();
	QVBoxLayout* mini_3d_layout = new QVBoxLayout();

	mini_3d_layout->setSpacing(kLayoutSpacing);
	mini_3d_layout->addWidget(window_arch_.get());
	mini_3d_layout->setStretch(0, 1);
	mini_3d_layout->addWidget(window_sagittal_.get());
	mini_3d_layout->setStretch(1, 1);

#ifndef WILL3D_EUROPE
	mini_3d_layout->addWidget(window_cross_section_.get());
	mini_3d_layout->setStretch(2, 1);
#else
	bool cross_section_view_enable = GlobalPreferences::GetInstance()->preferences_.advanced.implant_view.cross_section_view_enable;
	if (cross_section_view_enable)
	{
		mini_3d_layout->addWidget(window_cross_section_.get());
		mini_3d_layout->setStretch(2, 1);
	}
#endif // !WILL3D_EUROPE

	bottom_layout->setSpacing(kLayoutSpacing);
	bottom_layout->addWidget(window_3d_implant_.get());
	bottom_layout->setStretch(0, 2);
	bottom_layout->addLayout(mini_3d_layout);
	bottom_layout->setStretch(1, 1);

	main_layout_->addWidget(implant_panel_);
	main_layout_->setStretch(0, 1);
	main_layout_->addLayout(bottom_layout);
	main_layout_->setStretch(1, 6);
}

void CW3IMPLANTtab::SetArchMainLayout() 
{
	QHBoxLayout* bottom_layout = new QHBoxLayout();
	QVBoxLayout* mini_2d_layout = new QVBoxLayout();

	mini_2d_layout->setSpacing(kLayoutSpacing);

#ifndef WILL3D_EUROPE
	mini_2d_layout->addWidget(window_cross_section_.get());
	mini_2d_layout->setStretch(1, 1);
#else
	bool cross_section_view_enable = GlobalPreferences::GetInstance()->preferences_.advanced.implant_view.cross_section_view_enable;
	if (cross_section_view_enable)
	{
		mini_2d_layout->addWidget(window_cross_section_.get());
		mini_2d_layout->setStretch(1, 1);
	}
#endif // !WILL3D_EUROPE
	mini_2d_layout->addWidget(window_sagittal_.get());
	mini_2d_layout->setStretch(2, 1);

	bottom_layout->setSpacing(kLayoutSpacing);
	bottom_layout->addWidget(window_arch_.get());
	bottom_layout->setStretch(0, 2);
	bottom_layout->addLayout(mini_2d_layout);
	bottom_layout->setStretch(1, 1);

	main_layout_->addWidget(implant_panel_);
	main_layout_->setStretch(0, 1);
	main_layout_->addLayout(bottom_layout);
	main_layout_->setStretch(1, 6);
}

void CW3IMPLANTtab::SetPanoramaMainLayout()
{
	main_layout_->addWidget(implant_panel_);
	main_layout_->addWidget(window_pano_.get(), 1);
}

void CW3IMPLANTtab::SetCrossSectionMainLayout() 
{
	QHBoxLayout* bottom_layout = new QHBoxLayout();
	QVBoxLayout* mini_2d_layout = new QVBoxLayout();

	bottom_layout_ = bottom_layout;

	mini_2d_layout->setSpacing(kLayoutSpacing);
	mini_2d_layout->addWidget(window_sagittal_.get());
	mini_2d_layout->setStretch(1, 1);
	mini_2d_layout->addWidget(window_arch_.get());
	mini_2d_layout->setStretch(2, 1);
		
	bottom_layout->setSpacing(kLayoutSpacing);

#ifndef WILL3D_EUROPE
	bottom_layout->addWidget(window_cross_section_.get());
	bottom_layout->setStretch(0, 2);
#else
	bool cross_section_view_enable = GlobalPreferences::GetInstance()->preferences_.advanced.implant_view.cross_section_view_enable;
	if (cross_section_view_enable)
	{
		bottom_layout->addWidget(window_cross_section_.get());
		bottom_layout->setStretch(0, 2);
	}
#endif // !WILL3D_EUROPE

	bottom_layout->addLayout(mini_2d_layout);
	bottom_layout->setStretch(1, 1);
		
	connect(this, &CW3IMPLANTtab::sigCrossSectionMainLayoutMaximize, [&](bool maximize) 
	{
		if (maximize)
		{
			bottom_layout_->setStretch(1, 0);
		}
		else
		{
			bottom_layout_->setStretch(1, 1);
		}
	});

	main_layout_->addWidget(implant_panel_);
	main_layout_->setStretch(0, 1);
	main_layout_->addLayout(bottom_layout);
	main_layout_->setStretch(1, 6);
}

void CW3IMPLANTtab::ChangeLayout(const ImplantLayoutType& layout_type, bool on) 
{
	if (layout_type == ImplantLayoutType::CROSS_SECTION_MAIN)
	{
		if (single_cross_maximized_)
		{
			single_cross_maximized_ = false;
			implant_view_mgr_->set_single_cross_section_maximized(single_cross_maximized_);
			window_cross_section_->setVisible(true);
		}
	}
	
	if (on) 
	{
		curr_layout_type_ = layout_type;
		SyncOtherWindowMaximizeStatus();
	} 
	else 
	{
		curr_layout_type_ = ImplantLayoutType::DEFAULT;
	}

	SetVisibleWindows(false);
	SetLayout();
	const auto& event_common_handler = EventHandler::GetInstance()->GetCommonEventHandle();
	event_common_handler.EmitSigSetTabSlotLayout(tab_layout_);
	SetVisibleWindows(true);
}

void CW3IMPLANTtab::SyncOtherWindowMaximizeStatus() {
	switch (curr_layout_type_) {
	case CW3IMPLANTtab::ImplantLayoutType::DEFAULT:
		break;
	case CW3IMPLANTtab::ImplantLayoutType::VR_MAIN:
		window_cross_section_->SetMaximize(false);
		window_arch_->SetMaximize(false);
		window_pano_->SetMaximize(false);
		break;
	case CW3IMPLANTtab::ImplantLayoutType::ARCH_MAIN:
		window_3d_implant_->SetMaximize(false);
		window_cross_section_->SetMaximize(false);
		window_pano_->SetMaximize(false);
		break;
	case CW3IMPLANTtab::ImplantLayoutType::PANORAMA_MAIN:
		window_3d_implant_->SetMaximize(false);
		window_cross_section_->SetMaximize(false);
		window_arch_->SetMaximize(false);
		break;
	case CW3IMPLANTtab::ImplantLayoutType::CROSS_SECTION_MAIN:		
		window_3d_implant_->SetMaximize(false);
		window_arch_->SetMaximize(false);
		window_pano_->SetMaximize(false);
		break;
	}
}

void CW3IMPLANTtab::slotClip3DOnOff(bool clip_on) {
	implant_view_mgr_->Clip3DOnOff(clip_on);
}

void CW3IMPLANTtab::slotSelectLayout(int row, int column) {
	implant_view_mgr_->SelectLayoutCrossSection(row, column);
}

void CW3IMPLANTtab::slotPlacedImplant() {
	pano_engine_->SetImplantPlaced(); // 여기서 선택된 임플란트 아이디가 결정된다.
	emit sigSyncImplantAngle();
}

void CW3IMPLANTtab::slotAddImplant(const implant_resource::ImplantInfo& implant_params) {
#if 0
	if (!implant_view_mgr_->tmpIsRenderPano2D() ||
		(curr_layout_type_ != ImplantLayoutType::DEFAULT &&
			curr_layout_type_ != ImplantLayoutType::PANORAMA_MAIN)) {
		CW3MessageBox msg_no_implnant_model("Will3D", "Please add an implant in Panorama-MPR mode.");
		msg_no_implnant_model.exec();
		implant_panel_->SetImplantButtonStatusDefault(implant_params.id);
		return;
	}
#endif

	emit sigCommonToolCancelSelected();

	slotCancelAddImplant();

	const auto& res_implant = ResourceContainer::GetInstance()->GetImplantResource();
	int selected_implant_id = res_implant.selected_implant_id();
	if (selected_implant_id > -1)
	{
		implant_view_mgr_->SetImplantSelectedStatusFromTab(selected_implant_id, false);
	}

	int removed_implant_id = -1;
	CW3ProgressDialog* progress = CW3ProgressDialog::getInstance(CW3ProgressDialog::WAITTING);
	QFutureWatcher<void> watcher;
	auto future = QtConcurrent::run(pano_engine_.get(), &PanoEngine::AddImplant,
									implant_params, &removed_implant_id);
	connect(&watcher, SIGNAL(finished()), progress, SLOT(hide()));
	watcher.setFuture(future);
	progress->exec();
	watcher.waitForFinished();

	bool add_success = future.result();
	if (!add_success) {
		CW3MessageBox msg_no_implnant_model("Will3D", "Invalid Implant Model");
		msg_no_implnant_model.exec();
		implant_panel_->SetImplantButtonStatusDefault(implant_params.id);
		return;
	} else {
		QApplication::processEvents();
		// 3D volume 에서 GL widget 업데이트 하려고
		EventHandler::GetInstance()->GetCommonEventHandle().EmitSigUpdateImplant();
	}

	if (removed_implant_id > 0) {
		implant_panel_->SetImplantButtonStatusDefault(removed_implant_id);
		implant_view_mgr_->UpdateSceneAllViews();
	}
	task_tool_->SyncVisibilityResources();
}

void CW3IMPLANTtab::slotChangeImplant(const implant_resource::ImplantInfo & implant_params) {
	CW3ProgressDialog* progress = CW3ProgressDialog::getInstance(CW3ProgressDialog::WAITTING);
	QFutureWatcher<void> watcher;
	auto future = QtConcurrent::run(pano_engine_.get(), &PanoEngine::ChangeImplant,
									implant_params);
	connect(&watcher, SIGNAL(finished()), progress, SLOT(hide()));
	watcher.setFuture(future);
	progress->exec();
	watcher.waitForFinished();

	bool change_success = future.result();
	if (!change_success) {
		CW3MessageBox msg_no_implnant_model("Will3D", "Invalid Implant Model");
		msg_no_implnant_model.exec();
		implant_panel_->SetImplantSpecAsPrevModel(implant_params.id);
		return;
	} else {
		QApplication::processEvents();
		// 3D volume 에서 GL widget 업데이트 하려고
		EventHandler::GetInstance()->GetCommonEventHandle().EmitSigUpdateImplant();
	}

	implant_view_mgr_->ChangeImplantModelFromTab(implant_params.id);
}

void CW3IMPLANTtab::slotSelectImplant(int implant_id, bool selected) {
	// selected == true 이면 선택된 임플란트 아이디가 결정된다.
	//pano_engine_->SetImplantSelected(implant_id, selected);
	implant_view_mgr_->SetImplantSelectedStatusFromTab(implant_id, selected);
}

void CW3IMPLANTtab::slotDeleteImplant(int implant_id) {
	pano_engine_->DeleteImplant(implant_id);
	implant_view_mgr_->DeleteImplantFromTab(implant_id);
	task_tool_->SyncVisibilityResources();

	emit sigSyncImplantAngle();
}

void CW3IMPLANTtab::slotDeleteAllImplants() {
	pano_engine_->DeleteAllImplants();
	implant_view_mgr_->DeleteAllImplantsFromTab();
	task_tool_->SyncVisibilityResources();

	emit sigSyncImplantAngle();
}

void CW3IMPLANTtab::slotImplantSelectionChanged(int implant_id) {
	implant_panel_->SetImplantButtonStatusSelected(implant_id);
}

void CW3IMPLANTtab::slotCancelAddImplant()
{
	const auto& res_implant = ResourceContainer::GetInstance()->GetImplantResource();
	int add_implant_id = res_implant.add_implant_id();
	if (add_implant_id < 1)
	{
		return;
	}

	slotDeleteImplantFromView(add_implant_id);
}

void CW3IMPLANTtab::slotMaximizeSingleCrossSectionView(const bool maximized)
{
	single_cross_maximized_ = maximized;

	if (curr_layout_type_ == CW3IMPLANTtab::ImplantLayoutType::DEFAULT)
	{
		if (maximized)
		{
			main_layout_->setStretch(0, 1);
			main_layout_->setStretch(1, 0);
			main_layout_->setStretch(2, 0);
		}
		else
		{
			main_layout_->setStretch(0, top_layout_stretch_);
			main_layout_->setStretch(1, middle_layout_stretch_);
			main_layout_->setStretch(2, bottom_layout_stretch_);
		}	
	}
	else
	{
		if (maximized)
		{
			main_layout_->setStretch(0, 1);
			main_layout_->setStretch(1, 0);						
		}
		else
		{
			main_layout_->setStretch(0, top_layout_stretch_);
			main_layout_->setStretch(1, bottom_layout_stretch_);
		}
	}

	implant_panel_->blockSignals(maximized);
	implant_panel_->setVisible(!maximized);

	window_arch_->blockSignals(maximized);
	window_arch_->setVisible(!maximized);

	window_sagittal_->blockSignals(maximized);
	window_sagittal_->setVisible(!maximized);

	if (curr_layout_type_ == CW3IMPLANTtab::ImplantLayoutType::DEFAULT)
	{
		window_pano_->blockSignals(maximized);
		window_pano_->setVisible(!maximized);

		window_3d_implant_->blockSignals(maximized);
		window_3d_implant_->setVisible(!maximized);
	}
	else if (curr_layout_type_ == CW3IMPLANTtab::ImplantLayoutType::VR_MAIN)
	{
		window_3d_implant_->blockSignals(maximized);
		window_3d_implant_->setVisible(!maximized);
	}
	else if (curr_layout_type_ == CW3IMPLANTtab::ImplantLayoutType::CROSS_SECTION_MAIN)
	{
		emit sigCrossSectionMainLayoutMaximize(maximized);
	}
}

void CW3IMPLANTtab::slotDeleteImplantFromView(int implant_id) {
	slotDeleteImplant(implant_id);
	implant_panel_->SetImplantButtonStatusDefault(implant_id);

	emit sigSyncImplantAngle();
}

void CW3IMPLANTtab::slotImplantMemoChanged(const QString & memo) {
  pano_engine_->SetImplantMemo(memo);
}

void CW3IMPLANTtab::slotSyncArchType() {
	implant_panel_->SyncArchType(pano_engine_->curr_arch_type());
}

void CW3IMPLANTtab::SetVisibleWindows(bool isVisible) {
	if (!initialized())
		return;

	if (isVisible) 
	{
		if (tab_changed_)
		{
			implant_view_mgr_->SyncMeasureResource();
		}

#if 0
		CW3LayoutFunctions::setVisibleWidgets(tab_layout_, isVisible);
#else
		if (single_cross_maximized_)
		{
			window_cross_section_->setVisible(true);
			implant_view_mgr_->CheckCrossSectionMaximizeAlone();
		}
		else
		{
			CW3LayoutFunctions::setVisibleWidgets(tab_layout_, isVisible);
		}
#endif
		implant_view_mgr_->SyncPanoResource();
		implant_panel_->SyncArchType(pano_engine_->curr_arch_type());
		task_tool_->SyncVisibilityResources();

	} 
	else 
	{
		implant_panel_->setVisible(isVisible);
		window_cross_section_->setVisible(isVisible);
		window_arch_->setVisible(isVisible);
		window_pano_->setVisible(isVisible);
		window_3d_implant_->setVisible(isVisible);
		window_sagittal_->setVisible(isVisible);
		implant_view_mgr_->SetVisibleViews(isVisible);
	}
}

void CW3IMPLANTtab::SetApplicationUIMode(const bool & is_maximize) {
  implant_panel_->SetApplicationUIMode(is_maximize);
}

QStringList CW3IMPLANTtab::GetViewList() {
	switch (curr_layout_type_) {
	case CW3IMPLANTtab::ImplantLayoutType::DEFAULT:
		return QStringList{
			window_arch_.get()->window_title(),
			window_sagittal_.get()->window_title(),
			window_cross_section_.get()->window_title(),
			window_pano_.get()->window_title(),
			window_3d_implant_.get()->window_title()
		};
		break;
	case CW3IMPLANTtab::ImplantLayoutType::VR_MAIN:
		return QStringList{
			window_3d_implant_.get()->window_title(),
			window_arch_.get()->window_title(),
			window_sagittal_.get()->window_title(),
			window_cross_section_.get()->window_title()
		};
		break;
	case CW3IMPLANTtab::ImplantLayoutType::ARCH_MAIN:
		return QStringList{
			window_arch_.get()->window_title(),
			window_cross_section_.get()->window_title(),
			window_sagittal_.get()->window_title()
		};
		break;
	case CW3IMPLANTtab::ImplantLayoutType::CROSS_SECTION_MAIN:
		return QStringList{
			window_cross_section_.get()->window_title(),
			window_sagittal_.get()->window_title(),
			window_arch_.get()->window_title()
		};
		break;
	default:
		return QStringList();
		break;
	}
}

QImage CW3IMPLANTtab::GetScreenshot(int view_type) {
	QWidget* source = GetScreenshotSource(view_type);

	return BaseTab::GetScreenshot(source);
}

QWidget* CW3IMPLANTtab::GetScreenshotSource(int view_type)
{
	QWidget* source = nullptr;
	switch (curr_layout_type_)
	{
	case CW3IMPLANTtab::ImplantLayoutType::DEFAULT:
		switch (view_type)
		{
		case 1: source = window_arch_.get(); break;
		case 2: source = window_sagittal_.get(); break;
		case 3: source = window_cross_section_.get(); break;
		case 4: source = window_pano_.get(); break;
		case 5: source = window_3d_implant_.get(); break;
		}
		break;
	case CW3IMPLANTtab::ImplantLayoutType::VR_MAIN:
		switch (view_type)
		{
		case 1: source = window_3d_implant_.get(); break;
		case 2: source = window_arch_.get(); break;
		case 3: source = window_sagittal_.get(); break;
		case 4: source = window_cross_section_.get(); break;
		}
		break;
	case CW3IMPLANTtab::ImplantLayoutType::ARCH_MAIN:
		switch (view_type)
		{
		case 1: source = window_arch_.get(); break;
		case 2: source = window_cross_section_.get(); break;
		case 3: source = window_sagittal_.get(); break;
		}
		break;
	case CW3IMPLANTtab::ImplantLayoutType::CROSS_SECTION_MAIN:
		switch (view_type)
		{
		case 1: source = window_cross_section_.get(); break;
		case 2: source = window_sagittal_.get(); break;
		case 3: source = window_arch_.get(); break;
		}
		break;
	}

	return source;
}

void CW3IMPLANTtab::ApplyPreferences() 
{
	if (implant_view_mgr_)
		implant_view_mgr_->ApplyPreferences();

	if (window_pano_)
		window_pano_->ApplyPreferences();

	if (window_cross_section_)
		window_cross_section_->ApplyPreferences();
}

#ifdef WILL3D_EUROPE
void CW3IMPLANTtab::SyncControlButtonOut()
{
	implant_view_mgr_->SetSyncControlButtonOut();
}
#endif // WILL3D_EUROPE

void CW3IMPLANTtab::SyncMeasureResource() 
{
	implant_view_mgr_->SyncMeasureResource();
}

void CW3IMPLANTtab::slotSyncCrossSectionParams(const float interval, const float degree, const float thickness)
{
	if (!window_cross_section_)
	{
		return;
	}

	blockSignals(true);

	window_cross_section_->GetInterval()->setValue(interval);
	window_cross_section_->GetRotate()->setValue(degree);
	window_cross_section_->GetThickness()->setValue(thickness);

	blockSignals(false);
}
