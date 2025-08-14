#include "face_task_tool.h"

#include <QToolButton>
#include <QBoxLayout>

#include <Engine/Common/Common/W3Logger.h>
#include <Engine/Common/Common/W3Theme.h>
#include <Engine/Common/Common/language_pack.h>

#include "tool_box.h"
#include "visibility_tool_box.h"

FaceTaskTool::FaceTaskTool(QObject *parent)
	: BaseTool(parent) {
	CreateUI();
	ResetUI();
	Connections();
}

FaceTaskTool::~FaceTaskTool() {
}

void FaceTaskTool::ResetUI() {
	visibility_tool_->ResetUI();
	task_[FaceTaskID::COMPARE]->setChecked(false);
}

bool FaceTaskTool::IsVisibleFace() const {
	return visibility_tool_->IsChecked(VisibleID::FACEPHOTO);
}

void FaceTaskTool::SyncTaskUI(const FaceTaskID& task_id, bool state) {
	task_[task_id]->setChecked(state);
}

void FaceTaskTool::SyncVisibilityEnable(const bool & enable) {
	visibility_tool_->SyncVisibilityEnable(VisibleID::FACEPHOTO, enable);
}

void FaceTaskTool::EnableFaceUI() {
	visibility_tool_->EnableFaceUI(true);
}

QWidget* FaceTaskTool::GetTaskWidget() { return task_tool_box_.get(); }
QWidget* FaceTaskTool::GetVisibilityWidget() { return visibility_tool_->GetWidget(); }

void FaceTaskTool::CreateUI() {
	CW3Theme* theme = CW3Theme::getInstance();
	QMargins contentsMargin = theme->getToolVBarSizeInfo().marginContents;
	QMargins boxMargins = theme->getToolVBarSizeInfo().marginBox;
	int spacingM = theme->getToolVBarSizeInfo().spacingM;

	// create task tool buttons
	QVBoxLayout* task_layout = new QVBoxLayout();
	task_layout->setContentsMargins(contentsMargin.left() * 0.5, contentsMargin.top(),
									contentsMargin.right() * 0.5, contentsMargin.bottom());
	QString task_stylesheet = theme->toolTaskStyleSheet();
	for (int id = 0; id < FaceTaskID::FACE_TASK_END; ++id) {
		task_[id].reset(new QToolButton());
		task_[id]->setContentsMargins(ui_tools::kMarginZero);
		task_[id]->setStyleSheet(task_stylesheet);
		task_[id]->setStyle(theme->toolTaskStyle());
		task_layout->addWidget(task_[id].get(), 0, Qt::AlignLeft);
	}
	task_tool_box_.reset(new ToolBox());
	task_tool_box_->setCaptionName(lang::LanguagePack::txt_task(), Qt::AlignLeft);
	task_tool_box_->addToolLayout(task_layout);
	task_tool_box_->setContentsMargins(boxMargins);

	// create visibility tool ui
	visibility_tool_.reset(new VisibilityToolBox(false, false, false,
												 false, true, this));

	// status setting
	task_[FaceTaskID::GENERATE]->setObjectName("GenerateFace");
	task_[FaceTaskID::LOAD]->setObjectName("LoadPhoto");
	task_[FaceTaskID::MAPPING]->setObjectName("PhotoMapping");
	task_[FaceTaskID::CLEAR]->setObjectName("ClearMappingPoint");
	task_[FaceTaskID::COMPARE]->setObjectName("BeforeAfter");

	task_[FaceTaskID::GENERATE]->setText(lang::LanguagePack::txt_generate_face());
	task_[FaceTaskID::LOAD]->setText(lang::LanguagePack::txt_load_photo());
	task_[FaceTaskID::MAPPING]->setText(lang::LanguagePack::txt_photo_mapping());
	task_[FaceTaskID::CLEAR]->setText(lang::LanguagePack::txt_clear_points());
	task_[FaceTaskID::COMPARE]->setText(lang::LanguagePack::txt_before_and_after());

	task_[FaceTaskID::COMPARE]->setCheckable(true);
}

void FaceTaskTool::SetToolTips() {}

void FaceTaskTool::Connections() {
	connect(task_[FaceTaskID::COMPARE].get(), SIGNAL(clicked()), this, SLOT(slotTaskCompare()));
	connect(task_[FaceTaskID::LOAD].get(), SIGNAL(clicked()), this, SLOT(slotTaskLoad()));
	connect(task_[FaceTaskID::GENERATE].get(), SIGNAL(clicked()), this, SLOT(slotTaskGenerate()));
	connect(task_[FaceTaskID::CLEAR].get(), SIGNAL(clicked()), this, SLOT(slotTaskClear()));
	connect(task_[FaceTaskID::MAPPING].get(), SIGNAL(clicked()), this, SLOT(slotTaskMapping()));

	connect(visibility_tool_.get(), SIGNAL(sigVisible(VisibleID,int)), this, SIGNAL(sigFaceVisible(VisibleID,int)));	
	connect(visibility_tool_.get(), SIGNAL(sigChangeFaceTransparency(int)), this, SLOT(slotTransparencyChanged(int)));
}

void FaceTaskTool::slotTaskCompare() {
	emit sigFaceTask(FaceTaskID::COMPARE);
}

void FaceTaskTool::slotTaskLoad() {
	emit sigFaceTask(FaceTaskID::LOAD);
}

void FaceTaskTool::slotTaskGenerate() {
	emit sigFaceTask(FaceTaskID::GENERATE);
}

void FaceTaskTool::slotTaskClear() {
	emit sigFaceTask(FaceTaskID::CLEAR);
}

void FaceTaskTool::slotTaskMapping() {
	emit sigFaceTask(FaceTaskID::MAPPING);
}

void FaceTaskTool::slotTransparencyChanged(int value) {
	float alpha = pow(float(value) / 100.0f, 0.5f);

	if (alpha > 1.0f)
		alpha = 1.0f;

	emit sigFaceChangeFaceTransparency(alpha);
}
