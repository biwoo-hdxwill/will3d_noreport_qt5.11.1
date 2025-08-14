#include "tmj_task_tool.h"

#include <QBoxLayout>
#include <QButtonGroup>
#include <QCheckBox>
#include <QFrame>
#include <QLabel>
#include <QRadioButton>
#include <QSpinBox>
#include <QToolButton>

#include <Engine/Common/Common/W3LayoutFunctions.h>
#include <Engine/Common/Common/W3Logger.h>
#include <Engine/Common/Common/W3Memory.h>
#include <Engine/Common/Common/W3MessageBox.h>
#include <Engine/Common/Common/W3Theme.h>
#include <Engine/Common/Common/language_pack.h>
#ifndef WILL3D_VIEWER
#include <Engine/Core/W3ProjectIO/project_io_tmj.h>
#endif
#include <Engine/UIModule/UIPrimitive/text_edit.h>

#include "clipping_tool.h"
#include "tool_box.h"

TMJTaskTool::TMJTaskTool(QObject* parent) : BaseTool(parent)
{
	CreateUI();
	ResetUI();
}

TMJTaskTool::~TMJTaskTool()
{
	CW3LayoutFunctions::RemoveWidgetsAll(task_contents_layout_.get());
}
#ifndef WILL3D_VIEWER
void TMJTaskTool::ExportProject(ProjectIOTMJ& out)
{
	out.SaveTMJRect(tmj_rect_[TMJRectID::LEFT_W]->value(),
		tmj_rect_[TMJRectID::LEFT_H]->value(),
		tmj_rect_[TMJRectID::RIGHT_W]->value(),
		tmj_rect_[TMJRectID::RIGHT_H]->value());

	out.SaveLateralParams(lateral_[TMJLateralID::LEFT_INTERVAL]->value(),
		lateral_[TMJLateralID::LEFT_THICKNESS]->value(),
		lateral_[TMJLateralID::RIGHT_INTERVAL]->value(),
		lateral_[TMJLateralID::RIGHT_THICKNESS]->value());

	TmjLayoutType layout_type = GetLayoutType();
	bool mode = layout_type == TmjLayoutType::DEFAULT_3D ? false : true;
	out.SaveTMJMode(mode);

	std::string memo_string = tmj_memo_->toPlainText().toLocal8Bit();
	out.SaveMemo(memo_string);

	out.SaveOrientationAngle(
		orient_tool_->GetOrientDegree(ReorientViewID::ORIENT_A),
		orient_tool_->GetOrientDegree(ReorientViewID::ORIENT_R),
		orient_tool_->GetOrientDegree(ReorientViewID::ORIENT_I));
}

void TMJTaskTool::ImportProject(ProjectIOTMJ& in)
{
	float left_w, left_h, right_w, right_h;
	in.LoadTMJRect(left_w, left_h, right_w, right_h);
	tmj_rect_[TMJRectID::LEFT_W]->setValue(left_w);
	tmj_rect_[TMJRectID::LEFT_H]->setValue(left_h);
	tmj_rect_[TMJRectID::RIGHT_W]->setValue(right_w);
	tmj_rect_[TMJRectID::RIGHT_H]->setValue(right_h);

	float left_i, left_t, right_i, right_t;
	in.LoadLateralParams(left_i, left_t, right_i, right_t);
	lateral_[TMJLateralID::LEFT_INTERVAL]->setValue(left_i);
	lateral_[TMJLateralID::LEFT_THICKNESS]->setValue(left_t);
	lateral_[TMJLateralID::RIGHT_INTERVAL]->setValue(right_i);
	lateral_[TMJLateralID::RIGHT_THICKNESS]->setValue(right_t);

	bool mode;
	in.LoadTMJMode(mode);
	if (mode)
		slotSelectModeUI(Mode::MODE_2D, true);
	else
		slotSelectModeUI(Mode::MODE_3D, true);

	std::string memo_string;
	in.LoadMemo(memo_string);
	if (memo_string.length() != 0)
	{
		tmj_memo_->setText(QString::fromLocal8Bit(memo_string.c_str()));
	}

	float d_a, d_r, d_i;
	in.LoadOrientationAngle(d_a, d_r, d_i);

	orient_tool_->SyncOrientDegreeUIOnly(ReorientViewID::ORIENT_A, d_a);
	orient_tool_->SyncOrientDegreeUIOnly(ReorientViewID::ORIENT_R, d_r);
	orient_tool_->SyncOrientDegreeUIOnly(ReorientViewID::ORIENT_I, d_i);
}
#endif
void TMJTaskTool::ResetUI()
{
	SelectModeUI(Mode::MODE_2D, true);
	task_2d_[Task2D::TMJ_LAYOUT]->setChecked(true);
#ifndef WILL3D_LIGHT
	cut_tool_[CutType::CUT_FREEDRAW]->setChecked(true);
#endif
	tmj_rect_delete_[TMJDirectionType::TMJ_LEFT]->setEnabled(true);
	tmj_rect_delete_[TMJDirectionType::TMJ_RIGHT]->setEnabled(true);
	clip_tool_->ResetUI();
#ifndef WILL3D_LIGHT
	SetEnableCutUI(false);
#endif
}

void TMJTaskTool::UpdateTMJRect(const TMJRectID& id, const double& value)
{
	tmj_rect_[id]->setValue(value);
	tmj_rect_[id]->clearFocus();
}

void TMJTaskTool::UpdateLateralParam(const TMJLateralID& id,
	const double& value)
{
	lateral_[id]->setValue(value);
	lateral_[id]->clearFocus();
}

void TMJTaskTool::InitExternUIs(const OrientationTool::OrientUI& orient_ui,
	const LateralUI& lateral_ui,
	const FrontalUI& frontal_ui,
	const CutControlUI& cut_right,
	const CutControlUI& cut_left)
{
	orient_tool_->InitExternUIs(orient_ui);

	lateral_[TMJLateralID::LEFT_INTERVAL] = lateral_ui.left_interval;
	lateral_[TMJLateralID::LEFT_THICKNESS] = lateral_ui.left_thickness;
	lateral_[TMJLateralID::RIGHT_INTERVAL] = lateral_ui.right_interval;
	lateral_[TMJLateralID::RIGHT_THICKNESS] = lateral_ui.right_thickness;

	tmj_rect_[TMJRectID::LEFT_W] = frontal_ui.left_width;
	tmj_rect_[TMJRectID::LEFT_H] = frontal_ui.left_height;
	tmj_rect_[TMJRectID::RIGHT_W] = frontal_ui.right_width;
	tmj_rect_[TMJRectID::RIGHT_H] = frontal_ui.right_height;

#ifndef WILL3D_LIGHT
	cut_control_[TMJDirectionType::TMJ_LEFT] = cut_left;
	cut_control_[TMJDirectionType::TMJ_RIGHT] = cut_right;
#endif
}

void TMJTaskTool::ResetOrientDegreesUI()
{
	orient_tool_->ResetOrientDegreesUI();
}

int TMJTaskTool::GetOrientDegree(const ReorientViewID& view_type) const
{
	return orient_tool_->GetOrientDegree(view_type);
}

void TMJTaskTool::SetOrientDegrees(const int& degree_a, const int& degree_r,
	const int& degree_i)
{
	orient_tool_->SetOrientDegrees(degree_a, degree_r, degree_i);
}

void TMJTaskTool::SyncOrientDegreeUIOnly(const ReorientViewID& view_type,
	const int& degree_view)
{
	orient_tool_->SyncOrientDegreeUIOnly(view_type, degree_view);
}

float TMJTaskTool::GetLateralParam(const TMJLateralID& id)
{
	return (float)lateral_[id]->value();
}

float TMJTaskTool::GetTMJRectparam(const TMJRectID& id)
{
	return (float)tmj_rect_[id]->value();
}

TmjLayoutType TMJTaskTool::GetLayoutType() const
{
	TmjLayoutType layout_type;
	if (mode_ == MODE_2D)
	{
		if (task_2d_[Task2D::TMJ_LAYOUT]->isChecked())
		{
			layout_type = TmjLayoutType::DEFAULT_2D;
		}
		else if (task_2d_[Task2D::FRONTAL_MAIN_LAYOUT]->isChecked())
		{
			layout_type = TmjLayoutType::FRONTAL_MAIN_2D;
		}
		else if (task_2d_[Task2D::LATERAL_MAIN_LAYOUT]->isChecked())
		{
			layout_type = TmjLayoutType::LATERAL_MAIN_2D;
		}
	}
	else
	{
		layout_type = TmjLayoutType::DEFAULT_3D;
	}
	return layout_type;
}

void TMJTaskTool::LateralIntervalChanged(const TMJLateralID& lateral_id,
	double slider_value)
{
	QDoubleSpinBox* target = lateral_id == TMJLateralID::LEFT_INTERVAL
		? lateral_[TMJLateralID::LEFT_INTERVAL]
		: lateral_[TMJLateralID::RIGHT_INTERVAL];

	double interval;
	if (slider_value == target->minimum() + target->singleStep())
	{
		interval = 1.0;
		target->blockSignals(true);
		target->setValue(interval);
		target->blockSignals(false);
	}
	else
	{
		interval = slider_value;
	}

	emit sigTMJLateralParamChanged(lateral_id, interval);
}

void TMJTaskTool::CounterpartDrawModeOff(const TMJDirectionType & direction)
{
	TMJDirectionType counterpart = direction == TMJDirectionType::TMJ_LEFT
		? TMJDirectionType::TMJ_RIGHT
		: TMJDirectionType::TMJ_LEFT;
	if (tmj_rect_draw_[counterpart]->isChecked())
	{
		tmj_rect_draw_[counterpart]->setChecked(false);
		emit sigTMJDrawRect(counterpart, false);
	}
}

QWidget* TMJTaskTool::GetOrientationWidget()
{
	return orient_tool_->GetWidget();
}
QWidget* TMJTaskTool::GetModeSelectionWidget()
{
	return mode_selection_tool_box_.get();
}
QWidget* TMJTaskTool::GetTaskWidget() { return task_contents_.get(); }

QWidget* TMJTaskTool::GetTMJRectWidget() { return tmj_rect_tool_box_.get(); }
QWidget* TMJTaskTool::GetTMJ2DTaskWidget() { return task_2d_tool_box_.get(); }
QWidget* TMJTaskTool::GetTMJ3DTaskWidget() { return task_3d_tool_box_.get(); }
QWidget* TMJTaskTool::GetMemoWidget() { return memo_tool_box_.get(); }
QVBoxLayout* TMJTaskTool::GetTMJTaskLayout()
{
	return task_contents_layout_.get();
}

/**********************************************************************************************
Sets frontal width lower bound.

Must called when
								- Lateral slice interval changed
								- Lateral view count changed

@param	direction_type	Type of the direction : TMJ_LEFT or TMJ_RIGHT
@param	slice_count   	Number of slices.
 **********************************************************************************************/
void TMJTaskTool::SetFrontalWidthLowerBound(
	const TMJDirectionType& direction_type, const int& slice_count)
{
	TMJLateralID lateral_id = direction_type == TMJDirectionType::TMJ_LEFT
		? TMJLateralID::LEFT_INTERVAL
		: TMJLateralID::RIGHT_INTERVAL;

	TMJRectID roi_id = direction_type == TMJDirectionType::TMJ_LEFT
		? TMJRectID::LEFT_W
		: TMJRectID::RIGHT_W;
	tmj_rect_[roi_id]->blockSignals(true);
	tmj_rect_[roi_id]->setMinimum(lateral_[lateral_id]->value() *
		static_cast<double>(slice_count));
	tmj_rect_[roi_id]->blockSignals(false);
}

void TMJTaskTool::SyncFrontalWidth(const TMJDirectionType& direction_type,
	const float& width)
{
	TMJRectID roi_id = direction_type == TMJDirectionType::TMJ_LEFT
		? TMJRectID::LEFT_W
		: TMJRectID::RIGHT_W;
	if (tmj_rect_[roi_id]->value() != width)
	{
		tmj_rect_[roi_id]->blockSignals(true);
		tmj_rect_[roi_id]->setValue(width);
		tmj_rect_[roi_id]->blockSignals(false);
	}
}

void TMJTaskTool::SyncLateralWidth(const TMJDirectionType& direction_type,
	const float& height)
{
	TMJRectID roi_id = direction_type == TMJDirectionType::TMJ_LEFT
		? TMJRectID::LEFT_H
		: TMJRectID::RIGHT_H;
	if (tmj_rect_[roi_id]->value() != height)
	{
		tmj_rect_[roi_id]->blockSignals(true);
		tmj_rect_[roi_id]->setValue(height);
		tmj_rect_[roi_id]->blockSignals(false);
	}
}

void TMJTaskTool::DrawRectDone(const TMJDirectionType& type)
{
	tmj_rect_delete_[type]->setEnabled(true);
	tmj_rect_draw_[type]->setChecked(false);
}

void TMJTaskTool::slotCutToolChanged()
{
#ifndef WILL3D_LIGHT
	VRCutTool cut_tool = cut_tool_[CutType::CUT_FREEDRAW]->isChecked()
		? VRCutTool::FREEDRAW
		: VRCutTool::POLYGON;

	emit sigTMJCutEnable(true, cut_tool);
#endif
}

void TMJTaskTool::slotSelectLayout()
{
	TmjLayoutType layout_type = GetLayoutType();
#ifndef WILL3D_LIGHT
	if (layout_type != TmjLayoutType::DEFAULT_3D)
	{
		cut_activate_->setChecked(false);
	}
#endif

	emit sigTMJLaoutChanged(layout_type);
}

void TMJTaskTool::slotSelectModeUI(const Mode& mode, bool checked)
{
	SelectModeUI(mode, checked);
	slotSelectLayout();
}

void TMJTaskTool::slotRectDraw(const TMJDirectionType& direction,
	const bool& checked)
{
	if (tmj_rect_delete_[direction]->isEnabled())
	{
		CW3MessageBox msgBox("Will3D", lang::LanguagePack::msg_76(),
			CW3MessageBox::Question);
		if (msgBox.exec())
		{
			slotRectDelete(direction);
		}
		else
		{
			tmj_rect_draw_[direction]->setChecked(false);
			return;
		}
	}

	if (checked)
	{
		CounterpartDrawModeOff(direction);
	}
	emit sigTMJDrawRect(direction, checked);
}

void TMJTaskTool::slotRectDelete(const TMJDirectionType& type)
{
	if (!tmj_rect_delete_[type]->isEnabled()) return;

	emit sigTMJDeleteRect(type);
	tmj_rect_delete_[type]->setEnabled(false);
}

void TMJTaskTool::slotClipStatusChanged()
{
	float upper = static_cast<float>(clip_tool_->GetUpperValue());
	float lower = static_cast<float>(clip_tool_->GetLowerValue());
	float cliping_lower_value = ((lower / 100.0f) - 0.5f) * 2.0f;
	float cliping_upper_value = -((upper / 100.0f) - 0.5f) * 2.0f;

	std::vector<float> values = { cliping_lower_value, cliping_upper_value };

	emit sigTMJClipParamsChanged(
		static_cast<ClipID>(clip_tool_->GetClipPlaneID()), values,
		clip_tool_->IsClipEnable());
}

void TMJTaskTool::CreateUI()
{
	CW3Theme* theme = CW3Theme::getInstance();
	QMargins contentsMargin = theme->getToolVBarSizeInfo().marginContents;
	QMargins boxMargins = theme->getToolVBarSizeInfo().marginBox;
	int spacingS = theme->getToolVBarSizeInfo().spacingS;
	int spacingM = theme->getToolVBarSizeInfo().spacingM;
	int spacingL = theme->getToolVBarSizeInfo().spacingL;

	// create reorientation tool buttons
	orient_tool_.reset(new OrientationTool());

	// create mode selection layout
	QHBoxLayout* mode_selection_layout = new QHBoxLayout;
	mode_selection_layout->setSpacing(0);
	mode_selection_layout->setContentsMargins(ui_tools::kMarginZero);
	QString mode_selection_btn_stylesheet =
		theme->toolTMJModeSelectionStyleSheet();
	for (int id = 0; id < Mode::MODE_END; ++id)
	{
		mode_selection_[id].reset(new QToolButton());
		mode_selection_[id]->setObjectName("TMJModeSelection");
		mode_selection_[id]->setSizePolicy(QSizePolicy::Expanding,
			QSizePolicy::Fixed);
		mode_selection_[id]->setStyleSheet(mode_selection_btn_stylesheet);
		mode_selection_[id]->setCheckable(true);
		mode_selection_layout->addWidget(mode_selection_[id].get());
	}
	mode_selection_[Mode::MODE_2D]->setText(lang::LanguagePack::txt_2d());
	mode_selection_[Mode::MODE_3D]->setText(lang::LanguagePack::txt_3d());

	mode_selection_tool_box_.reset(new QFrame());
	mode_selection_tool_box_->setLayout(mode_selection_layout);
	mode_selection_tool_box_->setContentsMargins(ui_tools::kMarginZero);

	// create rect tool buttons
	QVBoxLayout* tmj_rect_layout = new QVBoxLayout;
	tmj_rect_layout->setSpacing(spacingM);
	tmj_rect_layout->setContentsMargins(ui_tools::kMarginZero);
	QString rect_stylesheet = theme->toolTMJRectStylesheet();

	for (int id = 0; id < TMJDirectionType::TMJ_TYPE_END; ++id)
	{
		QHBoxLayout* rect_item_layout = new QHBoxLayout();
		rect_item_layout->setContentsMargins(ui_tools::kMarginZero);
		rect_item_layout->setSpacing(spacingM);

		QLabel* txt_direction = new QLabel;
		txt_direction->setText(id == 0 ? "R : " : "L : ");
		txt_direction->setObjectName("TMJDirectionText");
		txt_direction->setStyleSheet(rect_stylesheet);
		rect_item_layout->addWidget(txt_direction);

		tmj_rect_draw_[id].reset(new QToolButton());
		tmj_rect_draw_[id]->setContentsMargins(ui_tools::kMarginZero);
		tmj_rect_draw_[id]->setObjectName("TMJRectButton");
		tmj_rect_draw_[id]->setStyleSheet(rect_stylesheet);
		tmj_rect_draw_[id]->setText(lang::LanguagePack::txt_draw());
		tmj_rect_draw_[id]->setCheckable(true);
		rect_item_layout->addWidget(tmj_rect_draw_[id].get());

		tmj_rect_delete_[id].reset(new QToolButton());
		tmj_rect_delete_[id]->setContentsMargins(ui_tools::kMarginZero);
		tmj_rect_delete_[id]->setObjectName("TMJRectButton");
		tmj_rect_delete_[id]->setStyleSheet(rect_stylesheet);
		tmj_rect_delete_[id]->setText(lang::LanguagePack::txt_delete());
		rect_item_layout->addWidget(tmj_rect_delete_[id].get());

		tmj_rect_layout->addLayout(rect_item_layout);
	}

	tmj_rect_tool_box_.reset(new ToolBox());
	tmj_rect_tool_box_->setCaptionName(
		lang::LanguagePack::txt_tmj() + " " + lang::LanguagePack::txt_box(),
		Qt::AlignLeft);
	tmj_rect_tool_box_->addToolLayout(tmj_rect_layout);
	tmj_rect_tool_box_->setContentsMargins(boxMargins);

	// create task 2D tool buttons
	QVBoxLayout* task_2d_layout = new QVBoxLayout();
	task_2d_layout->setContentsMargins(QMargins(spacingS, 0, spacingS, spacingS));
	task_2d_layout->setSpacing(9);

	QString general_text_stylesheet = theme->toolGeneralTextStyleSheet();
	for (int id = 0; id < Task2D::TASK_2D_END; ++id)
	{
		task_2d_[id].reset(new QRadioButton());
		task_2d_[id]->setObjectName("GeneralText");
		task_2d_[id]->setStyleSheet(general_text_stylesheet);
		task_2d_layout->addWidget(task_2d_[id].get());
	}
	task_2d_[Task2D::TMJ_LAYOUT]->setText(lang::LanguagePack::txt_tmj() + " " +
		lang::LanguagePack::txt_layout());
	task_2d_[Task2D::FRONTAL_MAIN_LAYOUT]->setText(lang::LanguagePack::txt_frontal_main());
	task_2d_[Task2D::LATERAL_MAIN_LAYOUT]->setText(lang::LanguagePack::txt_lateral_main());
	task_2d_tool_box_.reset(new ToolBox());
	task_2d_tool_box_->setCaptionName(lang::LanguagePack::txt_task(),
		Qt::AlignLeft);
	task_2d_tool_box_->addToolLayout(task_2d_layout);
	task_2d_tool_box_->setContentsMargins(boxMargins);

	// create 3d tool buttons
	QVBoxLayout* task_3d_layout = new QVBoxLayout();
	task_3d_layout->setContentsMargins(QMargins(spacingS, 0, spacingS, spacingS));
	task_3d_layout->setSpacing(9);

#ifndef WILL3D_LIGHT
	// create cut tool ui
	QVBoxLayout* cut_layout = new QVBoxLayout();
	cut_layout->setContentsMargins(contentsMargin);
	cut_layout->setSpacing(spacingM);

	cut_group_.reset(new QButtonGroup());

	cut_activate_.reset(new QCheckBox);
	cut_activate_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	cut_activate_->setText(lang::LanguagePack::txt_cut());
	cut_activate_->setObjectName("GeneralText");
	cut_activate_->setStyleSheet(general_text_stylesheet);
	cut_layout->addWidget(cut_activate_.get());

	for (int id = 0; id < CutType::CUT_END; ++id)
	{
		cut_tool_[id].reset(new QRadioButton());
		cut_tool_[id]->setObjectName("GeneralText");
		cut_tool_[id]->setStyleSheet(general_text_stylesheet);
		cut_group_->addButton(cut_tool_[id].get());
		cut_layout->addWidget(cut_tool_[id].get());
	}
	cut_tool_[CutType::CUT_FREEDRAW]->setText(
		lang::LanguagePack::txt_free_draw());
	cut_tool_[CutType::CUT_POLYGON]->setText(lang::LanguagePack::txt_polygon());
	task_3d_layout->addLayout(cut_layout);

	QFrame* black_line = new QFrame;
	black_line->setObjectName("blackline");
	black_line->setFrameStyle(QFrame::HLine | QFrame::Raised);
	black_line->setStyleSheet(theme->BlankLineStyleSheet());
	task_3d_layout->addWidget(black_line);
#endif

	// create clipping tool ui
	std::vector<QString> mode_names = {
		lang::LanguagePack::txt_anterior_posterior(),
		lang::LanguagePack::txt_lateral_medial(),
		lang::LanguagePack::txt_top_bottom() };
	clip_tool_.reset(new ClippingTool(
		mode_names, ClippingTool::DirectionType::VERTICAL, false));
	task_3d_layout->addLayout(clip_tool_->GetLayoutOnly());
	task_3d_tool_box_.reset(new ToolBox());
#ifndef WILL3D_LIGHT
	task_3d_tool_box_->setCaptionName(lang::LanguagePack::txt_task(), Qt::AlignLeft);
#else
	task_3d_tool_box_->setCaptionName(lang::LanguagePack::txt_clipping(), Qt::AlignLeft);
#endif
	task_3d_tool_box_->addToolLayout(task_3d_layout);
	task_3d_tool_box_->setContentsMargins(boxMargins);

	// create implant memo
	tmj_memo_.reset(new TextEdit());
	memo_tool_box_.reset(new ToolBox());
	memo_tool_box_->setCaptionName(lang::LanguagePack::txt_memo(), Qt::AlignLeft);
	memo_tool_box_->addToolWidget(tmj_memo_.get());
	memo_tool_box_->setContentsMargins(boxMargins);

	task_contents_layout_.reset(new QVBoxLayout());
	task_contents_layout_->setContentsMargins(2, 2, 2, 2);
	task_contents_layout_->setSpacing(0);
	task_contents_.reset(new QFrame());
	task_contents_->setLayout(task_contents_layout_.get());
}

void TMJTaskTool::SetToolTips() {}

void TMJTaskTool::SelectModeUI(const Mode& mode, const bool& checked)
{
	mode_ = mode;
	if (mode == Mode::MODE_2D)
	{
		mode_selection_[Mode::MODE_2D]->setChecked(false);
		mode_selection_[Mode::MODE_3D]->setChecked(true);
	}
	else
	{
		mode_selection_[Mode::MODE_2D]->setChecked(true);
		mode_selection_[Mode::MODE_3D]->setChecked(false);
	}

	emit sigTMJGetTaskLayout(task_contents_layout_.get());
}

void TMJTaskTool::SetEnableCutUI(const bool& is_enable)
{
#ifndef WILL3D_LIGHT
	for (int id = 0; id < CutType::CUT_END; ++id)
	{
		cut_tool_[id]->setEnabled(is_enable);
		// cut_control_[id].undo->setEnabled(is_enable);
		// cut_control_[id].redo->setEnabled(is_enable);
	}
#endif
}

void TMJTaskTool::Connections()
{
	orient_tool_->Connections();
	connect(orient_tool_.get(), &OrientationTool::sigReorient, this,
		&TMJTaskTool::sigTMJReorient);
	connect(orient_tool_.get(), &OrientationTool::sigReorientReset, this,
		&TMJTaskTool::sigTMJReorientReset);
	connect(orient_tool_.get(), &OrientationTool::sigOrientRotate, this,
		&TMJTaskTool::sigTMJOrientRotate);

	for (int id = 0; id < Mode::MODE_END; ++id)
	{
		connect(mode_selection_[id].get(), &QToolButton::clicked,
			[=](bool checked)
		{
			slotSelectModeUI(static_cast<Mode>(id), checked);
		});
	}

#ifndef WILL3D_LIGHT
	connect(cut_activate_.get(), &QCheckBox::stateChanged, this,
		&TMJTaskTool::slotCutEnable);
	for (int id = 0; id < CutType::CUT_END; ++id)
	{
		connect(cut_tool_[id].get(), &QRadioButton::released, this,
			&TMJTaskTool::slotCutToolChanged);
	}
#endif

	for (int id = 0; id < Task2D::TASK_2D_END; ++id)
	{
		connect(task_2d_[id].get(), &QRadioButton::released, this,
			&TMJTaskTool::slotSelectLayout);
	}

	for (int id = 0; id < TMJDirectionType::TMJ_TYPE_END; ++id)
	{
		const TMJDirectionType d_type = static_cast<TMJDirectionType>(id);
		connect(tmj_rect_draw_[id].get(), &QToolButton::clicked,
			[=](bool checked) { slotRectDraw(d_type, checked); });

		connect(tmj_rect_delete_[id].get(), &QToolButton::clicked,
			[=](bool checked) { slotRectDelete(d_type); });

#ifndef WILL3D_LIGHT
		connect(cut_control_[id].reset, &QToolButton::clicked,
			[=]() { emit sigTMJCutReset(d_type); });
		connect(cut_control_[id].undo, &QToolButton::clicked,
			[=]() { emit sigTMJCutUndo(d_type); });
		connect(cut_control_[id].redo, &QToolButton::clicked,
			[=]() { emit sigTMJCutRedo(d_type); });
#endif
	}

	connect(clip_tool_.get(), SIGNAL(sigEnable(int)), this,
		SLOT(slotClipStatusChanged()));
	connect(clip_tool_.get(), SIGNAL(sigRangeMove(int, int)), this,
		SLOT(slotClipStatusChanged()));
	connect(clip_tool_.get(), SIGNAL(sigRangeSet()), this,
		SLOT(slotClipStatusChanged()));
	connect(clip_tool_.get(), SIGNAL(sigPlaneChanged(int)), this,
		SLOT(slotClipStatusChanged()));

	for (int id = 0; id < TMJRectID::TMJ_RECT_END; ++id)
	{
		connect(tmj_rect_[id],
			static_cast<void (QDoubleSpinBox::*)(double)>(
				&QDoubleSpinBox::valueChanged),
			[=](double value)
		{
			emit sigTMJRectChanged(static_cast<TMJRectID>(id), value);
		});
	}

	connect(lateral_[TMJLateralID::LEFT_THICKNESS],
		static_cast<void (QDoubleSpinBox::*)(double)>(
			&QDoubleSpinBox::valueChanged),
		[=](double value)
	{
		emit sigTMJLateralParamChanged(TMJLateralID::LEFT_THICKNESS, value);
	});

	connect(lateral_[TMJLateralID::RIGHT_THICKNESS],
		static_cast<void (QDoubleSpinBox::*)(double)>(
			&QDoubleSpinBox::valueChanged),
		[=](double value)
	{
		emit sigTMJLateralParamChanged(TMJLateralID::RIGHT_THICKNESS,
			value);
	});

	connect(lateral_[TMJLateralID::LEFT_INTERVAL],
		static_cast<void (QDoubleSpinBox::*)(double)>(
			&QDoubleSpinBox::valueChanged),
		[=](double value)
	{
		LateralIntervalChanged(TMJLateralID::LEFT_INTERVAL, value);
	});

	connect(lateral_[TMJLateralID::RIGHT_INTERVAL],
		static_cast<void (QDoubleSpinBox::*)(double)>(
			&QDoubleSpinBox::valueChanged),
		[=](double value)
	{
		LateralIntervalChanged(TMJLateralID::RIGHT_INTERVAL, value);
	});
}

void TMJTaskTool::slotCutEnable(int state)
{
#ifndef WILL3D_LIGHT
	bool is_enable;
	if (state == Qt::CheckState::Checked)
	{
		is_enable = true;
	}
	else if (state == Qt::CheckState::Unchecked)
	{
		is_enable = false;
	}

	SetEnableCutUI(is_enable);

	VRCutTool cut_tool = cut_tool_[CutType::CUT_FREEDRAW]->isChecked()
		? VRCutTool::FREEDRAW
		: VRCutTool::POLYGON;

	emit sigTMJCutEnable(is_enable, cut_tool);
#endif
}

const bool TMJTaskTool::IsCutEnabled()
{
#ifndef WILL3D_LIGHT
	return cut_activate_->isChecked();
#else
	return false;
#endif
}
