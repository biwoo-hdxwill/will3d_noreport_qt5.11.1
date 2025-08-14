#include "otf_tool.h"
#include <QBoxLayout>
#include <QToolButton>
#include <QSettings>
#include <QLabel>
#include <QSlider>
#include <QTextCodec>

#include <Engine/Common/Common/global_preferences.h>
#include <Engine/Common/Common/W3Theme.h>
#include <Engine/Common/Common/define_otf.h>
#include <Engine/Common/Common/language_pack.h>

#include "tool_box.h"

OTFTool::OTFTool(QObject *parent)
	: BaseTool(parent) {
	CreateUI();
	ResetUI();
	SetToolTips();
	Connections();

	QSettings settings(GlobalPreferences::GetInstance()->ini_path(), QSettings::IniFormat);
	QString default_otf_preset = settings.value("OTF/default", common::otf_preset::BONE).toString();
	QStringList split = default_otf_preset.split('_');
	int remove_index = default_otf_preset.indexOf('_');

	if (split.size() >= 2)
	{
		default_otf_preset = default_otf_preset.right(default_otf_preset.size() - 1 - remove_index);
	}
	else
	{
		default_otf_preset = common::otf_preset::BONE;
	}
	InitOTFPreset(default_otf_preset);
}

OTFTool::~OTFTool() {
}

void OTFTool::ResetUI() {
	for (int id = 0; id < OTFSliderID::SL_END; ++id)
		slider_[id]->setValue(50);
}

bool OTFTool::SetOTFButtonStatus(const QString & preset) {
	bool is_set_tf = false;
	for (int id = 0; id < OTFPresetID::PRESET_END; ++id) {
		QString str = preset_[id]->objectName();
		if (preset.compare(str.right(str.length() - 3)) == 0 && !is_set_tf) {
			preset_[id]->setChecked(true);
			is_set_tf = true;
		} else {
			preset_[id]->setChecked(false);
		}
	}
	return is_set_tf;
}

QWidget * OTFTool::GetWidget() {
	return tool_box_.get();
}

void OTFTool::InitOTFPreset(const QString & curr_preset) {
	QString sectionKey = "OTF/favorite";

	QSettings settings("Will3D.ini", QSettings::IniFormat);
	settings.setIniCodec(QTextCodec::codecForName("UTF-8"));

	QStringList favorite_list = settings.value(sectionKey).toStringList();

	if (favorite_list.size() < 1) {
		favorite_list.append("01_teeth");
		favorite_list.append("02_gray");
		favorite_list.append("03_soft_tissue1");
		favorite_list.append("04_soft_tissue2");
		favorite_list.append("05_bone");
		favorite_list.append("06_mip");
		favorite_list.append("07_xray");
		settings.setValue(sectionKey, favorite_list);
	}

	CW3Theme* theme = CW3Theme::getInstance();

	QString preset_stylesheet = theme->toolOTFStyleSheet();
	for (int id = 0; id < OTFPresetID::PRESET_END; ++id) {
		QString objName = "0";

		if (id < favorite_list.size())
			objName = favorite_list.at(id);

		preset_[id]->setObjectName(objName);
		preset_[id]->setStyleSheet(preset_stylesheet);
		preset_[id]->setCheckable(objName.compare("0"));

		if (curr_preset.compare("0")) {
			if (objName.contains(curr_preset, Qt::CaseInsensitive)) {
				preset_[id]->setChecked(true);
			} else {
				preset_[id]->setChecked(false);
			}
		} else {
			preset_[id]->setChecked(false);
		}
	}
}

void OTFTool::slotAuto() {
	ResetUI();
	emit sigOTFAuto();
}

void OTFTool::slotPreset() {
	for (int id = 0; id < OTFPresetID::PRESET_END; ++id)
		preset_[id]->setChecked(false);

	QToolButton* pBtn = dynamic_cast<QToolButton*>(QObject::sender());
	pBtn->setChecked(true);
	QString preset = pBtn->objectName();
	preset = preset.right(preset.length() - 3);
	emit sigOTFPreset(preset);
}

void OTFTool::slotAdjust() {
	AdjustOTF values;
	values.bright = (float)slider_[OTFSliderID::BRIGHTNESS]->value();
	values.contrast = (float)slider_[OTFSliderID::CONTRAST]->value();
	values.opacity = (float)slider_[OTFSliderID::OPACITY]->value();
	emit sigOTFAdjust(values);
}

void OTFTool::CreateUI() {
	CW3Theme* theme = CW3Theme::getInstance();
	CW3Theme::toolVBarSizeInfo sInfo = CW3Theme::getInstance()->getToolVBarSizeInfo();
	int spacingM = sInfo.spacingM;
	int spacingS = sInfo.spacingS;
	QMargins contentsMargin = sInfo.marginContents;

	// create preset buttons
	QGridLayout* preset_layout = new QGridLayout();
	preset_layout->setSpacing(2);
	QString preset_stylesheet = theme->toolOTFStyleSheet();
	for (int id = 0; id < OTFPresetID::PRESET_END; ++id) {
		preset_[id].reset(new QToolButton());
		preset_[id]->setContentsMargins(ui_tools::kMarginZero);
		preset_[id]->setStyleSheet(preset_stylesheet);
		preset_[id]->setStyle(theme->toolIconButtonStyle());
		preset_[id]->setCheckable(true);
		preset_layout->addWidget(preset_[id].get(), id / 4, id % 4);
	}

	// create otf sliders
	QString slider_stylesheet = theme->appQSliderStyleSheet();
	QVBoxLayout* otf_slider_layout = new QVBoxLayout();
	otf_slider_layout->setSpacing(spacingS);
	otf_slider_layout->setContentsMargins(contentsMargin);
	for (int id = 0; id < OTFSliderID::SL_END; ++id) {
		slider_[id].reset(new QSlider());
		slider_[id]->setStyleSheet(slider_stylesheet);
		slider_[id]->setContentsMargins(ui_tools::kMarginZero);
		slider_[id]->setOrientation(Qt::Horizontal);
	}

	slider_[OTFSliderID::OPACITY]->setObjectName(lang::LanguagePack::txt_opacity());
	slider_[OTFSliderID::BRIGHTNESS]->setObjectName(lang::LanguagePack::txt_brightness());
	slider_[OTFSliderID::CONTRAST]->setObjectName(lang::LanguagePack::txt_contrast());
	slider_[OTFSliderID::OPACITY]->setMinimum(1);

	for (int id = 0; id < OTFSliderID::SL_END; ++id) {
		QLabel* labelCaption = new QLabel();
		labelCaption->setText(slider_[id]->objectName());
		labelCaption->setAlignment(Qt::AlignLeft);
		QLabel* otf_value = new QLabel();
		otf_value->setText(QString::number(50));
		otf_value->setAlignment(Qt::AlignRight);

		QHBoxLayout* layout_label = new QHBoxLayout();
		layout_label->setSpacing(0);
		layout_label->setContentsMargins(ui_tools::kMarginZero);
		layout_label->addWidget(labelCaption);
		layout_label->addWidget(otf_value);

		QVBoxLayout* slider_layout = new QVBoxLayout();
		slider_layout->setSpacing(spacingS);
		slider_layout->setContentsMargins(ui_tools::kMarginZero);
		slider_layout->addLayout(layout_label);
		slider_layout->addWidget(slider_[id].get());

		otf_slider_layout->addLayout(slider_layout);

		connect(slider_[id].get(), &QSlider::valueChanged,
				[=](const int new_value) { otf_value->setText(QString::number(new_value)); });
	}
	
	// create manual, auto buttons
	QHBoxLayout* otf_button_layout = new QHBoxLayout();
	otf_button_layout->setContentsMargins(contentsMargin);
	otf_button_layout->setSpacing(spacingM);

	QString orient_stylesheet = theme->toolOrientationStyleSheet();
	auto_.reset(new QToolButton());
	auto_->setContentsMargins(ui_tools::kMarginZero);
	auto_->setStyleSheet(orient_stylesheet);
	auto_->setObjectName("AutoOTF");
	auto_->setText(lang::LanguagePack::txt_auto());
	otf_button_layout->addWidget(auto_.get());

	manual_.reset(new QToolButton());
	manual_->setContentsMargins(ui_tools::kMarginZero);
	manual_->setStyleSheet(orient_stylesheet);
	manual_->setObjectName("ManualOTF");
	manual_->setText(lang::LanguagePack::txt_manual());
	otf_button_layout->addWidget(manual_.get());

	// create tool box
	QVBoxLayout* tool_box_layout = new QVBoxLayout();
	tool_box_layout->setContentsMargins(ui_tools::kMarginZero);
	tool_box_layout->setSpacing(4);
	tool_box_layout->addLayout(preset_layout);
	tool_box_layout->addLayout(otf_slider_layout);
	tool_box_layout->addLayout(otf_button_layout);

	QMargins boxMargins = CW3Theme::getInstance()->getToolVBarSizeInfo().marginBox;
	tool_box_.reset(new ToolBox());
	tool_box_->setCaptionName(tr("3D"), Qt::AlignLeft);
	tool_box_->addToolLayout(tool_box_layout);
	tool_box_->setContentsMargins(boxMargins);
}

void OTFTool::Connections() {
	connect(auto_.get(), SIGNAL(clicked()), this, SLOT(slotAuto()));
	connect(manual_.get(), SIGNAL(clicked()), this, SIGNAL(sigOTFManualOnOff()));
	for (int id = 0; id < OTFPresetID::PRESET_END; ++id)
		connect(preset_[id].get(), SIGNAL(clicked()), this, SLOT(slotPreset()));

	for (int id = 0; id < OTFSliderID::SL_END; ++id) {
		connect(slider_[id].get(), SIGNAL(valueChanged(int)), this, SLOT(slotAdjust()));
		connect(slider_[id].get(), SIGNAL(sliderReleased()), this, SIGNAL(sigOTFAdjustDone()));
	}
}

void OTFTool::SetToolTips() {

}
