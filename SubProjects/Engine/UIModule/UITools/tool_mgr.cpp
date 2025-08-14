#include "tool_mgr.h"

#include <QLabel>

#include <Engine/Common/Common/W3Logger.h>
#include <Engine/Common/Common/W3LayoutFunctions.h>
#include "common_task_tool.h"
#include "otf_tool.h"
#include "mpr_task_tool.h"
#include "pano_task_tool.h"
#include "implant_task_tool.h"
#include "tmj_task_tool.h"
#ifndef WILL3D_LIGHT
#include "ceph_task_tool.h"
#include "face_task_tool.h"
#include "si_task_tool.h"
#include "endo_task_tool.h"
#endif

std::unique_ptr<ToolMgr> ToolMgr::instance_ = nullptr;
QLabel* ToolMgr::empty_space_ = nullptr;
std::once_flag ToolMgr::onceFlag_;

void ToolMgr::SetInstance() {
	std::call_once(
		ToolMgr::onceFlag_,
		[=]() {
		instance_.reset(new ToolMgr);
	});
}

ToolMgr::ToolMgr() {
	empty_space_ = new QLabel;
	empty_space_->setMinimumSize(5, 5);
	empty_space_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	empty_space_->setObjectName("EmptySpace");
}

ToolMgr * ToolMgr::instance() {
	if (instance_.get() == nullptr) {
		common::Logger::instance()->Print(
			common::LogType::ERR,
			"ToolMgr::instance: ToolMgr Instance does not exist.");
		atexit(0);
	}

	return instance_.get();
}

void ToolMgr::SetCommonTaskTool(const std::weak_ptr<CommonTaskTool>& tool) {
	common_task_tool_ = tool;
}

void ToolMgr::SetMPRTaskTool(const std::weak_ptr<MPRTaskTool>& tool) {
	mpr_task_tool_ = tool;
}

void ToolMgr::SetPanoTaskTool(const std::weak_ptr<PanoTaskTool>& tool) {
	pano_task_tool_ = tool;
}

void ToolMgr::SetImplantTaskTool(const std::weak_ptr<ImplantTaskTool>& tool) {
	implant_task_tool_ = tool;
}

void ToolMgr::SetTMJTaskTool(const std::weak_ptr<TMJTaskTool>& tool)
{
	tmj_task_tool_ = tool;
}

#ifndef WILL3D_LIGHT
void ToolMgr::SetCephTaskTool(const std::weak_ptr<CephTaskTool>& tool) {
	ceph_task_tool_ = tool;
}

void ToolMgr::SetFaceTaskTool(const std::weak_ptr<FaceTaskTool>& tool) {
	face_task_tool_ = tool;
}

void ToolMgr::SetSITaskTool(const std::weak_ptr<SITaskTool>& tool) {
	si_task_tool_ = tool;
}

void ToolMgr::SetEndoTaskTool(const std::weak_ptr<EndoTaskTool>& tool) {
	endo_task_tool_ = tool;
}
#endif

void ToolMgr::SetOTFTaskTool(const std::weak_ptr<OTFTool>& tool) {
	otf_task_tool_ = tool;
}

QLayout* ToolMgr::GetCommonTaskToolLayout() const {
	return common_task_tool_.lock()->GetLayout();
}

bool ToolMgr::GetMPRTools(QVBoxLayout* layout) {
	bool is_tool_list_empty = false;
	ToolMgr::ToolList tool_list;
	if (!mpr_task_tool_.lock()->lightbox_on()) {
		tool_list = {
			mpr_task_tool_.lock()->GetOrientationWidget(),
			mpr_task_tool_.lock()->GetTaskWidget(),
			GetOTFToolBox(),
			mpr_task_tool_.lock()->GetVisibilityWidget(),
			mpr_task_tool_.lock()->GetClipWidget(),
			dynamic_cast<QWidget*>(empty_space_) };
	} else {
		tool_list = ToolMgr::ToolList();
		is_tool_list_empty = true;
	}

	common_task_tool_.lock()->SetImplantAngleVisible(false);

	AddTools(layout, tool_list);

	return is_tool_list_empty;
}

void ToolMgr::GetPanoTools(QVBoxLayout* layout) {
	ToolMgr::ToolList tool_list = {
		pano_task_tool_.lock()->GetOrientationWidget(),
		pano_task_tool_.lock()->GetTaskWidget(),
		pano_task_tool_.lock()->GetNerveToolWidget(),
		GetOTFToolBox(),
		pano_task_tool_.lock()->GetVisibilityWidget(),
		pano_task_tool_.lock()->GetClipWidget(),
		dynamic_cast<QWidget*>(empty_space_) };

	common_task_tool_.lock()->SetImplantAngleVisible(false);

	AddTools(layout, tool_list);
}

void ToolMgr::GetImplantTools(QVBoxLayout* layout) {
	ToolMgr::ToolList tool_list = {
		GetOTFToolBox(),
		implant_task_tool_.lock()->GetVisibilityWidget(),
		implant_task_tool_.lock()->GetClipWidget(),
		implant_task_tool_.lock()->GetBoneDensityWidget(),
		implant_task_tool_.lock()->GetMemoWidget() };
	
	common_task_tool_.lock()->SetImplantAngleVisible(true);

	AddTools(layout, tool_list);
}

void ToolMgr::GetTMJTools(QVBoxLayout* layout)
{
	const TMJTaskTool::Mode& mode = tmj_task_tool_.lock()->mode();
	QVBoxLayout* task_layout = tmj_task_tool_.lock()->GetTMJTaskLayout();
	if (mode == TMJTaskTool::Mode::MODE_2D)
	{
		GetTMJ2DTaskLayout(task_layout);
	}
	else if (mode == TMJTaskTool::Mode::MODE_3D)
	{
		GetTMJ3DTaskLayout(task_layout);
	}

	ToolMgr::ToolList tool_list = {
		tmj_task_tool_.lock()->GetOrientationWidget(),
		tmj_task_tool_.lock()->GetModeSelectionWidget(),
		tmj_task_tool_.lock()->GetTaskWidget(),
		dynamic_cast<QWidget*>(empty_space_) };

	common_task_tool_.lock()->SetImplantAngleVisible(false);

	AddTools(layout, tool_list);
}

#ifndef WILL3D_LIGHT
void ToolMgr::GetCephTools(QVBoxLayout* layout) {
	ToolMgr::ToolList tool_list = {
		ceph_task_tool_.lock()->GetTaskWidget(),
		GetOTFToolBox(),
		ceph_task_tool_.lock()->GetVisibilityWidget(),
		ceph_task_tool_.lock()->GetClipWidget(),
		dynamic_cast<QWidget*>(empty_space_) };

	common_task_tool_.lock()->SetImplantAngleVisible(false);

	AddTools(layout, tool_list);
}

void ToolMgr::GetSITools(QVBoxLayout* layout) {
	ToolMgr::ToolList tool_list = {
		si_task_tool_.lock()->GetTaskWidget(),
		GetOTFToolBox(),
		si_task_tool_.lock()->GetVisibilityWidget(),
		dynamic_cast<QWidget*>(empty_space_) };

	common_task_tool_.lock()->SetImplantAngleVisible(false);

	AddTools(layout, tool_list);
}

void ToolMgr::GetEndoTools(QVBoxLayout* layout) {
	ToolMgr::ToolList tool_list = {
		endo_task_tool_.lock()->GetPlayerWidget(),
		endo_task_tool_.lock()->GetPathWidget(),
		GetOTFToolBox(),
		endo_task_tool_.lock()->GetVisibilityWidget(),
		dynamic_cast<QWidget*>(empty_space_) };

	common_task_tool_.lock()->SetImplantAngleVisible(false);

	AddTools(layout, tool_list);
}

void ToolMgr::GetFaceTools(QVBoxLayout* layout) {
	ToolMgr::ToolList tool_list = {
		face_task_tool_.lock()->GetTaskWidget(),
		GetOTFToolBox(),
		face_task_tool_.lock()->GetVisibilityWidget(),
		dynamic_cast<QWidget*>(empty_space_) };

	common_task_tool_.lock()->SetImplantAngleVisible(false);

	AddTools(layout, tool_list);
}
#endif

void ToolMgr::GetTMJ2DTaskLayout(QVBoxLayout* layout) {
	RemoveTools(layout);

	ToolMgr::ToolList tool_list = {
		tmj_task_tool_.lock()->GetTMJRectWidget(),
		tmj_task_tool_.lock()->GetTMJ2DTaskWidget(),
		tmj_task_tool_.lock()->GetMemoWidget()
	};

	common_task_tool_.lock()->SetImplantAngleVisible(false);

	AddTools(layout, tool_list);
	SetVisibleTools(layout, true);
}

void ToolMgr::GetTMJ3DTaskLayout(QVBoxLayout* layout) {
	RemoveTools(layout);

	ToolMgr::ToolList tool_list = {
		tmj_task_tool_.lock()->GetTMJRectWidget(),
		tmj_task_tool_.lock()->GetTMJ3DTaskWidget(),
		GetOTFToolBox(),
		tmj_task_tool_.lock()->GetMemoWidget()
	};

	common_task_tool_.lock()->SetImplantAngleVisible(false);

	AddTools(layout, tool_list);
	SetVisibleTools(layout, true);
}

QWidget * ToolMgr::GetOTFToolBox() const {
	return otf_task_tool_.lock()->GetWidget();
}

void ToolMgr::AddTools(QVBoxLayout * layout, const ToolList & tool_list) {
	for (auto& tool : tool_list) {
		if (tool) {
			layout->addWidget(tool);
		}
	}
}

void ToolMgr::SetVisibleTools(QVBoxLayout* layout, bool visible) {
	int layout_cnt = layout->count();
	for (int i = 0; i < layout_cnt; i++) {
		auto item = layout->itemAt(i);
		item->widget()->setVisible(visible);
	}
}

void ToolMgr::RemoveTools(QVBoxLayout* layout) {
	SetVisibleTools(layout, false);
	CW3LayoutFunctions::RemoveWidgets(layout);
}
