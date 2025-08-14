#include "si_task_tool.h"

#include <QBoxLayout>
#include <QRadioButton>
#include <QToolButton>

#include <Engine/Common/Common/W3Logger.h>
#include <Engine/Common/Common/W3Theme.h>
#include <Engine/Common/Common/language_pack.h>

#include "tool_box.h"

SITaskTool::SITaskTool(QObject *parent)
	: BaseTool(parent) {
	CreateUI();
	ResetUI();
	Connections();
}

SITaskTool::~SITaskTool() {
}

void SITaskTool::ResetUI() {
	visible_[SIVisibleID::MAIN]->blockSignals(true);
	visible_[SIVisibleID::MAIN]->setChecked(true);
	visible_[SIVisibleID::MAIN]->blockSignals(false);

	visible_[SIVisibleID::MAIN]->setEnabled(false);
	visible_[SIVisibleID::SECOND]->setEnabled(false);
	visible_[SIVisibleID::BOTH]->setEnabled(false);
}

void SITaskTool::SetEnableSecondVolume(bool enable) {
	visible_[SIVisibleID::MAIN]->setEnabled(enable);
	visible_[SIVisibleID::SECOND]->setEnabled(enable);
	visible_[SIVisibleID::BOTH]->setEnabled(enable);

	if (enable)
	{
		visible_[SIVisibleID::BOTH]->setChecked(enable);
#if 1
		if (visible_[SIVisibleID::BOTH]->isChecked())
		{
			slotVisibleBoth(true);
		}
#endif
	}
}

QWidget * SITaskTool::GetTaskWidget() { return task_tool_box_.get(); }
QWidget * SITaskTool::GetVisibilityWidget() { return visible_tool_box_.get(); }

void SITaskTool::CreateUI() {
	CW3Theme* theme = CW3Theme::getInstance();
	QMargins contentsMargin = theme->getToolVBarSizeInfo().marginContents;
	QMargins boxMargins = theme->getToolVBarSizeInfo().marginBox;
	int spacingM = theme->getToolVBarSizeInfo().spacingM;

	// create task tool buttons
	QVBoxLayout* task_layout = new QVBoxLayout();
	task_layout->setContentsMargins(contentsMargin.left() * 0.5, contentsMargin.top(),
									contentsMargin.right() * 0.5, contentsMargin.bottom());
	QString task_stylesheet = theme->toolTaskStyleSheet();
	for (int id = 0; id < SITaskID::SI_TASK_END; ++id) {
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

	// create visible tool buttons
	QVBoxLayout* visible_layout = new QVBoxLayout();
	visible_layout->setSpacing(spacingM);
	visible_layout->setContentsMargins(contentsMargin);
	for (int id = 0; id < SIVisibleID::SI_VISIBLE_END; ++id) {
		visible_[id].reset(new QRadioButton());
		visible_layout->addWidget(visible_[id].get());
	}
	visible_[0]->setChecked(true);
	visible_tool_box_.reset(new ToolBox());
	visible_tool_box_->setCaptionName(lang::LanguagePack::txt_task(), Qt::AlignLeft);
	visible_tool_box_->addToolLayout(visible_layout);
	visible_tool_box_->setContentsMargins(boxMargins);

	// status setting
	task_[SITaskID::LOAD_NEW]->setObjectName("LoadNewVolume");
	task_[SITaskID::AUTO_REG]->setObjectName("AutoRegistration");

	task_[SITaskID::LOAD_NEW]->setText(lang::LanguagePack::txt_load_new_volume());
	task_[SITaskID::AUTO_REG]->setText(lang::LanguagePack::txt_auto_registration());

	visible_[SIVisibleID::MAIN]->setText(lang::LanguagePack::txt_main_volume());
	visible_[SIVisibleID::SECOND]->setText(lang::LanguagePack::txt_second_volume());
	visible_[SIVisibleID::BOTH]->setText(lang::LanguagePack::txt_both_volume());
}

void SITaskTool::SetToolTips() {}

void SITaskTool::Connections() {
	connect(task_[SITaskID::LOAD_NEW].get(), SIGNAL(clicked()), this, SLOT(slotTaskLoadNew()));
	connect(task_[SITaskID::AUTO_REG].get(), SIGNAL(clicked()), this, SLOT(slotTaskAutoReg()));

	connect(visible_[SIVisibleID::MAIN].get(), SIGNAL(toggled(bool)), this, SLOT(slotVisibleMain(bool)));
	connect(visible_[SIVisibleID::SECOND].get(), SIGNAL(toggled(bool)), this, SLOT(slotVisibleSecond(bool)));
	connect(visible_[SIVisibleID::BOTH].get(), SIGNAL(toggled(bool)), this, SLOT(slotVisibleBoth(bool)));
}

void SITaskTool::slotTaskLoadNew() {
	emit sigSITask(SITaskID::LOAD_NEW);
}

void SITaskTool::slotTaskAutoReg() {
	emit sigSITask(SITaskID::AUTO_REG);
}

void SITaskTool::slotVisibleMain(bool on) {
	emit sigSIVisible(SIVisibleID::MAIN, on);
}

void SITaskTool::slotVisibleSecond(bool on) {
	emit sigSIVisible(SIVisibleID::SECOND, on);
}

void SITaskTool::slotVisibleBoth(bool on) {
	emit sigSIVisible(SIVisibleID::BOTH, on);
}
