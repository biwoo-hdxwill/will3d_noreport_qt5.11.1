#include "pacs_mpr_setting_widget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QTabWidget>
#include <QSlider>
#include <QButtonGroup>
#include <QRadioButton>

#include "../../Common/Common/W3Theme.h"
#include "../../Common/Common/global_preferences.h"
#include "../../Common/Common/W3Memory.h"

namespace
{
	const QMargins kMarginZero(0, 0, 0, 0);
}

PacsMPRSettingWidget::PacsMPRSettingWidget(QWidget* parent /*= 0*/)
	: QWidget(parent)
{
	Initialize();
	SetLayout();
}

PacsMPRSettingWidget::~PacsMPRSettingWidget()
{
	int size = static_cast<int>(PACS_MPR_SliderID::END);
	for (int i = 0; i < size; ++i)
	{
		SAFE_DELETE_LATER(mpr_slider_layouts_[i]);
		SAFE_DELETE_LATER(mpr_sliders_[i]);
	}
	SAFE_DELETE_LATER(main_layout_);
}

void PacsMPRSettingWidget::slotRotationTypeChange(int id)
{
	if (radio_check_id_ != id)
	{
		radio_check_id_ = id;
		emit sigRotationTypeChange();
	}
}

void PacsMPRSettingWidget::Initialize()
{
	main_layout_ = new QVBoxLayout();
	main_layout_->setContentsMargins(kMarginZero);
	main_layout_->setSpacing(0);

	setLayout(main_layout_);

	CreateMPRSliderLayouts();
}

void PacsMPRSettingWidget::SetLayout()
{
	main_layout_->addWidget(CreateMPRSettingTabWidget());
}

QTabWidget* PacsMPRSettingWidget::CreateMPRSettingTabWidget()
{
	QTabWidget* tab_widgget = new QTabWidget(this);

	int spacing_s = 5;
	QMargins contents_margin = QMargins(5, 5, 5, 5);

	QVBoxLayout* mpr_translation_setting_layout = new QVBoxLayout();
	{
		mpr_translation_setting_layout->setSpacing(spacing_s);
		mpr_translation_setting_layout->setContentsMargins(contents_margin);
		mpr_translation_setting_layout->setAlignment(Qt::AlignTop);

		int size = static_cast<int>(PACS_MPR_SliderID::ROT_ANGLE);
		for (int i = 0; i < size; ++i)
		{
			mpr_translation_setting_layout->addLayout(mpr_slider_layouts_[i]);
		}
	}

	QVBoxLayout* mpr_rotation_setting_layout = new QVBoxLayout();
	{
		mpr_rotation_setting_layout->setSpacing(spacing_s);
		mpr_rotation_setting_layout->setContentsMargins(contents_margin);
		mpr_rotation_setting_layout->setAlignment(Qt::AlignTop);

		QHBoxLayout* radio_layout = new QHBoxLayout();
		{
			QButtonGroup* button_group = new QButtonGroup(this);

			GlobalPreferences::PACSDefaultSetting& setting = GlobalPreferences::GetInstance()->preferences_.pacs_default_setting;
			bool is_horizontal = !setting.mpr_is_2d_vertical;
			if (!is_horizontal)
			{
				radio_check_id_ = 1;
			}

			int size = static_cast<int>(PACS_Rotation_Type::END);
			for (int i = 0; i < size; ++i)
			{
				QRadioButton* radio = new QRadioButton(this);
				if (i == static_cast<int>(PACS_Rotation_Type::HORIZONTAL))
				{
					radio->setText(lang::LanguagePack::txt_horizontal());
					radio->setChecked(is_horizontal);
				}
				else if (i == static_cast<int>(PACS_Rotation_Type::VERTICAL))
				{
					radio->setText(lang::LanguagePack::txt_vertical());
					radio->setChecked(!is_horizontal);
				}

				button_group->addButton(radio, i);
				radio_layout->addWidget(radio);
			}
					
			connect(button_group, SIGNAL(buttonClicked(int)), this, SLOT(slotRotationTypeChange(int)));
		}

		mpr_rotation_setting_layout->addLayout(radio_layout);

		int size = static_cast<int>(PACS_MPR_SliderID::END);
		for (int i = static_cast<int>(PACS_MPR_SliderID::ROT_ANGLE); i < size; ++i)
		{
			mpr_rotation_setting_layout->addLayout(mpr_slider_layouts_[i]);
		}
	}

	QWidget* translate_widget = new  QWidget(tab_widgget);
	translate_widget->setContentsMargins(contents_margin);
	translate_widget->setLayout(mpr_translation_setting_layout);

	QWidget* rotation_widget = new  QWidget(tab_widgget);
	rotation_widget->setContentsMargins(contents_margin);
	rotation_widget->setLayout(mpr_rotation_setting_layout);

	tab_widgget->addTab(translate_widget, lang::LanguagePack::txt_translation());
	tab_widgget->addTab(rotation_widget, lang::LanguagePack::txt_rotation());

	connect(tab_widgget, &QTabWidget::currentChanged, [=]()
	{
		emit sigSliderTypeChange();
	});

	return tab_widgget;
}

void PacsMPRSettingWidget::EmitUpdateMPRSlider(const int id)
{
	int value = mpr_sliders_[id]->value();
	if (prev_slider_value_[id] == value)
	{
		return;
	}

	prev_slider_value_[id] = value;

	if (id == static_cast<int>(PACS_MPR_SliderID::TRANS_INTERVAL))
	{
		emit sigUpdateInterval(value);
	}
	else if (id == static_cast<int>(PACS_MPR_SliderID::ROT_ANGLE))
	{
		emit sigUpdateAngle(value);
	}
	else if (id == static_cast<int>(PACS_MPR_SliderID::TRANS_THICKNESS))
	{
		int sibling_id = static_cast<int>(PACS_MPR_SliderID::ROT_THICKNESS);
		prev_slider_value_[sibling_id] = value;
		mpr_sliders_[sibling_id]->setValue(value);

		emit sigUpdateThickness(value);
	}
	else if (id == static_cast<int>(PACS_MPR_SliderID::ROT_THICKNESS))
	{
		int sibling_id = static_cast<int>(PACS_MPR_SliderID::TRANS_THICKNESS);
		prev_slider_value_[sibling_id] = value;
		mpr_sliders_[sibling_id]->setValue(value);

		emit sigUpdateThickness(value);
	}
	else if (id == static_cast<int>(PACS_MPR_SliderID::TRANS_FILTER))
	{
		int sibling_id = static_cast<int>(PACS_MPR_SliderID::ROT_FILTER);
		prev_slider_value_[sibling_id] = value;
		mpr_sliders_[sibling_id]->setValue(value);

		emit sigUpdateFilter(value);
	}
	else if (id == static_cast<int>(PACS_MPR_SliderID::ROT_FILTER))
	{
		int sibling_id = static_cast<int>(PACS_MPR_SliderID::TRANS_FILTER);
		prev_slider_value_[sibling_id] = value;
		mpr_sliders_[sibling_id]->setValue(value);

		emit sigUpdateFilter(value);
	}
}

void PacsMPRSettingWidget::CreateMPRSliderLayouts()
{
	QString slider_stylesheet = CW3Theme::getInstance()->appQSliderStyleSheet();
	int size = static_cast<int>(PACS_MPR_SliderID::END);
	for (int i = 0; i < size; ++i)
	{
		mpr_sliders_[i] = new QSlider(this);
		mpr_sliders_[i]->setStyleSheet(slider_stylesheet);
		mpr_sliders_[i]->setContentsMargins(kMarginZero);
		mpr_sliders_[i]->setOrientation(Qt::Horizontal);
	}

	GlobalPreferences::PACSDefaultSetting& setting = GlobalPreferences::GetInstance()->preferences_.pacs_default_setting;

	int id = static_cast<int>(PACS_MPR_SliderID::TRANS_INTERVAL);
	mpr_sliders_[id]->setObjectName(lang::LanguagePack::txt_interval() + " : ");
	mpr_sliders_[id]->setRange(1, 10);
	mpr_sliders_[id]->setValue(setting.mpr_interval);

	id = static_cast<int>(PACS_MPR_SliderID::TRANS_THICKNESS);
	mpr_sliders_[id]->setObjectName(lang::LanguagePack::txt_thickness() + " : ");
	mpr_sliders_[id]->setRange(1, 50);
	mpr_sliders_[id]->setValue(setting.mpr_thickness);

	id = static_cast<int>(PACS_MPR_SliderID::TRANS_FILTER);
	mpr_sliders_[id]->setObjectName(lang::LanguagePack::txt_filter());
	mpr_sliders_[id]->setRange(0, 3);
	mpr_sliders_[id]->setValue(setting.mpr_filter_level);

	id = static_cast<int>(PACS_MPR_SliderID::ROT_ANGLE);
	mpr_sliders_[id]->setObjectName(lang::LanguagePack::txt_angle() + " : ");
	mpr_sliders_[id]->setRange(1, 30);
	mpr_sliders_[id]->setValue(setting.mpr_2d_angle);

	id = static_cast<int>(PACS_MPR_SliderID::ROT_THICKNESS);
	mpr_sliders_[id]->setObjectName(lang::LanguagePack::txt_thickness() + " : ");
	mpr_sliders_[id]->setRange(1, 50);
	mpr_sliders_[id]->setValue(setting.mpr_thickness);

	id = static_cast<int>(PACS_MPR_SliderID::ROT_FILTER);
	mpr_sliders_[id]->setObjectName(lang::LanguagePack::txt_filter());
	mpr_sliders_[id]->setRange(0, 3);
	mpr_sliders_[id]->setValue(setting.mpr_filter_level);

	CW3Theme::toolVBarSizeInfo size_info = CW3Theme::getInstance()->getToolVBarSizeInfo();
	for (int i = 0; i < size; ++i)
	{
		prev_slider_value_[i] = mpr_sliders_[i]->value();

		QLabel* slider_name = new QLabel(this);
		slider_name->setText(mpr_sliders_[i]->objectName());
		slider_name->setAlignment(Qt::AlignLeft);

		QLabel* slider_value = new QLabel(this);
		slider_value->setText(QString::number(mpr_sliders_[i]->value()));
		slider_value->setAlignment(Qt::AlignRight);

		QHBoxLayout* label_layout = new QHBoxLayout();
		label_layout->setSpacing(0);
		label_layout->setContentsMargins(kMarginZero);
		label_layout->addWidget(slider_name);
		label_layout->addWidget(slider_value);

		mpr_slider_layouts_[i] = new QVBoxLayout();
		mpr_slider_layouts_[i]->setSpacing(size_info.spacingS);
		mpr_slider_layouts_[i]->setContentsMargins(kMarginZero);
		mpr_slider_layouts_[i]->addLayout(label_layout);
		mpr_slider_layouts_[i]->addWidget(mpr_sliders_[i]);

		connect(mpr_sliders_[i], &QSlider::valueChanged, [=](const int value)
		{
			slider_value->setText(QString::number(value));
			EmitUpdateMPRSlider(i);
		});
	}
}
