#include "endo_task_tool.h"

#include <QBoxLayout>
#include <QCheckBox>
#include <QSlider>
#include <QLabel>
#include <QRadioButton>
#include <QToolButton>

#include <Engine/Common/Common/W3Logger.h>
#include <Engine/Common/Common/W3Theme.h>
#include <Engine/Common/Common/language_pack.h>

#include "tool_box.h"
#include "visibility_tool_box.h"

EndoTaskTool::EndoTaskTool(QObject *parent)
	: BaseTool(parent) {
	CreateUI();
	ResetUI();
	Connections();
}

EndoTaskTool::~EndoTaskTool() {
}

void EndoTaskTool::ResetUI() {
	free_explorer_->setChecked(false);
	direction_[EndoCameraDir::FORWARD]->setChecked(true);

	player_param_[EndoPlayerParamID::PLAY_INTERVAL]->setSliderPosition(4);
	player_param_[EndoPlayerParamID::PLAY_INTERVAL]->setValue(4);
	player_param_[EndoPlayerParamID::PLAY_SPEED]->setSliderPosition(5);
	player_param_[EndoPlayerParamID::PLAY_SPEED]->setValue(5);

	path_[EndoPathID::PATH1]->setChecked(true);
	for (int id = 0; id < EndoPathID::ENDO_PATH_END; ++id)
		remove_path_[id]->setEnabled(false);

	show_path_->setEnabled(false);
	show_path_->setCheckState(Qt::CheckState::Checked);

	visibility_tool_->ResetUI();
}

void EndoTaskTool::SyncSetPath(const EndoPathID& path_id, bool checked) {
	path_[path_id]->setChecked(checked);
}

void EndoTaskTool::SetAirwaySize(float size) {
	visibility_tool_->SetAirwaySize(size);
}

void EndoTaskTool::SetAirwayEnable(bool is_enable) {
	visibility_tool_->EnableAirwayUI(is_enable);
	show_path_->setEnabled(is_enable);
	show_path_->setChecked(is_enable);
}

QWidget* EndoTaskTool::GetPlayerWidget() { return player_tool_box_.get(); }
QWidget* EndoTaskTool::GetPathWidget() { return path_tool_box_.get(); }
QWidget* EndoTaskTool::GetVisibilityWidget() { return visibility_tool_->GetWidget(); }

void EndoTaskTool::slotEnableRemovePath(const int path_num, bool enable) {
	EndoPathID path_id = static_cast<EndoPathID>(path_num);
	remove_path_[path_id]->setEnabled(enable);
}

void EndoTaskTool::slotToggleFreeExplorer(bool state) {
	if (free_explorer_)
		free_explorer_->setChecked(state);
}

void EndoTaskTool::CreateUI() {
	CW3Theme* theme = CW3Theme::getInstance();
	QMargins contentsMargin = theme->getToolVBarSizeInfo().marginContents;
	QMargins boxMargins = theme->getToolVBarSizeInfo().marginBox;
	int spacingM = theme->getToolVBarSizeInfo().spacingM;
	int spacingL = theme->getToolVBarSizeInfo().spacingL;
	int spacingS = theme->getToolVBarSizeInfo().spacingS;

	// create player ui
	QVBoxLayout* task_layout = new QVBoxLayout();
	task_layout->setSpacing(spacingL);
	QHBoxLayout* player_layout = new QHBoxLayout();
	player_layout->setSpacing(0);
	player_layout->setContentsMargins(ui_tools::kMarginZero);
	QString player_btn_stylesheet = theme->toolEndoExplorerStyleSheet();
	for (int id = 0; id < EndoPlayerID::PLAYER_END; ++id) {
		player_[id].reset(new QToolButton());
		player_[id]->setContentsMargins(ui_tools::kMarginZero);
		player_[id]->setStyleSheet(player_btn_stylesheet);
		player_layout->addWidget(player_[id].get());
	}
	task_layout->addLayout(player_layout);

	QLabel* direction_label = new QLabel();
	direction_label->setText(lang::LanguagePack::txt_camera_dir());
	direction_label->setAlignment(Qt::AlignLeft);
	task_layout->addWidget(direction_label);

	QHBoxLayout* direction_layout = new QHBoxLayout();
	direction_layout->setSpacing(0);
	direction_layout->setContentsMargins(ui_tools::kMarginZero);
	for (int id = 0; id < EndoCameraDir::ENDO_CAM_END; ++id) {
		direction_[id].reset(new QRadioButton());
		direction_layout->addWidget(direction_[id].get());
	}
	task_layout->addLayout(direction_layout);

	QString slider_stylesheet = theme->appQSliderStyleSheet();
	for (int id = 0; id < EndoPlayerParamID::PLAYER_PARAM_END; ++id) {
		player_param_[id].reset(new QSlider());
		player_param_[id]->setStyleSheet(slider_stylesheet);
		player_param_[id]->setContentsMargins(ui_tools::kMarginZero);
		player_param_[id]->setOrientation(Qt::Horizontal);

		player_param_[id]->setPageStep(1);
		player_param_[id]->setSingleStep(1);
		player_param_[id]->setMinimum(1);
	}

	player_param_[EndoPlayerParamID::PLAY_INTERVAL]->setObjectName(lang::LanguagePack::txt_interval());
	player_param_[EndoPlayerParamID::PLAY_INTERVAL]->setMaximum(10);
	player_param_[EndoPlayerParamID::PLAY_SPEED]->setObjectName(lang::LanguagePack::txt_speed());
	player_param_[EndoPlayerParamID::PLAY_SPEED]->setInvertedAppearance(true);
	player_param_[EndoPlayerParamID::PLAY_SPEED]->setMaximum(5);

	QVBoxLayout* player_speed_layout = new QVBoxLayout();
	player_speed_layout->setSpacing(spacingS);
	player_speed_layout->setContentsMargins(contentsMargin);
	for (int id = 0; id < EndoPlayerParamID::PLAYER_PARAM_END; ++id) {
		QVBoxLayout* slider_layout = new QVBoxLayout();
		slider_layout->setSpacing(spacingS);
		slider_layout->setContentsMargins(ui_tools::kMarginZero);
		
		QLabel* labelCaption = new QLabel();
		labelCaption->setText(player_param_[id]->objectName());
		labelCaption->setAlignment(Qt::AlignLeft);
		slider_layout->addWidget(labelCaption);
		slider_layout->addWidget(player_param_[id].get());
		player_speed_layout->addLayout(slider_layout);
	}
	task_layout->addLayout(player_speed_layout);

	free_explorer_.reset(new QToolButton());
	free_explorer_->setContentsMargins(ui_tools::kMarginZero);
	task_layout->addWidget(free_explorer_.get());

	player_tool_box_.reset(new ToolBox());
	player_tool_box_->setCaptionName(lang::LanguagePack::txt_task(), Qt::AlignLeft);
	player_tool_box_->addToolLayout(task_layout);
	player_tool_box_->setContentsMargins(boxMargins);

	// create path ui
	QVBoxLayout* path_layout = new QVBoxLayout();
	path_layout->setSpacing(spacingM);
	path_layout->setContentsMargins(contentsMargin);

	QHBoxLayout* path_list_layout = new QHBoxLayout();
	path_list_layout->setSpacing(0);
	QVBoxLayout *select_path_layout = new QVBoxLayout();
	select_path_layout->setSpacing(spacingS);
	QVBoxLayout* remove_path_layout = new QVBoxLayout();
	remove_path_layout->setSpacing(spacingS);

	QString path_btn_stylesheet = theme->toolEndoPathStyleSheet();
	for (int id = 0; id < EndoPathID::ENDO_PATH_END; ++id) {
		path_[id].reset(new QRadioButton());
		path_[id]->setText(QString(lang::LanguagePack::txt_path() + "%1").arg(id + 1));
		path_[id]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		select_path_layout->addWidget(path_[id].get());

		remove_path_[id].reset(new QToolButton());
		remove_path_[id]->setContentsMargins(ui_tools::kMarginZero);
		remove_path_[id]->setStyleSheet(path_btn_stylesheet);
		remove_path_[id]->setObjectName("DeletePath");
		remove_path_[id]->setText("X");
		remove_path_layout->addWidget(remove_path_[id].get());
	}
	path_list_layout->addLayout(select_path_layout);
	path_list_layout->addLayout(remove_path_layout);
	path_layout->addLayout(path_list_layout);

	show_path_.reset(new QCheckBox());
	show_path_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	show_path_->setEnabled(false);
	path_layout->addWidget(show_path_.get());

	path_tool_box_.reset(new ToolBox());
	path_tool_box_->setCaptionName(lang::LanguagePack::txt_task(), Qt::AlignLeft);
	path_tool_box_->addToolLayout(path_layout);
	path_tool_box_->setContentsMargins(boxMargins);

	// create visibility tool ui
	visibility_tool_.reset(new VisibilityToolBox(false, false, false, true, false, this));

	// status setting
	free_explorer_->setObjectName("FreeExplorerOnOff");
	player_[EndoPlayerID::START_POS]->setObjectName("Start");
	player_[EndoPlayerID::STOP]->setObjectName("Stop");
	player_[EndoPlayerID::PLAY]->setObjectName("Play");
	player_[EndoPlayerID::END_POS]->setObjectName("End");
	player_[EndoPlayerID::PREV]->setObjectName("Prev");
	player_[EndoPlayerID::NEXT]->setObjectName("Next");
	direction_[EndoCameraDir::FORWARD]->setObjectName("Forward");
	direction_[EndoCameraDir::BACKWARD]->setObjectName("Backward");
	direction_[EndoCameraDir::CAM_FIXED]->setObjectName("Fixed");
	show_path_->setObjectName("ShowPath");

	free_explorer_->setText(lang::LanguagePack::txt_free_explorer());
	free_explorer_->setCheckable(true);
	direction_[EndoCameraDir::FORWARD]->setText(lang::LanguagePack::txt_forward());
	direction_[EndoCameraDir::BACKWARD]->setText(lang::LanguagePack::txt_backward());
	direction_[EndoCameraDir::CAM_FIXED]->setText(lang::LanguagePack::txt_fixed());
	show_path_->setText(lang::LanguagePack::txt_show_path());
}

void EndoTaskTool::SetToolTips() {

}

void EndoTaskTool::Connections() {
	for (int id = 0; id < EndoPlayerID::PLAYER_END; ++id)
		connect(player_[id].get(), &QToolButton::clicked, [=]() { emit sigEndoPlayerAction(static_cast<EndoPlayerID>(id), QPrivateSignal()); });
	
	connect(free_explorer_.get(), &QToolButton::toggled, [=](bool state) { emit sigEndoFreeOnOff(state, QPrivateSignal()); });

	for (int id = 0; id < EndoPlayerParamID::PLAYER_PARAM_END; ++id) {
		connect(player_param_[id].get(), &QSlider::valueChanged,
				[=](int value) { emit sigEndoPlayerParam(static_cast<EndoPlayerParamID>(id), value, QPrivateSignal()); });
	}

	for (int id = 0; id < EndoCameraDir::ENDO_CAM_END; ++id) {
		connect(direction_[id].get(), &QToolButton::clicked, [=]() { emit sigEndoCamDir(static_cast<EndoCameraDir>(id), QPrivateSignal()); });
	}

	for (int id = 0; id < EndoPathID::ENDO_PATH_END; ++id) {
		connect(path_[id].get(), &QToolButton::clicked, [=]() { emit sigEndoSelectPath(static_cast<EndoPathID>(id), QPrivateSignal()); });
		connect(remove_path_[id].get(), &QToolButton::clicked,
				[=]() {
					emit sigEndoRemovePath(static_cast<EndoPathID>(id), QPrivateSignal());
					remove_path_[id]->setEnabled(false);
				});
	}

	connect(visibility_tool_.get(), &VisibilityToolBox::sigVisible,
			[=](const VisibleID& id, int state) { emit sigEndoVisible(state, QPrivateSignal()); });

	connect(show_path_.get(), &QCheckBox::stateChanged, [=](int state) { emit sigEndoShowPath(state, QPrivateSignal()); });
}
