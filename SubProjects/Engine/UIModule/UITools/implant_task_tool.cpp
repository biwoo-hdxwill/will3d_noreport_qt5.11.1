#include "implant_task_tool.h"
#include <QBoxLayout>
#include <QCheckBox>
#include <QSpinBox>
#include <QRadioButton>
#include <QLabel>
#include <QDialog>

#include <Engine/Common/Common/W3Theme.h>
#include <Engine/Common/Common/language_pack.h>
#include <Engine/UIModule/UIPrimitive/text_edit.h>

#include "tool_box.h"

ImplantTaskTool::ImplantTaskTool(QObject *parent)
	: BasePanoTaskTool(parent) {
	CreateUI();
	SetToolTips();
}

ImplantTaskTool::~ImplantTaskTool() {
}

void ImplantTaskTool::InitExternUIs(QWidget* bd_ui, QDoubleSpinBox* sagittal_ui,
									const CrossSectionUI& cs_ui, const PanoUI& pano_ui) {
	CW3Theme* theme = CW3Theme::getInstance();
	QMargins contentsMargin = theme->getToolVBarSizeInfo().marginContents;
	QMargins boxMargins = theme->getToolVBarSizeInfo().marginBox;

	bd_tool_box_.reset(new ToolBox());
	bd_tool_box_->setCaptionName(lang::LanguagePack::txt_bone_density(), Qt::AlignLeft);
	bd_tool_box_->setContentsMargins(boxMargins);
	bd_tool_box_->setMaximumSize(QSize(500, 500));
	bd_tool_box_->addToolWidget(bd_ui);

	QSizePolicy size_policy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
	bd_tool_box_->setSizePolicy(size_policy);
	bd_tool_box_->setMinimumSize(150, 150);

	sagittal_rotate_ = sagittal_ui;
	BasePanoTaskTool::InitExternUIs(cs_ui, pano_ui);
}

void ImplantTaskTool::ResetUI() {
	BasePanoTaskTool::ResetUI();
}

void ImplantTaskTool::Connections() {
	BasePanoTaskTool::Connections();
	connect(sagittal_rotate_, SIGNAL(valueChanged(double)), this, SIGNAL(sigImplantSagittalRotate(double)));
	connect(implant_memo_.get(), SIGNAL(textChanged()), this, SLOT(slotImplantMemoChanged()));
}

double ImplantTaskTool::RotateValue() {
	return sagittal_rotate_->value();
}

void ImplantTaskTool::UpdateRotateAngle(double value) {
	double angle = sagittal_rotate_->value();
	sagittal_rotate_->setValue(angle + value);
}

void ImplantTaskTool::SetMemoText(const QString & txt) { 
	implant_memo_->setPlainText(txt);
}

QWidget * ImplantTaskTool::GetMemoWidget() { return memo_tool_box_.get(); }
QWidget * ImplantTaskTool::GetBoneDensityWidget() { 
	return is_bd_popup_ ? nullptr : bd_tool_box_.get(); 
}

void ImplantTaskTool::CreateUI() {
	CW3Theme* theme = CW3Theme::getInstance();
	QMargins boxMargins = theme->getToolVBarSizeInfo().marginBox;

	// create implant memo
	implant_memo_.reset(new TextEdit());
	memo_tool_box_.reset(new ToolBox());
	memo_tool_box_->setCaptionName(lang::LanguagePack::txt_memo(), Qt::AlignLeft);
	memo_tool_box_->addToolWidget(implant_memo_.get());
	memo_tool_box_->setContentsMargins(boxMargins);
}

void ImplantTaskTool::SetToolTips() {}

void ImplantTaskTool::slotBoneDensityPopup(bool popup) {
	if (popup) {
		is_bd_popup_ = popup;
		QHBoxLayout* layout = new QHBoxLayout();
		layout->addWidget(bd_tool_box_.get());
		layout->setAlignment(Qt::AlignCenter);

		bone_density_dialog_.reset(new QDialog(nullptr, Qt::WindowStaysOnTopHint));
		bone_density_dialog_->setWindowFlags(bone_density_dialog_->windowFlags() & ~Qt::WindowContextHelpButtonHint);
		bone_density_dialog_->setModal(false);
		bone_density_dialog_->setLayout(layout);
		bone_density_dialog_->setFixedSize(500, 500);

		connect(bone_density_dialog_.get(), SIGNAL(finished(int)), this, SLOT(slotBDPopupClosed()));
		bone_density_dialog_->show();
	} else {
		slotBDPopupClosed();
	}
}

void ImplantTaskTool::slotBDPopupClosed() {
	if (!is_bd_popup_)
	{
		return;
	}

	if (bone_density_dialog_)
		bone_density_dialog_->hide();
	is_bd_popup_ = false;
	emit sigImplantBDSyncPopupStatus(is_bd_popup_);
}

void ImplantTaskTool::slotImplantMemoChanged() {
	QString text = implant_memo_->toPlainText();
	emit sigImplantMemoChanged(text);
}
