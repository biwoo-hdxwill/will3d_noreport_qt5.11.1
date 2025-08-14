#include "orientation_tool.h"
#include <QSpinBox>
#include <QToolButton>
#include <QBoxLayout>

#include <Engine/Common/Common/language_pack.h>
#include <Engine/Common/Common/W3Theme.h>

#include "tool_box.h"

OrientationTool::OrientationTool(QObject *parent)
	: BaseTool(parent) {
	orient_[ReorientViewID::ORIENT_A] = nullptr;
	orient_[ReorientViewID::ORIENT_I] = nullptr;
	orient_[ReorientViewID::ORIENT_R] = nullptr;

	CreateUI();
	ResetUI();
	SetToolTips();
}

OrientationTool::~OrientationTool() {}

void OrientationTool::ResetUI() {
}

void OrientationTool::InitExternUIs(const OrientationTool::OrientUI & orient_ui) {
	orient_[ReorientViewID::ORIENT_A] = orient_ui.a;
	orient_[ReorientViewID::ORIENT_I] = orient_ui.i;
	orient_[ReorientViewID::ORIENT_R] = orient_ui.r;
}

void OrientationTool::SetOrientDegrees(const int & degree_a, const int & degree_r,
									   const int & degree_i) {
	orient_[ReorientViewID::ORIENT_A]->setValue(degree_a);
	orient_[ReorientViewID::ORIENT_I]->setValue(degree_i);
	orient_[ReorientViewID::ORIENT_R]->setValue(degree_r);
}

int OrientationTool::GetOrientDegree(const ReorientViewID & view_type) const {
	return orient_[view_type]->value();
}

void OrientationTool::ResetOrientDegreesUI() {
	for (int id = 0; id < ReorientViewID::REORI_VIEW_END; ++id) {
		orient_[id]->blockSignals(true);
		orient_[id]->setValue(0);
		orient_[id]->blockSignals(false);
	}
}

void OrientationTool::SyncOrientDegreeUIOnly(const ReorientViewID& view_type, const int& degree_view) {
	int curr_degree = orient_[view_type]->value();

	if (degree_view != curr_degree) {
		orient_[view_type]->blockSignals(true);
		orient_[view_type]->setValue(degree_view);
		orient_[view_type]->blockSignals(false);
	}
}

QWidget * OrientationTool::GetWidget() {
	return tool_box_.get();
}

void OrientationTool::CreateUI() {
	CW3Theme* theme = CW3Theme::getInstance();
	QMargins contentsMargin = theme->getToolVBarSizeInfo().marginContents;
	QMargins boxMargins = theme->getToolVBarSizeInfo().marginBox;
	int spacingM = theme->getToolVBarSizeInfo().spacingM;

	// create reorientation tool buttons
	QHBoxLayout* orient_layout = new QHBoxLayout();
	orient_layout->setContentsMargins(contentsMargin);
	orient_layout->setSpacing(spacingM);

	QString orient_stylesheet = theme->toolOrientationStyleSheet();
	for (int id = 0; id < ReorientID::REORIENT_END; ++id) {
		reorient_[id].reset(new QToolButton());
		reorient_[id]->setContentsMargins(ui_tools::kMarginZero);
		reorient_[id]->setStyleSheet(orient_stylesheet);
		orient_layout->addWidget(reorient_[id].get());
		orient_layout->setStretch(id, 1);
	}

	tool_box_.reset(new ToolBox());
	tool_box_->setCaptionName(lang::LanguagePack::txt_orientation(), Qt::AlignLeft);
	tool_box_->addToolLayout(orient_layout);
	tool_box_->setContentsMargins(boxMargins);

	reorient_[ReorientID::REORIENT]->setText(lang::LanguagePack::txt_adjust());
	reorient_[ReorientID::RESET]->setText(lang::LanguagePack::txt_reset());
}

void OrientationTool::Connections() {
	connect(reorient_[ReorientID::REORIENT].get(), SIGNAL(clicked()), this, SIGNAL(sigReorient()));
	connect(reorient_[ReorientID::RESET].get(), SIGNAL(clicked()), this, SIGNAL(sigReorientReset()));

	for (int id = 0; id < ReorientViewID::REORI_VIEW_END; ++id) {
		if (!orient_[id])
		{
			continue;
		}

		connect(orient_[id], static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged),
				[=](int angle) { emit sigOrientRotate(static_cast<ReorientViewID>(id), angle); });
	}
}

void OrientationTool::SetToolTips() {

}
