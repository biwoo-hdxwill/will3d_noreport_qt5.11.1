#include "common_task_tool.h"
#include <QAction>
#include <QApplication>
#include <QBoxLayout>
#include <QMenu>
#include <QToolButton>

#include <Engine/Common/Common/W3Theme.h>
#include <Engine/Common/Common/define_view.h>
#include <Engine/Common/Common/language_pack.h>
#include <Engine/Common/Common/global_preferences.h>

#include <Engine/Resource/Resource/implant_resource.h>
#include <Engine/Resource/ResContainer/resource_container.h>
#include "tool_box.h"

using namespace ui_tools;
namespace {
const QMargins kMarginToolBox(12, 5, 12, 5);

const std::vector<QString> kPathTapeIcon = {
	":/image/buttons/btn_tape_line.png",
	":/image/buttons/btn_tape_curve.png",
	":/image/buttons/tape_disabled.png"
};

const std::vector<QString> kPathAnalysisIcon = {
	":/image/buttons/btn_profile.png",
	":/image/buttons/btn_roi.png",
	":/image/buttons/btn_profile_disabled.png",
	":/image/buttons/btn_roi_disabled.png"
};

const std::vector<QString> kPathAnnoIcon = {
	":/image/buttons/btn_draw_arrow.png",
	":/image/buttons/btn_draw_rectangle.png",
	":/image/buttons/btn_draw_circle.png",
	":/image/buttons/btn_draw_line.png",
	":/image/buttons/btn_draw_freedraw.png",
	":/image/buttons/btn_draw_freedraw_disabled.png"
};
} // end of namespace

CommonTaskTool::CommonTaskTool(QObject *parent)
	: BaseTool(parent) 
{
	CreateUI();
	SetToolTips();
	Connections();
}

CommonTaskTool::~CommonTaskTool() 
{

}

common::CommonToolTypeOnOff CommonTaskTool::CurrToolType() {
	return status_.curr_tool;
}

bool CommonTaskTool::RemoveUnfinishiedMeasureTool() {
	if (status_.curr_tool >= common::CommonToolTypeOnOff::M_RULER &&
		status_.curr_tool < common::CommonToolTypeOnOff::M_DEL) {
		emit sigCommonToolOnce(common::CommonToolTypeOnce::M_DEL_INCOMPLETION, true);
		return true;
	}
	return false;
}

bool CommonTaskTool::CancelSelectedTool() {
	if (IsSetCurrTool()) {
		status_.curr_tool = common::CommonToolTypeOnOff::NONE;
		ReleaseOtherButtons();
		emit sigCommonToolOnOff(common::CommonToolTypeOnOff::NONE);
		return true;
	}
	return false;
}

void CommonTaskTool::SetOnlyTRDMode(bool enable) {
	//view_[ViewID::HIDE_TEXT]->setVisible(!enable);
	view_[ViewID::INVERT]->setVisible(!enable);

#ifndef WILL3D_EUROPE
	view_[ViewID::HIDE_UI]->setVisible(!enable);
	view_[ViewID::LIGHT]->setVisible(!enable);
	view_[ViewID::GRID]->setVisible(!enable);
#endif // !WILL3D_EUROPE	

	tool_box_file_->setVisible(!enable);
	tool_box_measure_->setVisible(!enable);
}

void CommonTaskTool::ResetUI() {
	SetTapeButton(ActionTypeTape::LINE);
	SetAnalysisButton(ActionTypeAnalysis::PROFILE);
	SetAnnoButton(ActionTypeAnno::FREEDRAW);

	ReleaseOtherButtons();
	EnableMeasrueButtons(true);

	view_[ViewID::INVERT]->setChecked(false);
	//view_[ViewID::HIDE_TEXT]->setChecked(false);
	view_[ViewID::HIDE_UI]->setChecked(false);
	view_[ViewID::GRID]->setChecked(false);
}

void CommonTaskTool::slotSyncImplantAngleButton() {
	const auto& implant_resource = ResourceContainer::GetInstance()->GetImplantResource();
	if (!implant_resource.IsSetImplant()) {
		measure_[MeasureID::IMPLANT_ANGLE]->setEnabled(false);
		return;
	}

	measure_[MeasureID::IMPLANT_ANGLE]->setEnabled(implant_resource.data().size() > 1 ? true : false);
}

void CommonTaskTool::slotTempSyncMenuEvent() {
  slotViewInvert(view_[ViewID::INVERT]->isChecked());
  //slotViewHideText(view_[ViewID::HIDE_TEXT]->isChecked());
#ifndef WILL3D_EUROPE
  slotViewHideUI(view_[ViewID::HIDE_UI]->isChecked());
  slotViewGrid(view_[ViewID::GRID]->isChecked());
#endif // !WILL3D_EUROPE
  slotMeasureHide(measure_[MeasureID::HIDEMEASURE]->isChecked());
}

void CommonTaskTool::CreateUI() {
	CW3Theme* theme = CW3Theme::getInstance();
	main_layout_.reset(new QHBoxLayout);
	main_layout_->setSpacing(1);
	main_layout_->setContentsMargins(ui_tools::kMarginZero);
	main_layout_->setAlignment(Qt::AlignLeft);

	// create file tool 
	QGridLayout* file_layout = new QGridLayout;
	file_layout->setSpacing(0);
	file_layout->setHorizontalSpacing(2);

	QString common_tool_stylesheet = theme->toolIconButtonStyleSheet();
	for (int id = 0; id < FileID::FID_END; ++id) 
	{
		file_[id].reset(new QToolButton());
		file_[id]->setContentsMargins(kMarginZero);
		file_[id]->setStyleSheet(common_tool_stylesheet);
		file_[id]->setStyle(theme->toolIconButtonStyle());

		file_layout->addWidget(file_[id].get(), 0, id);
	}

#ifdef WILL3D_EUROPE
	file_[FileID::CD_EXPORT].get()->setVisible(false);
#endif // WILL3D_EUROPE
	
	tool_box_file_.reset(new ToolBox());
	tool_box_file_->setCaptionName(lang::LanguagePack::txt_file());
	tool_box_file_->addToolLayout(file_layout);
	tool_box_file_->setContentsMargins(kMarginToolBox);
	tool_box_file_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
	main_layout_->addWidget(tool_box_file_.get());

	// create view tool
	QGridLayout* view_layout = new QGridLayout;
	view_layout->setSpacing(0);
	view_layout->setHorizontalSpacing(2);

	for (int id = 0; id < ViewID::VID_END; ++id) 
	{		
		view_[id].reset(new QToolButton());
		view_[id]->setContentsMargins(kMarginZero);
		view_[id]->setStyleSheet(common_tool_stylesheet);
		view_[id]->setStyle(theme->toolIconButtonStyle());

		view_layout->addWidget(view_[id].get(), 0, id);
	}
#ifdef WILL3D_EUROPE
	view_[ViewID::LIGHT].get()->setVisible(false);
	view_[ViewID::GRID].get()->setVisible(false);
	view_[ViewID::HIDE_UI].get()->setVisible(false);
#endif // WILL3D_EUROPE

	tool_box_view_.reset(new ToolBox());
	tool_box_view_->setCaptionName(lang::LanguagePack::txt_view());
	tool_box_view_->addToolLayout(view_layout);
	tool_box_view_->setContentsMargins(kMarginToolBox);
	tool_box_view_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
	main_layout_->addWidget(tool_box_view_.get());

	// create measure tool
	for (int id = 0; id < MeasureID::MID_END; ++id) {
		measure_[id].reset(new QToolButton());
		measure_[id]->setContentsMargins(kMarginZero);
		measure_[id]->setStyleSheet(common_tool_stylesheet);
		measure_[id]->setStyle(theme->toolIconButtonStyle());
	}

	int button_height_with_menu = theme->size_tool_icon().height() - theme->tool_icon_sub_menu_height();
	measure_[MeasureID::TAPE]->setFixedHeight(button_height_with_menu);
	measure_[MeasureID::ANALYSIS]->setFixedHeight(button_height_with_menu);
	measure_[MeasureID::DRAW]->setFixedHeight(button_height_with_menu);
	measure_[MeasureID::DEL]->setFixedHeight(button_height_with_menu);

	measure_list_.reset(new QToolButton());
	measure_list_->setContentsMargins(kMarginZero);
	measure_list_->setStyleSheet(common_tool_stylesheet);
	measure_list_->setStyle(theme->toolIconButtonStyle());

	QSettings settings(GlobalPreferences::GetInstance()->ini_path(), QSettings::IniFormat);
	bool enable_draw_line = settings.value("INTERFACE/enable_draw_line", false).toBool();

	for (int id = 0; id < MeasureMenuID::MENUID_END; ++id) {
		measure_menu_[id].reset(new QToolButton());
		measure_menu_[id]->setPopupMode(QToolButton::MenuButtonPopup);
		measure_menu_[id]->setObjectName("Menu");

		QMenu* menu = new QMenu(measure_menu_[id].get());
		menu->setFont(QApplication::font());
		switch (id) {
		case MeasureMenuID::M_TAPE:
			measure_menu_actions_.tape_line = menu->addAction(lang::LanguagePack::txt_line());
			measure_menu_actions_.tape_line->setIcon(QIcon(":/image/buttons/btn_tape_line_small.png"));
			measure_menu_actions_.tape_curve = menu->addAction(lang::LanguagePack::txt_curve());
			measure_menu_actions_.tape_curve->setIcon(QIcon(":/image/buttons/btn_tape_curve_small.png"));
			break;
		case MeasureMenuID::M_ANALYSIS:
			measure_menu_actions_.analysis_profile = menu->addAction(lang::LanguagePack::txt_profile());
			measure_menu_actions_.analysis_profile->setIcon(QIcon(":/image/buttons/btn_profile_small.png"));
			measure_menu_actions_.analysis_roi = menu->addAction(lang::LanguagePack::txt_roi());
			measure_menu_actions_.analysis_roi->setIcon(QIcon(":/image/buttons/btn_roi_small.png"));
			break;
		case MeasureMenuID::M_DRAW:
			measure_menu_actions_.anno_arrow = menu->addAction(lang::LanguagePack::txt_arrow());
			measure_menu_actions_.anno_arrow->setIcon(QIcon(":/image/buttons/btn_draw_small_arrow.png"));
			measure_menu_actions_.anno_rect = menu->addAction(lang::LanguagePack::txt_rectangle());
			measure_menu_actions_.anno_rect->setIcon(QIcon(":/image/buttons/btn_draw_small_rectangle.png"));
			measure_menu_actions_.anno_circle = menu->addAction(lang::LanguagePack::txt_circle());
			measure_menu_actions_.anno_circle->setIcon(QIcon(":/image/buttons/btn_draw_small_circle.png"));
			if (enable_draw_line)
			{
				measure_menu_actions_.anno_line = menu->addAction(lang::LanguagePack::txt_line());
				measure_menu_actions_.anno_line->setIcon(QIcon(":/image/buttons/btn_draw_small_line.png"));
			}
			measure_menu_actions_.anno_freedraw = menu->addAction(lang::LanguagePack::txt_free_draw());
			measure_menu_actions_.anno_freedraw->setIcon(QIcon(":/image/buttons/btn_draw_small_freedraw.png"));
			break;
		case MeasureMenuID::M_DELETE:
			measure_menu_actions_.del_one = menu->addAction(lang::LanguagePack::txt_delete());
			measure_menu_actions_.del_one->setIcon(QIcon(":/image/buttons/btn_delete_small.png"));
			measure_menu_actions_.del_all = menu->addAction(lang::LanguagePack::txt_delete_all());
			measure_menu_actions_.del_all->setIcon(QIcon(":/image/buttons/btn_delete_all_small.png"));
			break;
		default:
			break;
		}
		menu->setStyle(theme->toolMenuIconStyle());
		menu->setStyleSheet(QString(
			"QMenu { border: 1px; background: #FF1C1E28; }"
			"QMenu::item { background-color: transparent; }"
			"QMenu::item::selected { background-color: #FF434961; }"
		));

		measure_menu_[id]->setMenu(menu);
		measure_menu_[id]->setStyleSheet(common_tool_stylesheet);
	}

	QVBoxLayout* tape_layout = new QVBoxLayout();
	tape_layout->setSpacing(0);
	tape_layout->setContentsMargins(kMarginZero);
	tape_layout->addWidget(measure_[MeasureID::TAPE].get(), 1);
	tape_layout->addWidget(measure_menu_[MeasureMenuID::M_TAPE].get());

	QVBoxLayout* analysis_layout = new QVBoxLayout();
	analysis_layout->setSpacing(0);
	analysis_layout->setContentsMargins(kMarginZero);
	analysis_layout->addWidget(measure_[MeasureID::ANALYSIS].get());
	analysis_layout->setStretch(0, 9);
	analysis_layout->addWidget(measure_menu_[MeasureMenuID::M_ANALYSIS].get());
	analysis_layout->setStretch(1, 1);

	QVBoxLayout* draw_layout = new QVBoxLayout();
	draw_layout->setSpacing(0);
	draw_layout->setContentsMargins(kMarginZero);
	draw_layout->addWidget(measure_[MeasureID::DRAW].get());
	draw_layout->setStretch(0, 9);
	draw_layout->addWidget(measure_menu_[MeasureMenuID::M_DRAW].get());
	draw_layout->setStretch(1, 1);

	QVBoxLayout* delete_layout = new QVBoxLayout();
	delete_layout->setSpacing(0);
	delete_layout->setContentsMargins(kMarginZero);
	delete_layout->addWidget(measure_[MeasureID::DEL].get());
	delete_layout->setStretch(0, 9);
	delete_layout->addWidget(measure_menu_[MeasureMenuID::M_DELETE].get());
	delete_layout->setStretch(1, 1);

	QHBoxLayout* measure_layout = new QHBoxLayout();
	measure_layout->setAlignment(Qt::AlignmentFlag::AlignTop);
	measure_layout->setSpacing(2);
	measure_layout->addWidget(measure_[MeasureID::RULER].get());
	measure_layout->addLayout(tape_layout);
	measure_layout->addWidget(measure_[MeasureID::ANGLE].get());
	measure_layout->addLayout(analysis_layout);
	measure_layout->addWidget(measure_[MeasureID::AREA].get());
	measure_layout->addLayout(draw_layout);
	measure_layout->addWidget(measure_[MeasureID::NOTE].get());
	measure_layout->addWidget(measure_[MeasureID::HIDEMEASURE].get());
	measure_layout->addLayout(delete_layout);

#ifdef WILL3D_EUROPE
	measure_[MeasureID::IMPLANT_ANGLE].get()->setVisible(false);
	measure_list_.get()->setVisible(false);
#else
	measure_layout->addWidget(measure_[MeasureID::IMPLANT_ANGLE].get());
	measure_layout->addWidget(measure_list_.get());
#endif // WILL3D_EUROPE

	SetImplantAngleVisible(false);

	tool_box_measure_.reset(new ToolBox());
	tool_box_measure_->setCaptionName(lang::LanguagePack::txt_measure());
	tool_box_measure_->addToolLayout(measure_layout);
	tool_box_measure_->setContentsMargins(kMarginToolBox);
	tool_box_measure_->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
	main_layout_->addWidget(tool_box_measure_.get());

	file_[FileID::CAPTURE]->setObjectName("Capture");
	file_[FileID::SAVE]->setObjectName("Save");
	file_[FileID::CD_EXPORT]->setObjectName("CDExport");
	file_[FileID::PRINT]->setObjectName("Print");
	file_[FileID::PACS]->setObjectName("PACS");

	if (!GlobalPreferences::GetInstance()->preferences_.pacs.pacs_on)
	{
		file_[FileID::PACS]->setVisible(false);
	}

	view_[ViewID::RESET]->setObjectName("Reset");
	view_[ViewID::FIT]->setObjectName("Fit");
	view_[ViewID::PAN]->setObjectName("Pan");
	view_[ViewID::ZOOM]->setObjectName("Zoom");
	view_[ViewID::LIGHT]->setObjectName("Light");
	view_[ViewID::INVERT]->setObjectName("Invert");
	//view_[ViewID::HIDE_TEXT]->setObjectName("Hide");
	view_[ViewID::HIDE_UI]->setObjectName("HideUI");
	view_[ViewID::GRID]->setObjectName("Grid");
	measure_[MeasureID::RULER]->setObjectName("Ruler");
	measure_[MeasureID::TAPE]->setObjectName("Tape");
	measure_[MeasureID::ANGLE]->setObjectName("Angle");
	measure_[MeasureID::ANALYSIS]->setObjectName("Analysis");
	measure_[MeasureID::AREA]->setObjectName("Area");
	measure_[MeasureID::DRAW]->setObjectName("Draw");
	measure_[MeasureID::NOTE]->setObjectName("Note");
	measure_[MeasureID::HIDEMEASURE]->setObjectName("HideMeasure");
	measure_[MeasureID::DEL]->setObjectName("Delete");
	measure_[MeasureID::IMPLANT_ANGLE]->setObjectName("ImplAngle");
	measure_list_->setObjectName("MeasureList");

	view_[ViewID::PAN]->setCheckable(true);
	view_[ViewID::ZOOM]->setCheckable(true);
	view_[ViewID::LIGHT]->setCheckable(true);
	view_[ViewID::INVERT]->setCheckable(true);
	//view_[ViewID::HIDE_TEXT]->setCheckable(true);
	view_[ViewID::HIDE_UI]->setCheckable(true);
	view_[ViewID::GRID]->setCheckable(true);
	measure_[MeasureID::RULER]->setCheckable(true);
	measure_[MeasureID::TAPE]->setCheckable(true);
	measure_[MeasureID::ANGLE]->setCheckable(true);
	measure_[MeasureID::ANALYSIS]->setCheckable(true);
	measure_[MeasureID::AREA]->setCheckable(true);
	measure_[MeasureID::DRAW]->setCheckable(true);
	measure_[MeasureID::NOTE]->setCheckable(true);
	measure_[MeasureID::DEL]->setCheckable(true);
	measure_[MeasureID::HIDEMEASURE]->setCheckable(true);

	measure_[MeasureID::IMPLANT_ANGLE]->setEnabled(false);

//20250214 LIN WILL3D VIEWER에 Save기능 남김
#ifdef WILL3D_VIEWER
	//file_[FileID::SAVE]->setVisible(false);
	file_[FileID::SAVE]->setVisible(true);
	file_[FileID::CD_EXPORT]->setVisible(false);
	measure_[MeasureID::IMPLANT_ANGLE]->setVisible(false);
#endif	
}

void CommonTaskTool::Connections() {
	connect(file_[FileID::CAPTURE].get(), SIGNAL(clicked()), this, SLOT(slotFileCapture()));
	connect(file_[FileID::SAVE].get(), SIGNAL(clicked()), this, SLOT(slotFileSaveProject()));
	connect(file_[FileID::CD_EXPORT].get(), SIGNAL(clicked()), this, SLOT(slotFileCDExport()));
	connect(file_[FileID::PRINT].get(), SIGNAL(clicked()), this, SLOT(slotFilePrint()));
	connect(file_[FileID::PACS].get(), SIGNAL(clicked()), this, SLOT(slotPACS()));

	connect(view_[ViewID::RESET].get(), SIGNAL(clicked()), this, SLOT(slotViewReset()));
	connect(view_[ViewID::FIT].get(), SIGNAL(clicked()), this, SLOT(slotViewFit()));
	connect(view_[ViewID::PAN].get(), SIGNAL(toggled(bool)), this, SLOT(slotViewPan(bool)));
	connect(view_[ViewID::ZOOM].get(), SIGNAL(toggled(bool)), this, SLOT(slotViewZoom(bool)));
	connect(view_[ViewID::LIGHT].get(), SIGNAL(toggled(bool)), this, SLOT(slotViewLight(bool)));
	connect(view_[ViewID::INVERT].get(), SIGNAL(toggled(bool)), this, SLOT(slotViewInvert(bool)));
	//connect(view_[ViewID::HIDE_TEXT].get(), SIGNAL(toggled(bool)), this, SLOT(slotViewHideText(bool)));
	connect(view_[ViewID::HIDE_UI].get(), SIGNAL(toggled(bool)), this, SLOT(slotViewHideUI(bool)));
	connect(view_[ViewID::GRID].get(), SIGNAL(toggled(bool)), this, SLOT(slotViewGrid(bool)));

	connect(measure_list_.get(), &QToolButton::clicked, this, &CommonTaskTool::sigCommonMeasureListOn);
	connect(measure_[MeasureID::RULER].get(), SIGNAL(toggled(bool)), this, SLOT(slotMeasureRuler(bool)));
	connect(measure_[MeasureID::TAPE].get(), SIGNAL(toggled(bool)), this, SLOT(slotMeasureTape(bool)));
	connect(measure_[MeasureID::ANGLE].get(), SIGNAL(toggled(bool)), this, SLOT(slotMeasureAngle(bool)));
	connect(measure_[MeasureID::ANALYSIS].get(), SIGNAL(toggled(bool)), this, SLOT(slotMeasureAnalysis(bool)));
	connect(measure_[MeasureID::AREA].get(), SIGNAL(toggled(bool)), this, SLOT(slotMeasureArea(bool)));
	connect(measure_[MeasureID::HIDEMEASURE].get(), SIGNAL(toggled(bool)), this, SLOT(slotMeasureHide(bool)));
	connect(measure_[MeasureID::DRAW].get(), SIGNAL(toggled(bool)), this, SLOT(slotMeasureDraw(bool)));
	connect(measure_[MeasureID::NOTE].get(), SIGNAL(toggled(bool)), this, SLOT(slotMeasureNote(bool)));
	connect(measure_[MeasureID::IMPLANT_ANGLE].get(), SIGNAL(clicked()), this, SLOT(slotMeasureImplantAngle()));
	connect(measure_[MeasureID::DEL].get(), SIGNAL(toggled(bool)), this, SLOT(slotMeasureDelete(bool)));

	connect(measure_menu_actions_.tape_line, SIGNAL(triggered()), this, SLOT(slotActTapeLine()));
	connect(measure_menu_actions_.tape_curve, SIGNAL(triggered()), this, SLOT(slotActTapeCurve()));
	connect(measure_menu_actions_.analysis_profile, SIGNAL(triggered()), this, SLOT(slotActAnalysisProfile()));
	connect(measure_menu_actions_.analysis_roi, SIGNAL(triggered()), this, SLOT(slotActAnalysisROI()));
	connect(measure_menu_actions_.anno_arrow, SIGNAL(triggered()), this, SLOT(slotActDrawArrow()));
	connect(measure_menu_actions_.anno_rect, SIGNAL(triggered()), this, SLOT(slotActDrawRect()));
	connect(measure_menu_actions_.anno_circle, SIGNAL(triggered()), this, SLOT(slotActDrawCircle()));
	if (measure_menu_actions_.anno_line)
	{
		connect(measure_menu_actions_.anno_line, SIGNAL(triggered()), this, SLOT(slotActDrawLine()));
	}
	connect(measure_menu_actions_.anno_freedraw, SIGNAL(triggered()), this, SLOT(slotActDrawFreedraw()));
	connect(measure_menu_actions_.del_one, SIGNAL(triggered()), this, SLOT(slotActDelOne()));
	connect(measure_menu_actions_.del_all, SIGNAL(triggered()), this, SLOT(slotActDelAll()));
}

void CommonTaskTool::SetToolTips() {
	using namespace lang;
	view_[ViewID::RESET]->setToolTip(LanguagePack::txt_reset());
	view_[ViewID::PAN]->setToolTip(LanguagePack::txt_pan());
	view_[ViewID::ZOOM]->setToolTip(LanguagePack::txt_zoom());
	view_[ViewID::FIT]->setToolTip(LanguagePack::txt_fit());
	view_[ViewID::LIGHT]->setToolTip(LanguagePack::txt_light());
	view_[ViewID::INVERT]->setToolTip(LanguagePack::txt_invert());
	//view_[ViewID::HIDE_TEXT]->setToolTip(LanguagePack::txt_hide());
	view_[ViewID::HIDE_UI]->setToolTip(LanguagePack::txt_hide_ui());
	view_[ViewID::GRID]->setToolTip(LanguagePack::txt_grid());

	measure_list_->setToolTip(LanguagePack::txt_measure_list());
	measure_[MeasureID::RULER]->setToolTip(LanguagePack::txt_ruler());
	measure_[MeasureID::TAPE]->setToolTip(LanguagePack::txt_tape());
	measure_[MeasureID::ANGLE]->setToolTip(LanguagePack::txt_angle());
	measure_[MeasureID::ANALYSIS]->setToolTip(LanguagePack::txt_analysis());
	measure_[MeasureID::AREA]->setToolTip(LanguagePack::txt_area());
	measure_[MeasureID::HIDEMEASURE]->setToolTip(LanguagePack::txt_hide());
	measure_[MeasureID::DRAW]->setToolTip(LanguagePack::txt_free_draw());
	measure_[MeasureID::NOTE]->setToolTip(LanguagePack::txt_note());
	measure_[MeasureID::DEL]->setToolTip(LanguagePack::txt_delete());
	measure_[MeasureID::IMPLANT_ANGLE]->setToolTip(LanguagePack::txt_implant() + " " + LanguagePack::txt_angle());

	file_[FileID::CAPTURE]->setToolTip(LanguagePack::txt_capture());
	file_[FileID::SAVE]->setToolTip(LanguagePack::txt_save());
	file_[FileID::CD_EXPORT]->setToolTip(LanguagePack::txt_cd_usb_export());
	file_[FileID::PRINT]->setToolTip(LanguagePack::txt_print());
	file_[FileID::PACS]->setToolTip(LanguagePack::txt_pacs());
}

common::CommonToolTypeOnOff CommonTaskTool::GetToolTypeFromTapeType() {
	switch (status_.type_tape) {
	case ActionTypeTape::LINE:
		return common::CommonToolTypeOnOff::M_TAPELINE;
	case ActionTypeTape::CURVE:
		return common::CommonToolTypeOnOff::M_TAPECURVE;
	}
	return common::CommonToolTypeOnOff::NONE;
}

common::CommonToolTypeOnOff CommonTaskTool::GetToolTypeFromAnalysisType()
{
	switch (status_.type_analysis)
	{
	case ActionTypeAnalysis::PROFILE:
		return common::CommonToolTypeOnOff::M_PROFILE;
	case ActionTypeAnalysis::ROI:
		return common::CommonToolTypeOnOff::M_ROI;
	}
	return common::CommonToolTypeOnOff::NONE;
}

common::CommonToolTypeOnOff CommonTaskTool::GetToolTypeFromAnnoType() {
	switch (status_.type_anno) {
	case ActionTypeAnno::ARROW:
		return common::CommonToolTypeOnOff::M_ARROW;
	case ActionTypeAnno::RECT:
		return common::CommonToolTypeOnOff::M_RECTANGLE;
	case ActionTypeAnno::CIRCLE:
		return common::CommonToolTypeOnOff::M_CIRCLE;
	case ActionTypeAnno::LINE:
		return common::CommonToolTypeOnOff::M_LINE;
	case ActionTypeAnno::FREEDRAW:
		return common::CommonToolTypeOnOff::M_FREEDRAW;
	}
	return common::CommonToolTypeOnOff::NONE;
}

void CommonTaskTool::SetTapeButton(const ActionTypeTape& tape_type) {
	if (status_.type_tape == tape_type)
		return;

	status_.type_tape = tape_type;
	auto theme = CW3Theme::getInstance();
	QString style_sheet_text;
	switch (status_.type_tape) {
	case ActionTypeTape::LINE:
		style_sheet_text = theme->ChangeToolButtonImageStyleSheet(kPathTapeIcon.at(0),
																  kPathTapeIcon.at(2));
		break;
	case ActionTypeTape::CURVE:
		style_sheet_text = theme->ChangeToolButtonImageStyleSheet(kPathTapeIcon.at(1),
																  kPathTapeIcon.at(2));
		break;
	}
	measure_[MeasureID::TAPE]->setStyleSheet(style_sheet_text);
}

void CommonTaskTool::SetAnalysisButton(const ActionTypeAnalysis& analysis_type)
{
	if (status_.type_analysis == analysis_type)
	{
		return;
	}

	status_.type_analysis = analysis_type;
	auto theme = CW3Theme::getInstance();
	QString style_sheet_text;
	switch (status_.type_analysis)
	{
	case ActionTypeAnalysis::PROFILE:
		style_sheet_text = theme->ChangeToolButtonImageStyleSheet(kPathAnalysisIcon.at(0), kPathAnalysisIcon.at(2));
		break;
	case ActionTypeAnalysis::ROI:
		style_sheet_text = theme->ChangeToolButtonImageStyleSheet(kPathAnalysisIcon.at(1), kPathAnalysisIcon.at(3));
		break;
	}
	measure_[MeasureID::ANALYSIS]->setStyleSheet(style_sheet_text);
}

void CommonTaskTool::SetAnnoButton(const ActionTypeAnno& anno_type) {
	if (status_.type_anno == anno_type)
		return;

	status_.type_anno = anno_type;
	auto theme = CW3Theme::getInstance();
	QString style_sheet_text;
	switch (status_.type_anno) {
	case ActionTypeAnno::ARROW:
		style_sheet_text = theme->ChangeToolButtonImageStyleSheet(kPathAnnoIcon.at(0),
																  kPathAnnoIcon.at(5));
		break;
	case ActionTypeAnno::RECT:
		style_sheet_text = theme->ChangeToolButtonImageStyleSheet(kPathAnnoIcon.at(1),
																  kPathAnnoIcon.at(5));
		break;
	case ActionTypeAnno::CIRCLE:
		style_sheet_text = theme->ChangeToolButtonImageStyleSheet(kPathAnnoIcon.at(2),
																  kPathAnnoIcon.at(5));
		break;
	case ActionTypeAnno::LINE:
		style_sheet_text = theme->ChangeToolButtonImageStyleSheet(kPathAnnoIcon.at(3), kPathAnnoIcon.at(5));
		break;
	case ActionTypeAnno::FREEDRAW:
		style_sheet_text = theme->ChangeToolButtonImageStyleSheet(kPathAnnoIcon.at(4),
																  kPathAnnoIcon.at(5));
		break;
	}
	measure_[MeasureID::DRAW]->setStyleSheet(style_sheet_text);
}

void CommonTaskTool::ReleaseOtherButtons(QToolButton * exception) {
	if (exception == nullptr) {
		view_[ViewID::PAN]->setChecked(false);
		view_[ViewID::ZOOM]->setChecked(false);
		view_[ViewID::LIGHT]->setChecked(false);
		measure_[MeasureID::RULER]->setChecked(false);
		measure_[MeasureID::TAPE]->setChecked(false);
		measure_[MeasureID::ANGLE]->setChecked(false);
		measure_[MeasureID::ANALYSIS]->setChecked(false);
		measure_[MeasureID::AREA]->setChecked(false);
		measure_[MeasureID::DRAW]->setChecked(false);
		measure_[MeasureID::NOTE]->setChecked(false);
		measure_[MeasureID::DEL]->setChecked(false);
	} else {
		if (exception != view_[ViewID::PAN].get())
			view_[ViewID::PAN]->setChecked(false);

		if (exception != view_[ViewID::ZOOM].get())
			view_[ViewID::ZOOM]->setChecked(false);

		if (exception != view_[ViewID::LIGHT].get())
			view_[ViewID::LIGHT]->setChecked(false);

		if (exception != measure_[MeasureID::RULER].get())
			measure_[MeasureID::RULER]->setChecked(false);

		if (exception != measure_[MeasureID::TAPE].get())
			measure_[MeasureID::TAPE]->setChecked(false);

		if (exception != measure_[MeasureID::ANGLE].get())
			measure_[MeasureID::ANGLE]->setChecked(false);

		if (exception != measure_[MeasureID::ANALYSIS].get())
			measure_[MeasureID::ANALYSIS]->setChecked(false);

		if (exception != measure_[MeasureID::AREA].get())
			measure_[MeasureID::AREA]->setChecked(false);

		if (exception != measure_[MeasureID::DRAW].get())
			measure_[MeasureID::DRAW]->setChecked(false);

		if (exception != measure_[MeasureID::NOTE].get())
			measure_[MeasureID::NOTE]->setChecked(false);

		if (exception != measure_[MeasureID::DEL].get())
			measure_[MeasureID::DEL]->setChecked(false);
	}
}

void CommonTaskTool::EnableAllButtons(bool enable) {
	if (enable == status_.tools_enable_)
		return;

	for (int id = 0; id < FileID::FID_END; ++id)
		file_[id]->setEnabled(enable);

	for (int id = 0; id < ViewID::VID_END; ++id)
		view_[id]->setEnabled(enable);

	measure_list_->setEnabled(enable);
	for (int id = 0; id < MeasureID::MID_END; ++id) {
		if (id == MeasureID::IMPLANT_ANGLE)
			continue;
		measure_[id]->setEnabled(enable);
	}

	for (int id = 0; id < MeasureMenuID::MENUID_END; ++id)
		measure_menu_[id]->setEnabled(enable);

	status_.tools_enable_ = enable;
}

QLayout * CommonTaskTool::GetLayout() {
	return main_layout_.get();
}

void CommonTaskTool::SetTaskToolOnOff(const common::CommonToolTypeOnOff& type, bool on) {
	status_.curr_tool = on ? type : common::CommonToolTypeOnOff::NONE;
	emit sigCommonToolOnOff(status_.curr_tool);
}

void CommonTaskTool::EnableMeasrueButtons(bool enable) {
	measure_list_->setEnabled(enable);
	for (int id = 0; id < MeasureID::MID_END; ++id) {
		if (id == MeasureID::HIDEMEASURE)
			continue;

		measure_[id]->setEnabled(enable);
	}

	for (int id = 0; id < MeasureMenuID::MENUID_END; ++id)
		measure_menu_[id]->setEnabled(enable);

	slotSyncImplantAngleButton();
}

bool CommonTaskTool::IsSetCurrTool() {
	return status_.curr_tool != common::CommonToolTypeOnOff::NONE;
}

void CommonTaskTool::slotFileSaveProject() {
	emit sigFileTool(CommonToolTypeFile::SAVE);
}

void CommonTaskTool::slotFileCapture() {
	emit sigFileTool(CommonToolTypeFile::CAPTURE);
}

void CommonTaskTool::slotFilePrint() {
	emit sigFileTool(CommonToolTypeFile::PRINT);
}

void CommonTaskTool::slotFileCDExport() {
	emit sigFileTool(CommonToolTypeFile::CDEXPORT);
}

void CommonTaskTool::slotPACS()
{
	emit sigFileTool(CommonToolTypeFile::PACS);
}

void CommonTaskTool::slotViewReset() {
	ReleaseOtherButtons();
	emit sigCommonToolOnce(common::CommonToolTypeOnce::V_RESET, true);
}

void CommonTaskTool::slotViewFit() {
	emit sigCommonToolOnce(common::CommonToolTypeOnce::V_FIT, true);
}

void CommonTaskTool::slotViewPan(bool on) {
	if (on && status_.curr_tool != common::CommonToolTypeOnOff::V_PAN)
		ReleaseOtherButtons(view_[ViewID::PAN].get());

	SetTaskToolOnOff(common::CommonToolTypeOnOff::V_PAN, on);
}

void CommonTaskTool::slotViewZoom(bool on) {
	if (on && status_.curr_tool != common::CommonToolTypeOnOff::V_ZOOM)
		ReleaseOtherButtons(view_[ViewID::ZOOM].get());

	SetTaskToolOnOff(common::CommonToolTypeOnOff::V_ZOOM, on);
}

void CommonTaskTool::slotViewLight(bool on) {
	if (on && status_.curr_tool != common::CommonToolTypeOnOff::V_LIGHT)
		ReleaseOtherButtons(view_[ViewID::LIGHT].get());

	SetTaskToolOnOff(common::CommonToolTypeOnOff::V_LIGHT, on);
}

void CommonTaskTool::slotViewInvert(bool on) {
	emit sigCommonToolOnce(common::CommonToolTypeOnce::V_INVERT, on);
}

void CommonTaskTool::slotViewHideText(bool on) {
	if (on)
		view_[ViewID::HIDE_UI]->setChecked(false);

	emit sigCommonToolOnce(common::CommonToolTypeOnce::V_HIDE_TXT, on);
}

void CommonTaskTool::slotViewHideUI(bool on) {
	if (on) {
		//view_[ViewID::HIDE_TEXT]->setChecked(false);
		view_[ViewID::GRID]->setChecked(false);
	}
	emit sigCommonToolOnce(common::CommonToolTypeOnce::V_HIDE_UI, on);
}

void CommonTaskTool::slotViewGrid(bool on) {
	if (on)
		view_[ViewID::HIDE_UI]->setChecked(false);

	emit sigCommonToolOnce(common::CommonToolTypeOnce::V_GRID, on);
}

void CommonTaskTool::slotMeasureRuler(bool on) {
	if (on && status_.curr_tool != common::CommonToolTypeOnOff::M_RULER)
		ReleaseOtherButtons(measure_[MeasureID::RULER].get());

	SetTaskToolOnOff(common::CommonToolTypeOnOff::M_RULER, on);
}

void CommonTaskTool::slotMeasureTape(bool on) {
	common::CommonToolTypeOnOff tape_type = GetToolTypeFromTapeType();
	if (on && status_.curr_tool != tape_type)
		ReleaseOtherButtons(measure_[MeasureID::TAPE].get());

	SetTaskToolOnOff(tape_type, on);
}

void CommonTaskTool::slotMeasureAngle(bool on) {
	if (on && status_.curr_tool != common::CommonToolTypeOnOff::M_ANGLE)
		ReleaseOtherButtons(measure_[MeasureID::ANGLE].get());

	SetTaskToolOnOff(common::CommonToolTypeOnOff::M_ANGLE, on);
}

void CommonTaskTool::slotMeasureAnalysis(bool on) 
{
	common::CommonToolTypeOnOff analysis_type = GetToolTypeFromAnalysisType();
	if (on && status_.curr_tool != analysis_type)
	{
		ReleaseOtherButtons(measure_[MeasureID::ANALYSIS].get());
	}

	SetTaskToolOnOff(analysis_type, on);
}

void CommonTaskTool::slotMeasureArea(bool on) {
	if (on && status_.curr_tool != common::CommonToolTypeOnOff::M_AREALINE)
		ReleaseOtherButtons(measure_[MeasureID::AREA].get());

	SetTaskToolOnOff(common::CommonToolTypeOnOff::M_AREALINE, on);
}

void CommonTaskTool::slotMeasureHide(bool on) {
	EnableMeasrueButtons(!on);
	CancelSelectedTool();
	emit sigCommonToolOnce(common::CommonToolTypeOnce::M_HIDE_M, on);
}

void CommonTaskTool::slotMeasureDraw(bool on) {
	common::CommonToolTypeOnOff anno_type = GetToolTypeFromAnnoType();
	if (on && status_.curr_tool != anno_type)
		ReleaseOtherButtons(measure_[MeasureID::DRAW].get());

	SetTaskToolOnOff(anno_type, on);
}

void CommonTaskTool::slotMeasureNote(bool on) {
	if (on && status_.curr_tool != common::CommonToolTypeOnOff::M_NOTE)
		ReleaseOtherButtons(measure_[MeasureID::NOTE].get());

	SetTaskToolOnOff(common::CommonToolTypeOnOff::M_NOTE, on);
}

void CommonTaskTool::slotMeasureImplantAngle() {
	emit sigShowImplantAngle();
}

void CommonTaskTool::slotMeasureDelete(bool on) {
	if (on && status_.curr_tool != common::CommonToolTypeOnOff::M_DEL)
		ReleaseOtherButtons(measure_[MeasureID::DEL].get());

	SetTaskToolOnOff(common::CommonToolTypeOnOff::M_DEL, on);
}

void CommonTaskTool::slotActTapeLine() {
	ReleaseOtherButtons(measure_[MeasureID::TAPE].get());
	measure_[MeasureID::TAPE]->setChecked(true);
	SetTapeButton(ActionTypeTape::LINE);
	SetTaskToolOnOff(common::CommonToolTypeOnOff::M_TAPELINE, true);
}

void CommonTaskTool::slotActTapeCurve() {
	ReleaseOtherButtons(measure_[MeasureID::TAPE].get());
	measure_[MeasureID::TAPE]->setChecked(true);
	SetTapeButton(ActionTypeTape::CURVE);
	SetTaskToolOnOff(common::CommonToolTypeOnOff::M_TAPECURVE, true);
}

void CommonTaskTool::slotActAnalysisProfile()
{
	ReleaseOtherButtons(measure_[MeasureID::ANALYSIS].get());
	measure_[MeasureID::ANALYSIS]->setChecked(true);
	SetAnalysisButton(ActionTypeAnalysis::PROFILE);
	SetTaskToolOnOff(common::CommonToolTypeOnOff::M_PROFILE, true);
}

void CommonTaskTool::slotActAnalysisROI()
{
	ReleaseOtherButtons(measure_[MeasureID::ANALYSIS].get());
	measure_[MeasureID::ANALYSIS]->setChecked(true);
	SetAnalysisButton(ActionTypeAnalysis::ROI);
	SetTaskToolOnOff(common::CommonToolTypeOnOff::M_ROI, true);
}

void CommonTaskTool::slotActDrawArrow() {
	ReleaseOtherButtons(measure_[MeasureID::DRAW].get());
	measure_[MeasureID::DRAW]->setChecked(true);
	SetAnnoButton(ActionTypeAnno::ARROW);
	SetTaskToolOnOff(common::CommonToolTypeOnOff::M_ARROW, true);
}

void CommonTaskTool::slotActDrawRect() {
	ReleaseOtherButtons(measure_[MeasureID::DRAW].get());
	measure_[MeasureID::DRAW]->setChecked(true);
	SetAnnoButton(ActionTypeAnno::RECT);
	SetTaskToolOnOff(common::CommonToolTypeOnOff::M_RECTANGLE, true);
}

void CommonTaskTool::slotActDrawCircle() {
	ReleaseOtherButtons(measure_[MeasureID::DRAW].get());
	measure_[MeasureID::DRAW]->setChecked(true);
	SetAnnoButton(ActionTypeAnno::CIRCLE);
	SetTaskToolOnOff(common::CommonToolTypeOnOff::M_CIRCLE, true);
}

void CommonTaskTool::slotActDrawLine()
{
	ReleaseOtherButtons(measure_[MeasureID::DRAW].get());
	measure_[MeasureID::DRAW]->setChecked(true);
	SetAnnoButton(ActionTypeAnno::LINE);
	SetTaskToolOnOff(common::CommonToolTypeOnOff::M_LINE, true);
}

void CommonTaskTool::slotActDrawFreedraw() {
	ReleaseOtherButtons(measure_[MeasureID::DRAW].get());
	measure_[MeasureID::DRAW]->setChecked(true);
	SetAnnoButton(ActionTypeAnno::FREEDRAW);
	SetTaskToolOnOff(common::CommonToolTypeOnOff::M_FREEDRAW, true);
}

void CommonTaskTool::slotActDelOne() {
	ReleaseOtherButtons(measure_[MeasureID::DEL].get());
	measure_[MeasureID::DEL]->setChecked(true);
	SetTaskToolOnOff(common::CommonToolTypeOnOff::M_DEL, true);
}

void CommonTaskTool::slotActDelAll() {
	ReleaseOtherButtons();
	emit sigCommonToolOnce(common::CommonToolTypeOnce::M_DEL_ALL, true);
}

void CommonTaskTool::SetImplantAngleVisible(const bool visible)
{
#if !defined(WILL3D_VIEWER) && !defined(WILL3D_EUROPE)
	measure_[MeasureID::IMPLANT_ANGLE].get()->setVisible(visible);
#endif
}

QToolButton* CommonTaskTool::GetFileButton(FileID id)
{
	if (!file_[id])
	{
		return nullptr;
	}

	return file_[id].get();
}

QToolButton* CommonTaskTool::GetViewButton(ViewID id)
{
	if (!view_[id])
	{
		return nullptr;
	}

	return view_[id].get();
}

QToolButton* CommonTaskTool::GetMeasureButton(MeasureID id)
{
	if (!measure_[id])
	{
		return nullptr;
	}

	return measure_[id].get();
}
