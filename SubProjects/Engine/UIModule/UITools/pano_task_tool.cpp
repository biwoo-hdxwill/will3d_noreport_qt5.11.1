#include "pano_task_tool.h"
#include <QBoxLayout>
#include <QToolButton>
#include <QRadioButton>
#include <QMenu>
#include <QApplication>

#include <Engine/Common/Common/W3Theme.h>
#include <Engine/Common/Common/language_pack.h>

#include "tool_box.h"
#include "nerve_tool_box.h"

PanoTaskTool::PanoTaskTool(QObject *parent)
	: BasePanoTaskTool(parent)
{
	CreateUI();
	SetToolTips();
}

PanoTaskTool::~PanoTaskTool()
{
}

void PanoTaskTool::InitExternUIs(const OrientationTool::OrientUI& orient_ui,
	const CrossSectionUI& cs_ui,
	const PanoUI& pano_ui)
{
	orient_tool_->InitExternUIs(orient_ui);
	BasePanoTaskTool::InitExternUIs(cs_ui, pano_ui);
}

void PanoTaskTool::SetPanoArchTypeRadioUI(const ArchTypeID & arch_type)
{
	arch_type_[ArchTypeID::ARCH_MANDLBLE]->blockSignals(true);
	arch_type_[ArchTypeID::ARCH_MAXILLA]->blockSignals(true);

	ArchTypeID radio_arch_type;
	if (arch_type_[ArchTypeID::ARCH_MANDLBLE]->isChecked())
		radio_arch_type = ArchTypeID::ARCH_MANDLBLE;
	else if (arch_type_[ArchTypeID::ARCH_MAXILLA]->isChecked())
		radio_arch_type = ArchTypeID::ARCH_MAXILLA;

	if (radio_arch_type != arch_type)
		arch_type_[arch_type]->setChecked(true);

	arch_type_[ArchTypeID::ARCH_MANDLBLE]->blockSignals(false);
	arch_type_[ArchTypeID::ARCH_MAXILLA]->blockSignals(false);
}
//20250123 LIN importProject viewer에 적용
//#ifndef WILL3D_VIEWER
void PanoTaskTool::ImportProject(ProjectIOPanoEngine & in)
{
	nerve_tool_->importProject(in);
}
//#endif
void PanoTaskTool::ResetUI()
{
	BasePanoTaskTool::ResetUI();

	task_[PanoTaskID::MANUAL]->setChecked(false);
	arch_type_[ArchTypeID::ARCH_MANDLBLE]->setChecked(true);
}

void PanoTaskTool::Connections()
{
	BasePanoTaskTool::Connections();

	orient_tool_->Connections();
	connect(orient_tool_.get(), &OrientationTool::sigReorient, this, &PanoTaskTool::sigPanoReorient);
	connect(orient_tool_.get(), &OrientationTool::sigReorientReset, this, &PanoTaskTool::sigPanoReorientReset);
	connect(orient_tool_.get(), &OrientationTool::sigOrientRotate, this, &PanoTaskTool::sigPanoOrientRotate);

	connect(task_[PanoTaskID::AUTO].get(), SIGNAL(clicked()), this, SLOT(slotTaskAutoArch()));
	connect(task_[PanoTaskID::MANUAL].get(), SIGNAL(clicked()), this, SLOT(slotTaskManualArch()));

	connect(auto_arch_actions_[static_cast<int>(AutoArchMenu::RESET)], &QAction::triggered, this, &PanoTaskTool::sigTaskResetAutoArchSliceNumber);

	connect(nerve_tool_.get(), &NerveToolBox::sigChangedNerveValues, this, &PanoTaskTool::sigPanoNerveParamsChanged);
	connect(nerve_tool_.get(), &NerveToolBox::sigHoveredNerveRecord, this, &PanoTaskTool::sigPanoNerveRecordHovered);
	connect(nerve_tool_.get(), &NerveToolBox::sigToggledDraw, this, &PanoTaskTool::sigPanoNerveToggledDraw);
	connect(nerve_tool_.get(), &NerveToolBox::sigDeleteNerve, this, &PanoTaskTool::sigPanoNerveDelete);

	connect(arch_type_[ArchTypeID::ARCH_MANDLBLE].get(), SIGNAL(toggled(bool)), this, SLOT(slotArchTypeChanged()));
	connect(arch_type_[ArchTypeID::ARCH_MAXILLA].get(), SIGNAL(toggled(bool)), this, SLOT(slotArchTypeChanged()));
}

bool PanoTaskTool::IsTaskManualOn() const
{
	return task_[PanoTaskID::MANUAL]->isChecked();
}

void PanoTaskTool::TaskManualDeactivate()
{
	if (IsTaskManualOn())
		task_[PanoTaskID::MANUAL]->setChecked(false);
}

void PanoTaskTool::ResetOrientDegreesUI()
{
	orient_tool_->ResetOrientDegreesUI();
}

int PanoTaskTool::GetOrientDegree(const ReorientViewID & view_type) const
{
	return orient_tool_->GetOrientDegree(view_type);
}

void PanoTaskTool::SetOrientDegrees(const int & degree_a, const int & degree_r,
	const int & degree_i)
{
	orient_tool_->SetOrientDegrees(degree_a, degree_r, degree_i);
}

void PanoTaskTool::SyncOrientDegreeUIOnly(const ReorientViewID & view_type, const int& degree_view)
{
	orient_tool_->SyncOrientDegreeUIOnly(view_type, degree_view);
}

void PanoTaskTool::NerveCreated(int nerve_id)
{
	nerve_tool_->CreateRecord(nerve_id);
}

void PanoTaskTool::NerveDeleted(int nerve_id)
{
	nerve_tool_->DeleteRecord(nerve_id);
}

void PanoTaskTool::NerveToolDeactivate()
{
	nerve_tool_->ReleaseDrawButton();
}

void PanoTaskTool::NerveToolVisibilityChange(bool visible)
{
	nerve_tool_->ChangeVisibleCheck(visible);
}

QWidget* PanoTaskTool::GetOrientationWidget() { return orient_tool_->GetWidget(); }
QWidget* PanoTaskTool::GetTaskWidget() { return task_tool_box_.get(); }
QWidget* PanoTaskTool::GetNerveToolWidget() { return nerve_tool_box_.get(); }

void PanoTaskTool::CreateUI()
{
	CW3Theme* theme = CW3Theme::getInstance();
	QMargins contentsMargin = theme->getToolVBarSizeInfo().marginContents;
	QMargins boxMargins = theme->getToolVBarSizeInfo().marginBox;
	int spacingM = theme->getToolVBarSizeInfo().spacingM;
	float spacingS = theme->getToolVBarSizeInfo().spacingS;
	float spacingL = theme->getToolVBarSizeInfo().spacingL;

	// create reorientation tool buttons
	orient_tool_.reset(new OrientationTool());

	// create task tool buttons
	QVBoxLayout* task_layout = new QVBoxLayout();
	task_layout->setContentsMargins(contentsMargin.left() * 0.5, contentsMargin.top(),
		contentsMargin.right() * 0.5, contentsMargin.bottom());

	QHBoxLayout* arch_type_layout = new QHBoxLayout();
	arch_type_layout->setContentsMargins(QMargins(spacingS, 0, spacingS, spacingL));
	arch_type_layout->setSpacing(20);
	for (int id = 0; id < ArchTypeID::ARCH_TYPE_END; ++id)
	{
		arch_type_[id].reset(new QRadioButton());
		arch_type_layout->addWidget(arch_type_[id].get());
	}
	task_layout->addLayout(arch_type_layout);

	QString task_stylesheet = theme->toolTaskStyleSheet();
	for (int id = 0; id < PanoTaskID::PANO_TASK_END; ++id)
	{
		task_[id].reset(new QToolButton());
		task_[id]->setContentsMargins(ui_tools::kMarginZero);
		task_[id]->setStyleSheet(task_stylesheet);
		task_[id]->setStyle(theme->toolTaskStyle());
		//task_layout->addWidget(task_[id].get());
	}
#if 1
	auto_arch_menu_.reset(new QToolButton());
	auto_arch_menu_->setPopupMode(QToolButton::MenuButtonPopup);
	auto_arch_menu_->setObjectName("Menu");

	QMenu* menu = new QMenu(auto_arch_menu_.get());
	menu->setFont(QApplication::font());
	auto_arch_menu_->setObjectName("AutoArchMenu");
	auto_arch_actions_[static_cast<int>(AutoArchMenu::RESET)] = menu->addAction(lang::LanguagePack::txt_reset());

	auto_arch_menu_->setMenu(menu);
	auto_arch_menu_->setStyleSheet(theme->toolIconButtonStyleSheet());
	menu->setStyle(theme->toolMenuIconStyle());
	menu->setStyleSheet(
		QString(
			"QMenu { border: 1px; background: #FF1C1E28; }"
			"QMenu::item { background-color: transparent; }"
			"QMenu::item::selected { background-color: #FF434961; }"
		)
	);

	QHBoxLayout* auto_arch_layout = new QHBoxLayout();
	auto_arch_layout->setSpacing(0);
	auto_arch_layout->setContentsMargins(ui_tools::kMarginZero);
	auto_arch_layout->addWidget(task_[PanoTaskID::AUTO].get());
	auto_arch_layout->setStretch(0, 9);
	auto_arch_layout->addWidget(auto_arch_menu_.get());
	auto_arch_layout->setStretch(1, 1);

	task_layout->addLayout(auto_arch_layout);
	task_layout->addWidget(task_[PanoTaskID::MANUAL].get(), 0, Qt::AlignLeft);
#endif

	task_tool_box_.reset(new ToolBox());
	task_tool_box_->setCaptionName(lang::LanguagePack::txt_task(), Qt::AlignLeft);
	task_tool_box_->addToolLayout(task_layout);
	task_tool_box_->setContentsMargins(boxMargins);

	// create nerve tool box ui
	nerve_tool_.reset(new NerveToolBox());
	nerve_tool_box_.reset(new ToolBox());
	nerve_tool_box_->setCaptionName(lang::LanguagePack::txt_nerve(), Qt::AlignLeft);
	nerve_tool_box_->addToolWidget(nerve_tool_.get());
	nerve_tool_box_->setContentsMargins(boxMargins);

	task_[PanoTaskID::AUTO]->setObjectName("AutoArch");
	task_[PanoTaskID::MANUAL]->setObjectName("ManualArch");
	task_[PanoTaskID::AUTO]->setText(lang::LanguagePack::txt_auto_arch());
	task_[PanoTaskID::MANUAL]->setText(lang::LanguagePack::txt_manual_arch());
	arch_type_[ArchTypeID::ARCH_MANDLBLE]->setText(lang::LanguagePack::txt_mandible());
	arch_type_[ArchTypeID::ARCH_MAXILLA]->setText(lang::LanguagePack::txt_maxilla());

	task_[PanoTaskID::MANUAL]->setCheckable(true);
}

void PanoTaskTool::SetToolTips()
{
}

void PanoTaskTool::slotArchTypeChanged()
{
	ArchTypeID arch_type;
	if (arch_type_[ArchTypeID::ARCH_MAXILLA]->isChecked())
	{
		arch_type = ArchTypeID::ARCH_MAXILLA;
	}
	else if (arch_type_[ArchTypeID::ARCH_MANDLBLE]->isChecked())
	{
		arch_type = ArchTypeID::ARCH_MANDLBLE;
	}

	emit sigPanoArchTypeChanged(arch_type);
}

void PanoTaskTool::slotTaskAutoArch()
{
	if (nerve_tool_->IsPressDrawButton())
		nerve_tool_->PressDrawButton(false);

	emit sigPanoTask(PanoTaskID::AUTO);
}

void PanoTaskTool::slotTaskManualArch()
{
	emit sigPanoTask(PanoTaskID::MANUAL);
}
