#include "preferences_advanced_widget.h"

#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QToolButton>
#include <QSpinBox>
#include <QDoubleSpinBox>

#include "../../Common/Common/global_preferences.h"
#include "../../Common/Common/language_pack.h"
#include "../../Common/Common/W3Theme.h"

namespace
{
	//mpr
	const bool kHideMPRViewsOnMaximizedVRLayout = false;
	const bool kManualOrientation = true; // auto false
	const float kMPRDefaultInterval = 0.f; // 0 = pixel spacing

	//cross section
	const float kThicknessIncrements = 1.f;
	const float kIntervalIncrements = 1.f;
	const float kCrossSectionDefaultInterval = 1.f;
	const bool kFlipSlicesAcrossTheArchCenterLine = false;

	//panorama
	const float kPanoramaDefaultThickness = 10.f;
	const float kArchThicknessIncrements = 1.f;
	const float kPanoramaDefaultRange = 30.f;

	//implant
	const int kImplantTranslationIncrements = 1;
	const int kImplantRotationIncrements = 1;

	//volume rendering
	const int KQuality = 0; //index 0 = High

	//windowing
	const bool kUseFixedValue = false;
	const int kFixedLevel = 1000;
	const int kFixedWidth = 4000;
}

PreferencesAdvancedWidget::PreferencesAdvancedWidget(QWidget *parent /*= nullptr*/)
	:BaseWidget(parent)
{
	contents_layout()->addLayout(CreateAdvancedLayout());
}

PreferencesAdvancedWidget::~PreferencesAdvancedWidget()
{

}

void PreferencesAdvancedWidget::SetGlobalPreferences()
{
	GlobalPreferences::Advanced& advanced = GlobalPreferences::GetInstance()->preferences_.advanced;

	advanced.mpr.hide_mpr_views_on_maximized_vr_layout = mpr_hide_mpr_views_on_maximized_vr_layout_check_->isChecked();
	advanced.mpr.reorientation = mpr_reorientation_auto_radio_->isChecked() ? GlobalPreferences::Reorientation::Auto : GlobalPreferences::Reorientation::Manual;
	advanced.mpr.default_interval = mpr_default_interval_spin_->value();

	advanced.cross_section_view.thickness_increments = cross_section_view_thickness_increments_spin_->value();
	advanced.cross_section_view.interval_increments = cross_section_view_interval_increments_spin_->value();;
	advanced.cross_section_view.default_interval = cross_section_view_default_interval_spin_->value();
	advanced.cross_section_view.flip_slices_across_the_arch_centerline = cross_section_view_flip_slices_across_the_arch_centerline_check_->isChecked();

	advanced.panorama_axial_view.panorama_default_thickness = panorama_default_thickness_spin_->value();
	advanced.panorama_axial_view.panorama_thickness_increments = panorama_axial_view_arch_thickness_increments_spin_->value();
	advanced.panorama_axial_view.panorama_default_range = panorama_default_range_spin_->value();

	advanced.implant_view.translation_increments = implant_translation_increments_spin_->value();
	advanced.implant_view.rotation_increments = implant_rotation_increments_spin_->value();

	advanced.volume_rendering.quality = static_cast<GlobalPreferences::Quality2>(volume_rendering_quality_combo_->currentIndex());

	advanced.windowing.use_fixed_value = windowing_use_fixed_value_->isChecked();
	advanced.windowing.fixed_level = windowing_fixed_level_spin_->value();
	advanced.windowing.fixed_width = windowing_fixed_width_spin_->value();
}

void PreferencesAdvancedWidget::Reset()
{
	mpr_hide_mpr_views_on_maximized_vr_layout_check_->setChecked(kHideMPRViewsOnMaximizedVRLayout);
	mpr_reorientation_manual_radio_->setChecked(kManualOrientation);
	mpr_default_interval_spin_->setValue(kMPRDefaultInterval);

	cross_section_view_thickness_increments_spin_->setValue(kThicknessIncrements);
	cross_section_view_interval_increments_spin_->setValue(kIntervalIncrements);
	cross_section_view_default_interval_spin_->setValue(kCrossSectionDefaultInterval);
	cross_section_view_flip_slices_across_the_arch_centerline_check_->setChecked(kFlipSlicesAcrossTheArchCenterLine);

	panorama_default_thickness_spin_->setValue(kPanoramaDefaultThickness);
	panorama_axial_view_arch_thickness_increments_spin_->setValue(kArchThicknessIncrements);
	panorama_default_range_spin_->setValue(kPanoramaDefaultRange);

	implant_translation_increments_spin_->setValue(kImplantTranslationIncrements);
	implant_rotation_increments_spin_->setValue(kImplantRotationIncrements);

	volume_rendering_quality_combo_->setCurrentIndex(KQuality);

	windowing_use_fixed_value_->setChecked(kUseFixedValue);
	windowing_fixed_level_spin_->setValue(kFixedLevel);
	windowing_fixed_width_spin_->setValue(kFixedWidth);
}

QVBoxLayout* PreferencesAdvancedWidget::CreateAdvancedLayout()
{
	QVBoxLayout* layout = new QVBoxLayout();

	layout->setSpacing(kSpacing10);
	layout->setContentsMargins(kContentsMargins);

	layout->addLayout(CreateMPRLayout());
	layout->addLayout(CreateCrossSectionLayout());
	layout->addLayout(CreatePanoramaLayout());
	layout->addLayout(CreateImplantLayout());
	layout->addLayout(CreateVolumeRenderingLayout());
	layout->addLayout(CreateWindowingLayout());

	GetAdvancedValues();

	return layout;
}

QVBoxLayout* PreferencesAdvancedWidget::CreateMPRLayout()
{
	QVBoxLayout* mpr_layout = new QVBoxLayout();
	{
		QVBoxLayout* contents_layout = new QVBoxLayout();
		{
			contents_layout->setContentsMargins(kStepMargins);

			QHBoxLayout* reorientation_radio_layout = new QHBoxLayout();
			{
				reorientation_radio_layout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
				reorientation_radio_layout->setSpacing(kSpacing10);

				QLabel* reorientation_label = CreateLabel(lang::LanguagePack::txt_reorientation() + " :", QSizePolicy::Fixed, QSizePolicy::Fixed);

				QButtonGroup* reorientation_radio_group = new QButtonGroup(this);
				mpr_reorientation_auto_radio_ = CreateTextRadioBttton(lang::LanguagePack::txt_auto(), QSizePolicy::Fixed);
				mpr_reorientation_manual_radio_ = CreateTextRadioBttton(lang::LanguagePack::txt_manual(), QSizePolicy::Fixed);

				reorientation_radio_group->addButton(mpr_reorientation_auto_radio_);
				reorientation_radio_group->addButton(mpr_reorientation_manual_radio_);

				reorientation_radio_layout->addWidget(reorientation_label);
				reorientation_radio_layout->addWidget(mpr_reorientation_auto_radio_);
				reorientation_radio_layout->addWidget(mpr_reorientation_manual_radio_);
			}

			QHBoxLayout* spin_box_layout = new QHBoxLayout();
			{
				spin_box_layout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

				QLabel* default_interval_label = CreateLabel(lang::LanguagePack::txt_default_interval() + " :", QSizePolicy::Fixed, QSizePolicy::Fixed);

				mpr_default_interval_spin_ = CreateDoubleSpinBox(0.f, 10.f, GlobalPreferences::GetInstance()->preferences_.advanced.mpr.default_interval);

				spin_box_layout->addWidget(default_interval_label);
				spin_box_layout->addWidget(mpr_default_interval_spin_);
			}

			mpr_hide_mpr_views_on_maximized_vr_layout_check_ = new QCheckBox(lang::LanguagePack::txt_hide_mpr_views_on_maximized_vr_layout(), this);

			contents_layout->addWidget(mpr_hide_mpr_views_on_maximized_vr_layout_check_);
			contents_layout->addLayout(reorientation_radio_layout);
			contents_layout->addLayout(spin_box_layout);
		}

		QLabel* title = CreateLabel(lang::LanguagePack::txt_mpr(), QSizePolicy::Expanding, QSizePolicy::Fixed);
		mpr_layout->addWidget(CreateHorizontalLine());
		mpr_layout->addWidget(title);
		mpr_layout->addLayout(contents_layout);
	}

	return mpr_layout;
}

QVBoxLayout* PreferencesAdvancedWidget::CreateCrossSectionLayout()
{
	QVBoxLayout* cross_section_layout = new QVBoxLayout();
	{
		QVBoxLayout* contents_layout = new QVBoxLayout();
		{
			contents_layout->setContentsMargins(kStepMargins);

			QHBoxLayout* spin_box_layout = new QHBoxLayout();
			{
				spin_box_layout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

				QVBoxLayout* spin_box_label_layout = new QVBoxLayout();
				QVBoxLayout* spin_box_widget_layout = new QVBoxLayout();

				QLabel* thickness_increments_label = CreateLabel(lang::LanguagePack::txt_thickness_increments() + " :", QSizePolicy::Fixed, QSizePolicy::Fixed);
				QLabel* interval_increments_label = CreateLabel(lang::LanguagePack::txt_interval_increments() + " :", QSizePolicy::Fixed, QSizePolicy::Fixed);
				QLabel* default_interval_label = CreateLabel(lang::LanguagePack::txt_default_interval() + " :", QSizePolicy::Fixed, QSizePolicy::Fixed);

				cross_section_view_thickness_increments_spin_ = CreateDoubleSpinBox(1.f, 10.f, GlobalPreferences::GetInstance()->preferences_.advanced.cross_section_view.interval_increments);
				cross_section_view_interval_increments_spin_ = CreateDoubleSpinBox(0.5f, 10.f, GlobalPreferences::GetInstance()->preferences_.advanced.cross_section_view.interval_increments);
				cross_section_view_default_interval_spin_ = CreateDoubleSpinBox(1.f, 10.f, GlobalPreferences::GetInstance()->preferences_.advanced.cross_section_view.default_interval);

				spin_box_label_layout->addWidget(thickness_increments_label);
				spin_box_label_layout->addWidget(interval_increments_label);
				spin_box_label_layout->addWidget(default_interval_label);

				spin_box_widget_layout->addWidget(cross_section_view_thickness_increments_spin_);
				spin_box_widget_layout->addWidget(cross_section_view_interval_increments_spin_);
				spin_box_widget_layout->addWidget(cross_section_view_default_interval_spin_);

				spin_box_layout->addLayout(spin_box_label_layout);
				spin_box_layout->addLayout(spin_box_widget_layout);
			}

			cross_section_view_flip_slices_across_the_arch_centerline_check_ = new QCheckBox(lang::LanguagePack::txt_flip_slices_across_the_arch_centerline(), this);

			contents_layout->addLayout(spin_box_layout);
			contents_layout->addWidget(cross_section_view_flip_slices_across_the_arch_centerline_check_);
		}

		QLabel* title = CreateLabel(lang::LanguagePack::txt_cross_section_view(), QSizePolicy::Expanding, QSizePolicy::Fixed);

		cross_section_layout->addWidget(CreateHorizontalLine());
		cross_section_layout->addWidget(title);
		cross_section_layout->addLayout(contents_layout);
	}

	return cross_section_layout;
}

QVBoxLayout* PreferencesAdvancedWidget::CreatePanoramaLayout()
{
	QVBoxLayout* panorama_layout = new QVBoxLayout();
	{
		QHBoxLayout* contents_layout = new QHBoxLayout();
		{
			QVBoxLayout* label_layout = new QVBoxLayout();
			QVBoxLayout* spin_layout = new QVBoxLayout();

			QLabel* panorama_default_thickness_label = CreateLabel(lang::LanguagePack::txt_panorama_default_thickness() + " :", QSizePolicy::Fixed, QSizePolicy::Fixed);
			QLabel* thickness_increments_label = CreateLabel(lang::LanguagePack::txt_panorama_thickness_increments() + " :", QSizePolicy::Fixed, QSizePolicy::Fixed);
			QLabel* default_range_label = CreateLabel(lang::LanguagePack::txt_panorama() + " " + lang::LanguagePack::txt_default_range() + " :", QSizePolicy::Fixed, QSizePolicy::Fixed);

			panorama_default_thickness_spin_ = CreateDoubleSpinBox(0.f, 40.f, GlobalPreferences::GetInstance()->preferences_.advanced.panorama_axial_view.panorama_default_thickness);
			panorama_axial_view_arch_thickness_increments_spin_ = CreateDoubleSpinBox(1.f, 10.f, GlobalPreferences::GetInstance()->preferences_.advanced.panorama_axial_view.panorama_thickness_increments);
			panorama_default_range_spin_ = CreateDoubleSpinBox(1.f, 200.f, GlobalPreferences::GetInstance()->preferences_.advanced.panorama_axial_view.panorama_default_range);

			label_layout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
			label_layout->addWidget(panorama_default_thickness_label);
			label_layout->addWidget(thickness_increments_label);
			label_layout->addWidget(default_range_label);

			spin_layout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
			spin_layout->addWidget(panorama_default_thickness_spin_);
			spin_layout->addWidget(panorama_axial_view_arch_thickness_increments_spin_);
			spin_layout->addWidget(panorama_default_range_spin_);

			contents_layout->setContentsMargins(kStepMargins);
			contents_layout->addLayout(label_layout);
			contents_layout->addLayout(spin_layout);
		}

		QLabel* title = CreateLabel(lang::LanguagePack::txt_panorama_view(), QSizePolicy::Expanding, QSizePolicy::Fixed);

		panorama_layout->addWidget(CreateHorizontalLine());
		panorama_layout->addWidget(title);
		panorama_layout->addLayout(contents_layout);
	}

	return panorama_layout;
}

QVBoxLayout* PreferencesAdvancedWidget::CreateImplantLayout()
{
	QVBoxLayout* implant_layout = new QVBoxLayout();
	{
		QHBoxLayout* contents_layout = new QHBoxLayout();
		{
			QVBoxLayout* label_layout = new QVBoxLayout();
			QVBoxLayout* spin_layout = new QVBoxLayout();

			QLabel* translation_increments_label = CreateLabel(lang::LanguagePack::txt_translation_increments() + " :", QSizePolicy::Fixed, QSizePolicy::Fixed);
			QLabel* rotation_increments_label = CreateLabel(lang::LanguagePack::txt_rotation_increments() + " :", QSizePolicy::Fixed, QSizePolicy::Fixed);

			implant_translation_increments_spin_ = CreateSpinBox(1, 10, GlobalPreferences::GetInstance()->preferences_.advanced.implant_view.translation_increments);
			implant_rotation_increments_spin_ = CreateSpinBox(1, 30, GlobalPreferences::GetInstance()->preferences_.advanced.implant_view.rotation_increments);

			label_layout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
			label_layout->addWidget(translation_increments_label);
			label_layout->addWidget(rotation_increments_label);

			spin_layout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
			spin_layout->addWidget(implant_translation_increments_spin_);
			spin_layout->addWidget(implant_rotation_increments_spin_);

			contents_layout->setContentsMargins(kStepMargins);
			contents_layout->addLayout(label_layout);
			contents_layout->addLayout(spin_layout);
		}

		QLabel* title = CreateLabel(lang::LanguagePack::txt_implant(), QSizePolicy::Expanding, QSizePolicy::Fixed);

		implant_layout->addWidget(CreateHorizontalLine());
		implant_layout->addWidget(title);
		implant_layout->addLayout(contents_layout);
	}

	return implant_layout;
}

QVBoxLayout* PreferencesAdvancedWidget::CreateVolumeRenderingLayout()
{
	QVBoxLayout* volume_rendering_layout = new QVBoxLayout();
	{
		QHBoxLayout* contents_layout = new QHBoxLayout();
		{
			QLabel* quality_label = CreateLabel(lang::LanguagePack::txt_quality() + " :", QSizePolicy::Fixed, QSizePolicy::Fixed);
			volume_rendering_quality_combo_ = CreateComboBox(GlobalPreferences::GetInstance()->preferences_.advanced.volume_rendering.quality_string_list);

			contents_layout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
			contents_layout->setContentsMargins(kStepMargins);
			contents_layout->addWidget(quality_label);
			contents_layout->addWidget(volume_rendering_quality_combo_);
		}

		QLabel* title = CreateLabel(lang::LanguagePack::txt_volume_rendering(), QSizePolicy::Expanding, QSizePolicy::Fixed);

		volume_rendering_layout->addWidget(CreateHorizontalLine());
		volume_rendering_layout->addWidget(title);
		volume_rendering_layout->addLayout(contents_layout);
	}

	return volume_rendering_layout;
}

QVBoxLayout* PreferencesAdvancedWidget::CreateWindowingLayout()
{
	QVBoxLayout* windowing_layout = new QVBoxLayout();
	{
		QVBoxLayout* contents_layout = new QVBoxLayout();
		{
			QHBoxLayout* spin_box_layout = new QHBoxLayout();
			{
				QVBoxLayout* spin_box_label_layout = new QVBoxLayout();
				QVBoxLayout* spin_box_widget_layout = new QVBoxLayout();

				QLabel* fixed_level_label = CreateLabel( lang::LanguagePack::txt_window_center().toLower() + " :", QSizePolicy::Fixed, QSizePolicy::Fixed);
				QLabel* fixed_width_label = CreateLabel( lang::LanguagePack::txt_window_width().toLower() + " :", QSizePolicy::Fixed, QSizePolicy::Fixed);

				windowing_fixed_level_spin_ = CreateSpinBox(0, 65535, GlobalPreferences::GetInstance()->preferences_.advanced.windowing.fixed_level);
				windowing_fixed_width_spin_ = CreateSpinBox(1, 35768, GlobalPreferences::GetInstance()->preferences_.advanced.windowing.fixed_width);

				windowing_use_fixed_value_ = new QCheckBox(lang::LanguagePack::txt_use_fixed_value(), this);

				spin_box_label_layout->addWidget(fixed_level_label);
				spin_box_label_layout->addWidget(fixed_width_label);

				spin_box_widget_layout->addWidget(windowing_fixed_level_spin_);
				spin_box_widget_layout->addWidget(windowing_fixed_width_spin_);

				spin_box_layout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
				spin_box_layout->addLayout(spin_box_label_layout);
				spin_box_layout->addLayout(spin_box_widget_layout);
			}

			contents_layout->setContentsMargins(kStepMargins);
			contents_layout->addWidget(windowing_use_fixed_value_);
			contents_layout->addLayout(spin_box_layout);
		}

		QLabel* title = CreateLabel(lang::LanguagePack::txt_windowing(), QSizePolicy::Expanding, QSizePolicy::Fixed);

		windowing_layout->addWidget(CreateHorizontalLine());
		windowing_layout->addWidget(title);
		windowing_layout->addLayout(contents_layout);
	}

	return windowing_layout;
}

void PreferencesAdvancedWidget::GetAdvancedValues()
{
	GlobalPreferences::Advanced& advanced = GlobalPreferences::GetInstance()->preferences_.advanced;

	mpr_hide_mpr_views_on_maximized_vr_layout_check_->setChecked(advanced.mpr.hide_mpr_views_on_maximized_vr_layout);
	mpr_reorientation_auto_radio_->setChecked(advanced.mpr.reorientation == GlobalPreferences::Reorientation::Auto);
	mpr_reorientation_manual_radio_->setChecked(advanced.mpr.reorientation == GlobalPreferences::Reorientation::Manual);
	mpr_default_interval_spin_->setValue(advanced.mpr.default_interval);

	cross_section_view_thickness_increments_spin_->setValue(advanced.cross_section_view.thickness_increments);
	cross_section_view_interval_increments_spin_->setValue(advanced.cross_section_view.interval_increments);
	cross_section_view_default_interval_spin_->setValue(advanced.cross_section_view.default_interval);
	cross_section_view_flip_slices_across_the_arch_centerline_check_->setChecked(advanced.cross_section_view.flip_slices_across_the_arch_centerline);

	panorama_default_thickness_spin_->setValue(advanced.panorama_axial_view.panorama_default_thickness);
	panorama_axial_view_arch_thickness_increments_spin_->setValue(advanced.panorama_axial_view.panorama_thickness_increments);
	panorama_default_range_spin_->setValue(advanced.panorama_axial_view.panorama_default_range);

	implant_translation_increments_spin_->setValue(advanced.implant_view.translation_increments);
	implant_rotation_increments_spin_->setValue(advanced.implant_view.rotation_increments);

	int quality = static_cast<int>(advanced.volume_rendering.quality);
	volume_rendering_quality_combo_->setCurrentIndex((volume_rendering_quality_combo_->count() > quality) ? quality : 0);

	windowing_use_fixed_value_->setChecked(advanced.windowing.use_fixed_value);
	windowing_fixed_level_spin_->setValue(advanced.windowing.fixed_level);
	windowing_fixed_width_spin_->setValue(advanced.windowing.fixed_width);
}
