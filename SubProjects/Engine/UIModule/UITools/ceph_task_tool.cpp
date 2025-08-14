#include "ceph_task_tool.h"

#include <QApplication>
#include <QToolButton>
#include <QBoxLayout>

#include <Engine/Common/Common/W3Logger.h>
#include <Engine/Common/Common/W3Theme.h>
#include <Engine/Common/Common/W3Define.h>
#include <Engine/Common/Common/language_pack.h>

#include "tool_box.h"
#include "clipping_tool.h"
#include "visibility_tool_box.h"

CephTaskTool::CephTaskTool(QObject *parent)
	: BaseTool(parent) {
	CreateUI();
	Connections();
}

CephTaskTool::~CephTaskTool() {
}

void CephTaskTool::ResetUI() {
	task_[CephTaskID::SURGERY]->setChecked(false);
	clip_tool_->ResetUI();
	visibility_tool_->ResetUI();
}

ClipStatus CephTaskTool::GetClipStatus() {
	ClipStatus status;
	switch (clip_tool_->GetClipPlaneID()) {
	case ClipID::AXIAL:
		status.plane = MPRClipID::AXIAL;
		break;
	case ClipID::CORONAL:
		status.plane = MPRClipID::CORONAL;
		break;
	case ClipID::SAGITTAL:
		status.plane = MPRClipID::SAGITTAL;
		break;
	}
	status.enable = clip_tool_->IsClipEnable();
	status.flip = clip_tool_->IsFlip();
	status.lower = clip_tool_->GetLowerValue();
	status.upper = clip_tool_->GetUpperValue();

	return status;
}

bool CephTaskTool::IsVisible(const VisibleID & id) {
	return visibility_tool_->IsChecked(id);
}

void CephTaskTool::SyncTaskUI(const CephTaskID& task_id, bool state) {
	task_[task_id]->setChecked(state);
}

void CephTaskTool::SyncTaskEventLeave(const CephTaskID & task_id) {
	QApplication::sendEvent(task_[task_id].get(), new QEvent(QEvent::HoverLeave));
}

void CephTaskTool::SyncVisibilityEnable(const VisibleID& visible_id, const bool& enable) 
{
	visibility_tool_->SyncVisibilityEnable(visible_id, enable);

	if (visible_id == VisibleID::FACEPHOTO)
	{
		bool is_visible = true;
		if (visibility_tool_->IsChecked(visible_id))
		{
			is_visible = false;
		}

		visibility_tool_->SyncVisibilityChecked(visible_id, is_visible);
	}	
}

void CephTaskTool::SyncVisibilityResources() {
  visibility_tool_->SyncVisibilityResources();
}

void CephTaskTool::SetClipUpper(int value) {
	clip_tool_->SetUpperValue(value);
}

void CephTaskTool::SetClipLower(int value) {
	clip_tool_->SetLowerValue(value);
}

void CephTaskTool::SetClipParams(const bool& isEnable, const bool& isFlip,
								 const MPRClipID& plane, const int& lower, const int& upper) {
	int clip_id = -1;
	if (plane == MPRClipID::AXIAL)
		clip_id = 0;
	else if (plane == MPRClipID::CORONAL)
		clip_id = 1;
	else if (plane == MPRClipID::SAGITTAL)
		clip_id = 2;

	if (clip_id != -1)
		clip_tool_->SetClipParams(isEnable, isFlip, clip_id,
								  lower, upper);
}

QWidget * CephTaskTool::GetTaskWidget() { return task_tool_box_.get(); }
QWidget * CephTaskTool::GetClipWidget() { return clip_tool_->GetWidget(); }
QWidget * CephTaskTool::GetVisibilityWidget() { return visibility_tool_->GetWidget(); }

void CephTaskTool::CreateUI() {
	CW3Theme* theme = CW3Theme::getInstance();
	QMargins contentsMargin = theme->getToolVBarSizeInfo().marginContents;
	QMargins boxMargins = theme->getToolVBarSizeInfo().marginBox;
	int spacingM = theme->getToolVBarSizeInfo().spacingM;

	// create task tool buttons
	QVBoxLayout* task_layout = new QVBoxLayout();
	task_layout->setContentsMargins(contentsMargin.left() * 0.5, contentsMargin.top(),
									contentsMargin.right() * 0.5, contentsMargin.bottom());
	QString task_stylesheet = theme->toolTaskStyleSheet();

	for (int id = 0; id < CephTaskID::CEPH_TASK_END; ++id) 
	{
		task_[id].reset(new QToolButton());
		task_[id]->setContentsMargins(ui_tools::kMarginZero);
		task_[id]->setStyleSheet(task_stylesheet);
		task_[id]->setStyle(theme->toolTaskStyle());
		task_layout->addWidget(task_[id].get(), 0, Qt::AlignLeft);
	}

#if CHINA_VERSION
	task_[CephTaskID::COORDSYS]->setVisible(false);
#endif //CHINA_VERSION

	task_tool_box_.reset(new ToolBox());
	task_tool_box_->setCaptionName(lang::LanguagePack::txt_task(), Qt::AlignLeft);
	task_tool_box_->addToolLayout(task_layout);
	task_tool_box_->setContentsMargins(boxMargins);

	// create clipping tool ui
	std::vector<QString> mode_names = {
		"Axial", "Coronal", "Sagittal",
	};
	clip_tool_.reset(new ClippingTool(mode_names, ClippingTool::DirectionType::GRID));

	// create visibility tool ui
	visibility_tool_.reset(new VisibilityToolBox(false, false, false,
												 false, true, this));

	// status setting
	task_[CephTaskID::COORDSYS]->setObjectName("CoordSystem");
	task_[CephTaskID::TRACING]->setObjectName("Tracing");
	task_[CephTaskID::SELECT_METHOD]->setObjectName("SelectMethod");
	task_[CephTaskID::SURGERY]->setObjectName("3DSurgery");
	task_[CephTaskID::SHOW_SKIN]->setObjectName("ShowSkin");

	task_[CephTaskID::COORDSYS]->setText(lang::LanguagePack::txt_coordinate_system() + "\n"
										 + lang::LanguagePack::txt_customize_coord_system());
	task_[CephTaskID::TRACING]->setText(lang::LanguagePack::txt_edit_tracing() + "\n"
										+ lang::LanguagePack::txt_edit_a_tracing_point());
	task_[CephTaskID::SELECT_METHOD]->setText(lang::LanguagePack::txt_select_method() + "\n"
											  + lang::LanguagePack::txt_select_analysis_method());
	task_[CephTaskID::SURGERY]->setText(lang::LanguagePack::txt_3d_surgery() + "\n"
										+ lang::LanguagePack::txt_surgery_simulation());
	task_[CephTaskID::SHOW_SKIN]->setText(lang::LanguagePack::txt_show_skin() + "\n"
										  + lang::LanguagePack::txt_visible_face_skin());

	task_[CephTaskID::SURGERY]->setCheckable(true);
}

void CephTaskTool::SetToolTips() {
	
}

void CephTaskTool::Connections() {
	connect(task_[CephTaskID::COORDSYS].get(), SIGNAL(clicked()), this, SLOT(slotTaskCoordSys()));
	connect(task_[CephTaskID::TRACING].get(), SIGNAL(clicked()), this, SLOT(slotTaskTracing()));
	connect(task_[CephTaskID::SELECT_METHOD].get(), SIGNAL(clicked()), this, SLOT(slotTaskSelectMethod()));
	connect(task_[CephTaskID::SURGERY].get(), SIGNAL(clicked()), this, SLOT(slotTaskSurgery()));
	connect(task_[CephTaskID::SHOW_SKIN].get(), SIGNAL(clicked()), this, SLOT(slotTaskShowSkin()));

	connect(clip_tool_.get(), SIGNAL(sigEnable(int)), this, SIGNAL(sigCephClipParamsChanged()));
	connect(clip_tool_.get(), SIGNAL(sigRangeMove(int, int)), this, SIGNAL(sigCephClipParamsChanged()));
	connect(clip_tool_.get(), SIGNAL(sigPlaneChanged(int)), this, SIGNAL(sigCephClipParamsChanged()));
	connect(clip_tool_.get(), SIGNAL(sigRangeSet()), this, SIGNAL(sigCephClipSet()));

	connect(visibility_tool_.get(), SIGNAL(sigVisible(VisibleID, int)), this, SIGNAL(sigCephVisible(VisibleID, int)));
	connect(visibility_tool_.get(), SIGNAL(sigChangeFaceTransparency(int)), this, SIGNAL(sigCephChangeFaceTransparency(int)));
}

void CephTaskTool::slotTaskCoordSys() {
	emit sigCephTask(CephTaskID::COORDSYS);
}

void CephTaskTool::slotTaskTracing() {
	emit sigCephTask(CephTaskID::TRACING);
}

void CephTaskTool::slotTaskSelectMethod() {
	emit sigCephTask(CephTaskID::SELECT_METHOD);
}

void CephTaskTool::slotTaskSurgery() {
	emit sigCephTask(CephTaskID::SURGERY);
}

void CephTaskTool::slotTaskShowSkin() {
	emit sigCephTask(CephTaskID::SHOW_SKIN);
}
