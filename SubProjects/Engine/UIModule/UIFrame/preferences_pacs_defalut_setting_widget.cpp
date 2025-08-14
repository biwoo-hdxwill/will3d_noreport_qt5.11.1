#include "preferences_pacs_defalut_setting_widget.h"

#include <QLabel>
#include <QSpinBox>
#include <QCheckBox>
#include <QButtonGroup>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include "../../Common/Common/language_pack.h"
#include "../../Common/Common/W3Theme.h"
#include "../../Common/Common/global_preferences.h"

namespace
{
	//default
	const int kMPRSliceAngle = 1;
	const int kMPRSliceInterval = 1;
	const int kMPRSliceThickness = 1;
	const int kMPRFilterLevel = 0;

	const int kMPR3DAngle = 1;
}

PreferencesPACSDefaultSettingWidget::PreferencesPACSDefaultSettingWidget(QWidget* parent /*= nullptr*/)
	: BaseWidget(parent)
{
	contents_layout()->addLayout(CreateMainContentsLayout());
}

PreferencesPACSDefaultSettingWidget::~PreferencesPACSDefaultSettingWidget()
{

}

void PreferencesPACSDefaultSettingWidget::SetGlobalPreferences()
{
	GlobalPreferences::PACSDefaultSetting& pacs_defalut_setting = GlobalPreferences::GetInstance()->preferences_.pacs_default_setting;

	//MPR 2D
	pacs_defalut_setting.mpr_view_list_swap = view_list_swap_check_->isChecked();
	pacs_defalut_setting.mpr_is_2d_vertical = slice_rotate_type_vertical_radio_->isChecked();
	pacs_defalut_setting.mpr_2d_angle = slice_angel_spin_->value();
	pacs_defalut_setting.mpr_interval = slice_interval_spin_->value();
	pacs_defalut_setting.mpr_thickness = slice_thickness_spin_->value();
	pacs_defalut_setting.mpr_filter_level = slice_filter_spin_->value();

	//MPR 3D
	pacs_defalut_setting.mpr_is_dir_posterior = volume_rotate_dir_Posterior_radio_->isChecked();
	pacs_defalut_setting.mpr_is_3d_vertical = volume_slice_rotate_type_vertical_radio_->isChecked();
	pacs_defalut_setting.mpr_3d_angle = volume_angel_spin_->value();
}

void PreferencesPACSDefaultSettingWidget::Reset()
{
	//MPR 2D
	view_list_swap_check_->setChecked(false);
	slice_rotate_type_horizontal_radio_->setChecked(true);
	slice_interval_spin_->setValue(kMPRSliceInterval);
	slice_angel_spin_->setValue(kMPRSliceAngle);
	slice_thickness_spin_->setValue(kMPRSliceThickness);
	slice_filter_spin_->setValue(kMPRFilterLevel);

	//MPR 3D
	volume_rotate_dir_Anterior_radio_->setChecked(true);
	volume_rotate_type_horizontal_radio_->setChecked(true);
	volume_angel_spin_->setValue(kMPR3DAngle);
}

QVBoxLayout* PreferencesPACSDefaultSettingWidget::CreateMainContentsLayout()
{
	QVBoxLayout* pacs_layout = new QVBoxLayout();
	{
		pacs_layout->setAlignment(Qt::AlignTop);
		pacs_layout->setContentsMargins(kContentsMargins);
		pacs_layout->setSpacing(kSpacing10);

		pacs_layout->addLayout(CreatePACSDefaultSettingLayout());
	}

	GetPACSValues();

	return pacs_layout;
}

QVBoxLayout* PreferencesPACSDefaultSettingWidget::CreatePACSDefaultSettingLayout()
{
	QVBoxLayout* default_setting_layout = new QVBoxLayout();
	{
		default_setting_layout->setSpacing(kSpacing10);
		default_setting_layout->setContentsMargins(kMarginZero);

		QVBoxLayout* contents_layout = new QVBoxLayout();
		{
			contents_layout->addLayout(CreateMPRDefaultSetting2D());
			contents_layout->addLayout(CreateMPRDefaultSetting3D());
		}

		default_setting_layout->addLayout(contents_layout);
	}

	return default_setting_layout;
}

QVBoxLayout* PreferencesPACSDefaultSettingWidget::CreateMPRDefaultSetting2D()
{
	QVBoxLayout* default_setting_2d_layout = new QVBoxLayout();
	{
		default_setting_2d_layout->setSpacing(kSpacing10);
		default_setting_2d_layout->setContentsMargins(kMarginZero);

		QHBoxLayout* contents_layout = new QHBoxLayout();
		{
			contents_layout->setSpacing(kSpacing10);
			contents_layout->setContentsMargins(kStepMargins);

			QHBoxLayout* left_layout = new QHBoxLayout();
			{
				QVBoxLayout* label_layout = new QVBoxLayout();
				{
					label_layout->setSpacing(kSpacing10);

					QLabel* view_list_swap_label = CreateLabel(lang::LanguagePack::txt_view_list_swap() + " :");
					QLabel* interval_label = CreateLabel(lang::LanguagePack::txt_interval() + " (mm) :");
					QLabel* thickness_label = CreateLabel(lang::LanguagePack::txt_thickness() + " :");

					label_layout->addWidget(view_list_swap_label);
					label_layout->addWidget(interval_label);
					label_layout->addWidget(thickness_label);
				}

				QVBoxLayout* controller_layout = new QVBoxLayout();
				{
					controller_layout->setSpacing(kSpacing10);

					view_list_swap_check_ = new QCheckBox(this);
					slice_interval_spin_ = CreateSpinBox(0, 10, 0);
					slice_thickness_spin_ = CreateSpinBox(1, 50, 1);

					controller_layout->addWidget(view_list_swap_check_);
					controller_layout->addWidget(slice_interval_spin_);
					controller_layout->addWidget(slice_thickness_spin_);
				}

				left_layout->addLayout(label_layout);
				left_layout->addLayout(controller_layout);
			}

			QHBoxLayout* right_layout = new QHBoxLayout();
			{
				QVBoxLayout* label_layout = new QVBoxLayout();
				{
					label_layout->setSpacing(kSpacing10);

					QLabel* rotation_type_label = CreateLabel(lang::LanguagePack::txt_rotation_type() + " :");
					QLabel* angel_label = CreateLabel(lang::LanguagePack::txt_angle() + " :");
					QLabel* filter_label = CreateLabel(lang::LanguagePack::txt_filter());

					label_layout->addWidget(rotation_type_label);
					label_layout->addWidget(angel_label);
					label_layout->addWidget(filter_label);
				}

				QVBoxLayout* controller_layout = new QVBoxLayout();
				{
					controller_layout->setSpacing(kSpacing10);

					QButtonGroup* slice_rotate_type_radio_group = new QButtonGroup(this);
					slice_rotate_type_horizontal_radio_ = CreateTextRadioBttton(lang::LanguagePack::txt_horizontal(), QSizePolicy::Fixed);
					slice_rotate_type_vertical_radio_ = CreateTextRadioBttton(lang::LanguagePack::txt_vertical(), QSizePolicy::Fixed);

					slice_rotate_type_radio_group->addButton(slice_rotate_type_horizontal_radio_);
					slice_rotate_type_radio_group->addButton(slice_rotate_type_vertical_radio_);
					slice_rotate_type_horizontal_radio_->setChecked(true);

					slice_angel_spin_ = CreateSpinBox(1, 30, 1);
					slice_filter_spin_ = CreateSpinBox(0, 3, 0);

					QHBoxLayout* rotate_type_layout = new QHBoxLayout();
					rotate_type_layout->addWidget(slice_rotate_type_horizontal_radio_);
					rotate_type_layout->addWidget(slice_rotate_type_vertical_radio_);

					controller_layout->addLayout(rotate_type_layout);
					controller_layout->addWidget(slice_angel_spin_);
					controller_layout->addWidget(slice_filter_spin_);
				}

				right_layout->addLayout(label_layout);
				right_layout->addLayout(controller_layout);
			}

			contents_layout->addLayout(left_layout);
			contents_layout->addWidget(CreateVerticalLine());
			contents_layout->addLayout(right_layout);
		}

		QLabel* title = CreateLabel(lang::LanguagePack::txt_pacs_mpr_default_setting_2d(), QSizePolicy::Expanding, QSizePolicy::Fixed);
		default_setting_2d_layout->addWidget(CreateHorizontalLine());
		default_setting_2d_layout->addWidget(title);
		default_setting_2d_layout->addLayout(contents_layout);
	}

	return default_setting_2d_layout;
}

QVBoxLayout* PreferencesPACSDefaultSettingWidget::CreateMPRDefaultSetting3D()
{
	QVBoxLayout* default_setting_3d_layout = new QVBoxLayout();
	{
		default_setting_3d_layout->setSpacing(kSpacing10);
		default_setting_3d_layout->setContentsMargins(kMarginZero);

		QHBoxLayout* contents_layout = new QHBoxLayout();
		{
			contents_layout->setContentsMargins(kStepMargins);
			contents_layout->setAlignment(Qt::AlignLeft);

			QVBoxLayout* label_layout = new QVBoxLayout();
			{
				label_layout->setSpacing(kSpacing10);

				QLabel* rotation_type_label = CreateLabel(lang::LanguagePack::txt_rotation_type() + " :");
				QLabel* rotation_dir_label = CreateLabel(lang::LanguagePack::txt_rotation_dir() + " :");
				QLabel* angle_label = CreateLabel(lang::LanguagePack::txt_angle() + " :");

				label_layout->addWidget(rotation_type_label);
				label_layout->addWidget(rotation_dir_label);
				label_layout->addWidget(angle_label);
			}

			QVBoxLayout* controller_layout = new QVBoxLayout();
			{
				controller_layout->setSpacing(kSpacing10);

				QButtonGroup* volume_rotate_dir_Anterior_radio_group = new QButtonGroup(this);
				volume_rotate_dir_Anterior_radio_ = CreateTextRadioBttton(lang::LanguagePack::txt_anterior(), QSizePolicy::Fixed);
				volume_rotate_dir_Posterior_radio_ = CreateTextRadioBttton(lang::LanguagePack::txt_posterior(), QSizePolicy::Fixed);

				volume_rotate_dir_Anterior_radio_group->addButton(volume_rotate_dir_Anterior_radio_);
				volume_rotate_dir_Anterior_radio_group->addButton(volume_rotate_dir_Posterior_radio_);
				volume_rotate_dir_Anterior_radio_->setChecked(true);

				QButtonGroup* volume_rotate_type_horizontal_radio_group = new QButtonGroup(this);
				volume_rotate_type_horizontal_radio_ = CreateTextRadioBttton(lang::LanguagePack::txt_horizontal(), QSizePolicy::Fixed);
				volume_slice_rotate_type_vertical_radio_ = CreateTextRadioBttton(lang::LanguagePack::txt_vertical(), QSizePolicy::Fixed);

				volume_rotate_type_horizontal_radio_group->addButton(volume_rotate_type_horizontal_radio_);
				volume_rotate_type_horizontal_radio_group->addButton(volume_slice_rotate_type_vertical_radio_);
				volume_rotate_type_horizontal_radio_->setChecked(true);

				volume_angel_spin_ = CreateSpinBox(1, 30, 1);

				QHBoxLayout* rotation_type_layout = new QHBoxLayout();
				QHBoxLayout* rotation_dir_layout = new QHBoxLayout();

				rotation_type_layout->addWidget(volume_rotate_dir_Anterior_radio_);
				rotation_type_layout->addWidget(volume_rotate_dir_Posterior_radio_);

				rotation_dir_layout->addWidget(volume_rotate_type_horizontal_radio_);
				rotation_dir_layout->addWidget(volume_slice_rotate_type_vertical_radio_);

				controller_layout->addLayout(rotation_type_layout);
				controller_layout->addLayout(rotation_dir_layout);
				controller_layout->addWidget(volume_angel_spin_);
			}

			contents_layout->addLayout(label_layout);
			contents_layout->addLayout(controller_layout);
		}

		QLabel* title = CreateLabel(lang::LanguagePack::txt_pacs_mpr_default_setting_3d(), QSizePolicy::Expanding, QSizePolicy::Fixed);
		default_setting_3d_layout->addWidget(CreateHorizontalLine());
		default_setting_3d_layout->addWidget(title);
		default_setting_3d_layout->addLayout(contents_layout);

	}
	return default_setting_3d_layout;
}

void PreferencesPACSDefaultSettingWidget::GetPACSValues()
{
	GlobalPreferences::PACSDefaultSetting& pacs_defalut_setting = GlobalPreferences::GetInstance()->preferences_.pacs_default_setting;

	//MPR 2D
	view_list_swap_check_->setChecked(pacs_defalut_setting.mpr_view_list_swap);
	slice_rotate_type_horizontal_radio_->setChecked(!pacs_defalut_setting.mpr_is_2d_vertical);
	slice_rotate_type_vertical_radio_->setChecked(pacs_defalut_setting.mpr_is_2d_vertical);
	slice_interval_spin_->setValue(pacs_defalut_setting.mpr_interval);
	slice_angel_spin_->setValue(pacs_defalut_setting.mpr_2d_angle);
	slice_thickness_spin_->setValue(pacs_defalut_setting.mpr_thickness);
	slice_filter_spin_->setValue(pacs_defalut_setting.mpr_filter_level);

	//MPR 3D
	volume_rotate_dir_Anterior_radio_->setChecked(!pacs_defalut_setting.mpr_is_dir_posterior);
	volume_rotate_dir_Posterior_radio_->setChecked(pacs_defalut_setting.mpr_is_dir_posterior);
	volume_rotate_type_horizontal_radio_->setChecked(!pacs_defalut_setting.mpr_is_3d_vertical);
	volume_slice_rotate_type_vertical_radio_->setChecked(pacs_defalut_setting.mpr_is_3d_vertical);
	volume_angel_spin_->setValue(pacs_defalut_setting.mpr_3d_angle);
}
