#include "pacs_3d_setting_widget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QButtonGroup>
#include <QRadioButton>

#include "../../Common/Common/global_preferences.h"
#include "../../Common/Common/W3Theme.h"
#include "../../Common/Common/W3Memory.h"

namespace
{
	const QMargins kMarginZero(0, 0, 0, 0);
}

Pacs3DSettingWidget::Pacs3DSettingWidget(QWidget* parent /*= 0*/)
	: QWidget(parent)
{
	Initialize();
	SetLayout();
}

Pacs3DSettingWidget::~Pacs3DSettingWidget()
{
	SAFE_DELETE_LATER(main_layout_);
	SAFE_DELETE_LATER(rotation_type_button_group_);
	SAFE_DELETE_LATER(rotation_dir_button_group_);
}

void Pacs3DSettingWidget::slotRotationTypeChange(const int button_id)
{
	RotationType new_type = static_cast<RotationType>(button_id);
	if (rot_type != new_type)
	{
		rot_type = new_type;
		emit sigRotationTypeChange();
	}
}

void Pacs3DSettingWidget::slotRotationDirChange(const int button_id)
{
	RotationDir new_dir = static_cast<RotationDir>(button_id);
	if (rot_dir != new_dir)
	{
		rot_dir = new_dir;
		emit sigRotationDirChange();
	}
}

void Pacs3DSettingWidget::Initialize()
{
	main_layout_ = new QVBoxLayout();
	main_layout_->setContentsMargins(kMarginZero);
	main_layout_->setSpacing(0);

	setLayout(main_layout_);
}

void Pacs3DSettingWidget::SetLayout()
{
	main_layout_->addLayout(CreateVRSettingLayout());
}

QVBoxLayout* Pacs3DSettingWidget::CreateVRSettingLayout()
{
	int spacing_s = 5;
	QMargins contents_margin = QMargins(5, 5, 5, 5);

	QVBoxLayout* contents_layout = new QVBoxLayout();
	{
		contents_layout->setAlignment(Qt::AlignTop);
		contents_layout->setSpacing(10);
		contents_layout->setContentsMargins(kMarginZero);

		GlobalPreferences::PACSDefaultSetting& setting = GlobalPreferences::GetInstance()->preferences_.pacs_default_setting;

		QHBoxLayout* rotation_type_layout = new QHBoxLayout();
		{
			rotation_type_layout->setSpacing(10);
			rotation_type_layout->setContentsMargins(kMarginZero);

			QLabel* label = new QLabel(this);
			label->setText(lang::LanguagePack::txt_rotation_type());

			rotation_type_button_group_ = new QButtonGroup(this);			

			QRadioButton* horizontal_radio = new QRadioButton(this);
			horizontal_radio->setText(lang::LanguagePack::txt_horizontal());			

			QRadioButton* vertical_radio = new QRadioButton(this);
			vertical_radio->setText(lang::LanguagePack::txt_vertical());			

			rotation_type_button_group_->addButton(horizontal_radio, static_cast<int>(RotationType::horizontal));
			rotation_type_button_group_->addButton(vertical_radio, static_cast<int>(RotationType::vertical));

			if (setting.mpr_is_3d_vertical)
			{
				vertical_radio->setChecked(true);
				rot_type = RotationType::vertical;
			}
			else
			{
				horizontal_radio->setChecked(true);
				rot_type = RotationType::horizontal;
			}

			rotation_type_layout->addWidget(label);
			rotation_type_layout->addWidget(horizontal_radio);
			rotation_type_layout->addWidget(vertical_radio);

			connect(rotation_type_button_group_, SIGNAL(buttonClicked(int)), this, SLOT(slotRotationTypeChange(int)));
		}

		QHBoxLayout* rotation_dir_layout = new QHBoxLayout();
		{
			rotation_dir_layout->setSpacing(10);
			rotation_dir_layout->setContentsMargins(kMarginZero);

			QLabel* label = new QLabel(this);
			label->setText(lang::LanguagePack::txt_rotation_dir());

			rotation_dir_button_group_ = new QButtonGroup(this);

			QRadioButton* anterior_radio = new QRadioButton(this);
			anterior_radio->setText(lang::LanguagePack::txt_anterior());
			
			QRadioButton* posterior_radio = new QRadioButton(this);
			posterior_radio->setText(lang::LanguagePack::txt_posterior());

			rotation_dir_button_group_->addButton(anterior_radio, static_cast<int>(RotationDir::anterior));
			rotation_dir_button_group_->addButton(posterior_radio, static_cast<int>(RotationDir::posterior));

			if (setting.mpr_is_dir_posterior)
			{
				posterior_radio->setChecked(true);
				rot_dir = RotationDir::posterior;
			}
			else
			{
				anterior_radio->setChecked(true);
				rot_dir = RotationDir::anterior;
			}

			rotation_dir_layout->addWidget(label);
			rotation_dir_layout->addWidget(anterior_radio);
			rotation_dir_layout->addWidget(posterior_radio);

			connect(rotation_dir_button_group_, SIGNAL(buttonClicked(int)), this, SLOT(slotRotationDirChange(int)));
		}

		QVBoxLayout* slider_layout = new QVBoxLayout();
		{
			slider_layout->setSpacing(spacing_s);
			slider_layout->setContentsMargins(kMarginZero);

			QString slider_stylesheet = CW3Theme::getInstance()->appQSliderStyleSheet();
			QSlider* angle_slider = new QSlider(this);
			angle_slider->setStyleSheet(slider_stylesheet);
			angle_slider->setContentsMargins(kMarginZero);
			angle_slider->setOrientation(Qt::Horizontal);
			angle_slider->setRange(1, 30);
			angle_slider->setValue(setting.mpr_3d_angle);

			QHBoxLayout* label_layout = new QHBoxLayout();
			label_layout->setSpacing(0);
			label_layout->setContentsMargins(kMarginZero);

			QLabel* slider_name = new QLabel(this);
			slider_name->setText(lang::LanguagePack::txt_angle() + " : ");
			slider_name->setAlignment(Qt::AlignLeft);

			QLabel* slider_value = new QLabel(this);
			slider_value->setText(QString::number(angle_slider->value()));
			slider_value->setAlignment(Qt::AlignRight);

			label_layout->addWidget(slider_name);
			label_layout->addWidget(slider_value);

			slider_layout->addLayout(label_layout);
			slider_layout->addWidget(angle_slider);

			connect(angle_slider, &QSlider::valueChanged, [=](const int value)
			{
				slider_value->setText(QString::number(value));
				emit sigUpdateAngle(value);
			});
		}

		contents_layout->addLayout(rotation_type_layout);
		contents_layout->addLayout(rotation_dir_layout);
		contents_layout->addLayout(slider_layout);
	}

	return contents_layout;
}
