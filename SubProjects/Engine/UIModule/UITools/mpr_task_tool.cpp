#include "mpr_task_tool.h"

#include <QAction>
#include <QApplication>
#include <QBoxLayout>
#include <QMenu>
#include <QToolButton>
#include <QRadioButton>

#include <Engine/Common/Common/global_preferences.h>

#include <Engine/Common/Common/W3Theme.h>
#include <Engine/Common/Common/W3Define.h>
#include <Engine/Common/Common/language_pack.h>

#include "clipping_tool.h"
#include "tool_box.h"
#include "visibility_tool_box.h"

MPRTaskTool::MPRTaskTool(QObject* parent) : BaseTool(parent)
{
	QSettings settings(GlobalPreferences::GetInstance()->ini_path(), QSettings::IniFormat);
#ifndef WILL3D_LIGHT
	use_auto_orientation_ = settings.value("MPR/auto_orientation", true).toBool();
#else
	use_auto_orientation_ = false;
#endif

	use_draw_arch_ = settings.value("MPR/draw_arch", false).toBool();

	CreateUI();
	SetToolTips();
	//Connections();
}

MPRTaskTool::~MPRTaskTool() {}

void MPRTaskTool::InitExternUIs(const OrientationTool::OrientUI& orient_ui)
{
	if (!orientation_tool_)
	{
		return;
	}

	orientation_tool_->InitExternUIs(orient_ui);
}

void MPRTaskTool::ResetUI()
{
	lightbox_on_ = false;

	task_[MPRTaskID::ZOOM3D]->setChecked(false);
	task_[MPRTaskID::OBLIQUE]->setChecked(false);
#ifndef WILL3D_LIGHT
	task_[MPRTaskID::CUT3D]->setChecked(false);
#endif

	if (use_draw_arch_)
	{
		arch_type_[ArchTypeID::ARCH_MANDLBLE]->setChecked(true);
	}

	clip_tool_->ResetUI();
	visibility_tool_->ResetUI();
}

void MPRTaskTool::SetLightboxOn(bool on) { lightbox_on_ = on; }

ClipStatus MPRTaskTool::GetClipStatus()
{
	ClipStatus status;
	switch (clip_tool_->GetClipPlaneID())
	{
	case ClipID::AXIAL:
		status.plane = MPRClipID::AXIAL;
		break;
	case ClipID::CORONAL:
		status.plane = MPRClipID::CORONAL;
		break;
	case ClipID::SAGITTAL:
		status.plane = MPRClipID::SAGITTAL;
		break;
	case ClipID::MPROVERLAY:
		status.plane = MPRClipID::MPROVERLAY;
		break;
	}
	status.enable = clip_tool_->IsClipEnable();
	status.flip = clip_tool_->IsFlip();
	status.lower = clip_tool_->GetLowerValue();
	status.upper = clip_tool_->GetUpperValue();

	return status;
}

void MPRTaskTool::SetAirwaySize(float size)
{
	visibility_tool_->SetAirwaySize(size);
}

void MPRTaskTool::SetAirwayEnable(bool is_enable)
{
	visibility_tool_->EnableAirwayUI(is_enable);
}

bool MPRTaskTool::IsZoom3DOn() const
{
	return task_[MPRTaskID::ZOOM3D]->isChecked();
}

bool MPRTaskTool::IsClipOn() const { return clip_tool_->IsClipEnable(); }

void MPRTaskTool::SyncTaskUI(const MPRTaskID& task_id, bool state)
{
	task_[task_id]->setChecked(state);
}

void MPRTaskTool::SyncVisibilityResources()
{
	visibility_tool_->SyncVisibilityResources();
}

void MPRTaskTool::SyncVisibilityEnable(const VisibleID& visible_id,
	const bool& enable)
{
	visibility_tool_->SyncVisibilityEnable(visible_id, enable);
}

void MPRTaskTool::SyncVisibilityChecked(const VisibleID& visible_id, const bool& checked)
{
	visibility_tool_->SyncVisibilityChecked(visible_id, checked);
}

QWidget* MPRTaskTool::GetOrientationWidget()
{
	if (use_auto_orientation_)
	{
		return orient_tool_box_.get();
	}
	else
	{
		return orientation_tool_->GetWidget();
	}
}

QWidget* MPRTaskTool::GetTaskWidget()
{
	return task_tool_box_.get();
}

QWidget* MPRTaskTool::GetClipWidget()
{
	return clip_tool_->GetWidget();
}

QWidget* MPRTaskTool::GetVisibilityWidget()
{
	return visibility_tool_->GetWidget();
}

void MPRTaskTool::CreateUI()
{
	CW3Theme* theme = CW3Theme::getInstance();
	QMargins contentsMargin = theme->getToolVBarSizeInfo().marginContents;
	QMargins boxMargins = theme->getToolVBarSizeInfo().marginBox;
	int spacingM = theme->getToolVBarSizeInfo().spacingM;
	int spacing_s = theme->getToolVBarSizeInfo().spacingS;
	int spacing_l = theme->getToolVBarSizeInfo().spacingL;

	if (use_auto_orientation_)
	{
		// create reorientation tool buttons
		QHBoxLayout* orient_layout = new QHBoxLayout();
		orient_layout->setSpacing(0);
		orient_layout->setContentsMargins(contentsMargin);
		orient_layout->setSpacing(spacingM);

		QString orient_tool_stylesheet = theme->toolOrientationStyleSheet();
		for (int id = 0; id < ReorientID::REORIENT_END; ++id)
		{
			reorient_[id].reset(new QToolButton());
			reorient_[id]->setContentsMargins(ui_tools::kMarginZero);
			reorient_[id]->setStyleSheet(orient_tool_stylesheet);
			orient_layout->addWidget(reorient_[id].get());
			orient_layout->setStretch(id, 1);
		}

		orient_tool_box_.reset(new ToolBox());
		orient_tool_box_->setCaptionName(lang::LanguagePack::txt_orientation(),
			Qt::AlignLeft);
		orient_tool_box_->addToolLayout(orient_layout);
		orient_tool_box_->setContentsMargins(boxMargins);
	}
	else
	{
		orientation_tool_.reset(new OrientationTool());
	}

	// create task tool buttons
	QString task_stylesheet = theme->toolTaskStyleSheet();
	for (int id = 0; id < MPRTaskID::MPR_TASK_END; ++id)
	{
		if (!use_draw_arch_ && id == MPRTaskID::DRAW_ARCH)
		{
			continue;
		}
		task_[id].reset(new QToolButton());
		task_[id]->setContentsMargins(ui_tools::kMarginZero);
		task_[id]->setStyleSheet(task_stylesheet);
		task_[id]->setStyle(theme->toolTaskStyle());
	}
	
#ifndef WILL3D_LIGHT
	cut_menu_.reset(new QToolButton());
	cut_menu_->setPopupMode(QToolButton::MenuButtonPopup);
	cut_menu_->setObjectName("Menu");

	QMenu* menu = new QMenu(cut_menu_.get());
	menu->setFont(QApplication::font());
	cut_menu_->setObjectName("3DCutToolSelect");
	cut_actions_[CutType::POLYGON] = menu->addAction(lang::LanguagePack::txt_polygon());
	cut_actions_[CutType::FREEDRAW] = menu->addAction(lang::LanguagePack::txt_free_draw());

	cut_menu_->setMenu(menu);
	cut_menu_->setStyleSheet(theme->toolIconButtonStyleSheet());
	menu->setStyle(theme->toolMenuIconStyle());
	menu->setStyleSheet(
		QString("QMenu { border: 1px; background: #FF1C1E28; }"
			"QMenu::item { background-color: transparent; }"
			"QMenu::item::selected { background-color: #FF434961; }"));

	QHBoxLayout* cut_layout = new QHBoxLayout();
	cut_layout->setSpacing(0);
	cut_layout->setContentsMargins(ui_tools::kMarginZero);
	cut_layout->addWidget(task_[MPRTaskID::CUT3D].get());
	cut_layout->setStretch(0, 9);
	cut_layout->addWidget(cut_menu_.get());
	cut_layout->setStretch(1, 1);
#endif

	QVBoxLayout* task_layout = new QVBoxLayout();
	task_layout->setContentsMargins(
		contentsMargin.left() * 0.5, contentsMargin.top(),
		contentsMargin.right() * 0.5, contentsMargin.bottom()
	);

	task_layout->addWidget(task_[MPRTaskID::ZOOM3D].get(), 0, Qt::AlignLeft);
#ifndef WILL3D_LIGHT
	task_layout->addLayout(cut_layout);
#endif
	task_layout->addWidget(task_[MPRTaskID::STLEXPORT].get(), 0, Qt::AlignLeft);
	task_layout->addWidget(task_[MPRTaskID::OBLIQUE].get(), 0, Qt::AlignLeft);

	if (use_draw_arch_)
	{
		QHBoxLayout* arch_type_layout = new QHBoxLayout();
		arch_type_layout->setContentsMargins(QMargins(spacing_s, spacing_l, spacing_s, spacing_l));
		arch_type_layout->setSpacing(20);
		for (int id = 0; id < ArchTypeID::ARCH_TYPE_END; ++id)
		{
			arch_type_[id].reset(new QRadioButton());
			arch_type_layout->addWidget(arch_type_[id].get());
		}
		task_layout->addLayout(arch_type_layout);
		task_layout->addWidget(task_[MPRTaskID::DRAW_ARCH].get(), 0, Qt::AlignLeft);
	}

	task_tool_box_.reset(new ToolBox());
	task_tool_box_->setCaptionName(lang::LanguagePack::txt_task(), Qt::AlignLeft);
	task_tool_box_->addToolLayout(task_layout);
	task_tool_box_->setContentsMargins(boxMargins);

	// create clipping tool ui
	std::vector<QString> mode_names = {
		"Axial", "Coronal", "Sagittal",
		lang::LanguagePack::txt_mpr_overlay().split(":").at(0) };
	clip_tool_.reset(
		new ClippingTool(mode_names, ClippingTool::DirectionType::GRID));

	// create visibility tool ui
	visibility_tool_.reset(new VisibilityToolBox(true, true, true, true, true));
	visibility_tool_->SetAdjustImplantButtonVisible(true);

	// status setting
	if (use_auto_orientation_)
	{
		reorient_[ReorientID::REORIENT]->setObjectName("AutoOrientation");
		reorient_[ReorientID::RESET]->setObjectName("ResetOrientation");
	}
	task_[MPRTaskID::ZOOM3D]->setObjectName("3Dzoom");
	task_[MPRTaskID::OBLIQUE]->setObjectName("Oblique");
#ifndef WILL3D_LIGHT
	task_[MPRTaskID::CUT3D]->setObjectName("3DCut");
#endif
	task_[MPRTaskID::STLEXPORT]->setObjectName("STLExport");

	if (use_auto_orientation_)
	{
		reorient_[ReorientID::REORIENT]->setText(lang::LanguagePack::txt_auto());
		reorient_[ReorientID::RESET]->setText(lang::LanguagePack::txt_reset());
	}
	task_[MPRTaskID::ZOOM3D]->setText(lang::LanguagePack::txt_3d_zoom());
	task_[MPRTaskID::OBLIQUE]->setText(lang::LanguagePack::txt_oblique());
#ifndef WILL3D_LIGHT
	task_[MPRTaskID::CUT3D]->setText(lang::LanguagePack::txt_3d_cut() + "\n " + lang::LanguagePack::txt_polygon());
#endif
	task_[MPRTaskID::STLEXPORT]->setText(lang::LanguagePack::txt_stl_export());
	if (use_draw_arch_)
	{
		task_[MPRTaskID::DRAW_ARCH]->setObjectName("DrawArch");
		task_[MPRTaskID::DRAW_ARCH]->setText(lang::LanguagePack::txt_draw_arch());
		arch_type_[ArchTypeID::ARCH_MANDLBLE]->setText(lang::LanguagePack::txt_mandible());
		arch_type_[ArchTypeID::ARCH_MAXILLA]->setText(lang::LanguagePack::txt_maxilla());

		arch_type_[ArchTypeID::ARCH_MANDLBLE]->setChecked(true);
	}
	task_[MPRTaskID::ZOOM3D]->setCheckable(true);
	task_[MPRTaskID::OBLIQUE]->setCheckable(true);
#ifndef WILL3D_LIGHT
	task_[MPRTaskID::CUT3D]->setCheckable(true);
#endif

#ifdef WILL3D_EUROPE
	task_[MPRTaskID::STLEXPORT]->setVisible(false);
#endif // WILL3D_EUROPE

}

void MPRTaskTool::Connections()
{
	if (use_auto_orientation_)
	{
		connect(reorient_[ReorientID::REORIENT].get(), &QToolButton::clicked, this, &MPRTaskTool::sigMPRAutoReorient);
		connect(reorient_[ReorientID::RESET].get(), &QToolButton::clicked, this, &MPRTaskTool::sigMPRReorientReset);
	}
	else
	{
		orientation_tool_->Connections();
		connect(orientation_tool_.get(), &OrientationTool::sigReorient, this, &MPRTaskTool::sigAdjustOrientationClicked);
		connect(orientation_tool_.get(), &OrientationTool::sigReorientReset, this, &MPRTaskTool::sigMPRReorientReset);
		connect(orientation_tool_.get(), &OrientationTool::sigOrientRotate, this, &MPRTaskTool::sigRotateOrientation);
	}

	connect(task_[MPRTaskID::ZOOM3D].get(), &QToolButton::toggled, this, &MPRTaskTool::slotTaskZoom3D);
	connect(task_[MPRTaskID::OBLIQUE].get(), &QToolButton::toggled, this, &MPRTaskTool::slotTaskOblique);
	connect(task_[MPRTaskID::STLEXPORT].get(), &QToolButton::clicked, this, &MPRTaskTool::slotTaskSTLExport);
	if (task_[MPRTaskID::DRAW_ARCH].get())
	{
		connect(task_[MPRTaskID::DRAW_ARCH].get(), &QToolButton::clicked, this, &MPRTaskTool::slotTaskDrawArch);
	}

#ifndef WILL3D_LIGHT
	connect(task_[MPRTaskID::CUT3D].get(), &QToolButton::toggled, this, &MPRTaskTool::slotTaskCut3D);

	connect(cut_actions_[CutType::POLYGON], &QAction::triggered, this, &MPRTaskTool::slotTaskCut3DPolygon);
	connect(cut_actions_[CutType::FREEDRAW], &QAction::triggered, this, &MPRTaskTool::slotTaskCut3DFreedraw);
#endif

	connect(visibility_tool_.get(), &VisibilityToolBox::sigVisible, this, &MPRTaskTool::sigMPRVisible);
	connect(visibility_tool_.get(), &VisibilityToolBox::sigChangeFaceTransparency, this, &MPRTaskTool::sigMPRChangeFaceTransparency);
	connect(visibility_tool_.get(), &VisibilityToolBox::sigAdjustImplantButtonToggled, this, &MPRTaskTool::sigAdjustImplantButtonToggled);

	connect(clip_tool_.get(), &ClippingTool::sigEnable, this, &MPRTaskTool::sigMPRClipEnable);
	connect(clip_tool_.get(), &ClippingTool::sigRangeMove, this, &MPRTaskTool::sigMPRClipRangeMove);
	connect(clip_tool_.get(), &ClippingTool::sigRangeSet, this, &MPRTaskTool::sigMPRClipRangeSet);
	connect(clip_tool_.get(), &ClippingTool::sigPlaneChanged, this, &MPRTaskTool::slotClipPlaneChanged);
	connect(clip_tool_.get(), &ClippingTool::sigFlip, this, &MPRTaskTool::sigFlipClipping);
}

void MPRTaskTool::SetToolTips() {}

void MPRTaskTool::ResetOtherTaskButtons(const MPRTaskID& selected_id)
{
	for (int id = 0; id < MPRTaskID::MPR_TASK_END; ++id)
	{
		if (id == selected_id ||
			!task_[id])
		{
			continue;
		}

		task_[id]->setChecked(false);
	}
}

void MPRTaskTool::SetCheckCut3DButton()
{
#ifndef WILL3D_LIGHT
	task_[MPRTaskID::CUT3D]->blockSignals(true);
	task_[MPRTaskID::CUT3D]->setChecked(true);
	task_[MPRTaskID::CUT3D]->blockSignals(false);
#endif
}

void MPRTaskTool::slotTaskZoom3D(bool toggle)
{
	if (toggle) ResetOtherTaskButtons(MPRTaskID::ZOOM3D);
	emit sigMPRTask(MPRTaskID::ZOOM3D, toggle);
}

void MPRTaskTool::slotTaskCut3D(bool toggle)
{
#ifndef WILL3D_LIGHT
	if (toggle) ResetOtherTaskButtons(MPRTaskID::CUT3D);
	emit sigMPRTask(MPRTaskID::CUT3D, toggle);
#endif
}

void MPRTaskTool::slotTaskCut3DPolygon()
{
#ifndef WILL3D_LIGHT
	task_[MPRTaskID::CUT3D]->setText(lang::LanguagePack::txt_3d_cut() + "\n " + lang::LanguagePack::txt_polygon());
	cut_tool_ = VRCutTool::POLYGON;
	SetCheckCut3DButton();
	slotTaskCut3D(true);
#endif
}

void MPRTaskTool::slotTaskCut3DFreedraw()
{
#ifndef WILL3D_LIGHT
	task_[MPRTaskID::CUT3D]->setText(lang::LanguagePack::txt_3d_cut() + "\n " + lang::LanguagePack::txt_free_draw());
	cut_tool_ = VRCutTool::FREEDRAW;
	SetCheckCut3DButton();
	slotTaskCut3D(true);
#endif
}

void MPRTaskTool::slotTaskOblique(bool toggle)
{
	if (toggle) ResetOtherTaskButtons(MPRTaskID::OBLIQUE);
	clip_tool_->SetEnable(!toggle);
	emit sigMPRTask(MPRTaskID::OBLIQUE, toggle);
}

void MPRTaskTool::slotTaskSTLExport()
{
	ResetOtherTaskButtons(MPRTaskID::STLEXPORT);
	emit sigMPRTask(MPRTaskID::STLEXPORT, true);
}

void MPRTaskTool::slotTaskDrawArch()
{
	ResetOtherTaskButtons(MPRTaskID::DRAW_ARCH);
	emit sigMPRTask(MPRTaskID::DRAW_ARCH, true);
}

void MPRTaskTool::slotClipPlaneChanged(int id)
{
	MPRClipID plane_type;
	switch (id)
	{
	case ClipID::AXIAL:
		plane_type = MPRClipID::AXIAL;
		break;
	case ClipID::CORONAL:
		plane_type = MPRClipID::CORONAL;
		break;
	case ClipID::SAGITTAL:
		plane_type = MPRClipID::SAGITTAL;
		break;
	case ClipID::MPROVERLAY:
		plane_type = MPRClipID::MPROVERLAY;
		break;
	default:
		break;
	}

	emit sigMPRClipPlaneChanged(plane_type);
}

void MPRTaskTool::ResetOrientationDegreesUI()
{
	if (!orientation_tool_)
	{
		return;
	}

	orientation_tool_->ResetOrientDegreesUI();
}

int MPRTaskTool::GetOrientationDegree(const ReorientViewID& view_type) const
{
	if (!orientation_tool_)
	{
		return 0;
	}

	return orientation_tool_->GetOrientDegree(view_type);
}

void MPRTaskTool::SetOrientationDegrees(const int& degree_a, const int& degree_r, const int& degree_i)
{
	if (!orientation_tool_)
	{
		return;
	}

	orientation_tool_->SetOrientDegrees(degree_a, degree_r, degree_i);
}

void MPRTaskTool::SyncOrientationDegreeUIOnly(const ReorientViewID& view_type, const int& degree_view)
{
	if (!orientation_tool_)
	{
		return;
	}

	orientation_tool_->SyncOrientDegreeUIOnly(view_type, degree_view);
}

const ArchTypeID MPRTaskTool::GetArchType()
{
	return arch_type_[ArchTypeID::ARCH_MANDLBLE]->isChecked() ? ArchTypeID::ARCH_MANDLBLE : ArchTypeID::ARCH_MAXILLA;
}
