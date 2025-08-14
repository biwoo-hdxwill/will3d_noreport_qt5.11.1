#include "preferences_objects_widget.h"

#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QToolButton>
#include <QDoubleSpinBox>

#include "../../Common/Common/color_dialog.h"
#include "../../Common/Common/global_preferences.h"
#include "../../Common/Common/language_pack.h"
#include "../../Common/Common/W3Theme.h"
#include "../../Common/Common/W3Memory.h"

namespace
{
	//MEASURE
	const QColor kLineColor = QColor("#00ff00");
	const QColor kTextColor = QColor("#ffffff");
	const int kTextSizeIndex = 0;

	//NERVE
	const float kNerveDiameter = 2.f;
	const QColor kNerveColor = QColor("#ff0000");

	//IMPLANT
	const QColor kImplantColor = QColor("#40b1ff");
	const QColor kImplantColorSelectd = QColor("#21ffff");
	const QColor kImplantColorCollided = QColor("#ff0000");
	const float kAlpha = 0.4f;
	const float kCollisionMargin = 2;
}

PreferencesObjectsWidget::PreferencesObjectsWidget(QWidget *parent /*= nullptr*/)
	:BaseWidget(parent)
{
	contents_layout()->addLayout(CreateObjectsLayout());
}

PreferencesObjectsWidget::~PreferencesObjectsWidget()
{

}

void PreferencesObjectsWidget::SetGlobalPreferences()
{
	GlobalPreferences::Preferences *preferences = &GlobalPreferences::GetInstance()->preferences_;

	preferences->objects.measure.line_color = measure_line_color_;
	preferences->objects.measure.text_color = measure_text_color_;
	preferences->objects.measure.text_size = static_cast<GlobalPreferences::Size>(measure_text_size_combo_->currentIndex());
	preferences->objects.measure.tape_line_multi_label = measure_tape_line_multi_check_->isChecked();

	preferences->objects.nerve.default_diameter = nerve_default_diameter_spin_->value();
	preferences->objects.nerve.default_color = nerve_default_color_;

#ifndef WILL3D_VIEWER
	preferences->objects.implant.rendering_type = implant_rendering_type_volume_radio_->isChecked() ? GlobalPreferences::MeshRenderingType::Volume : GlobalPreferences::MeshRenderingType::Wire;
	preferences->objects.implant.default_color_volume = implant_default_color_volume_;
	preferences->objects.implant.default_color_wire = implant_default_color_wire_;
	preferences->objects.implant.selected_color_volume = implant_selected_color_volume_;
	preferences->objects.implant.selected_color_wire = implant_selected_color_wire_;
	preferences->objects.implant.collided_color_volume = implant_collided_color_volume_;
	preferences->objects.implant.collided_color_wire = implant_collided_color_wire_;
	preferences->objects.implant.alpha = implant_alpha_spin_->value();
	preferences->objects.implant.collision_margin = implant_collision_margin_spin_->value();
	preferences->objects.implant.collision_margin_visible_on_2d_views = implant_margin_visible_on_2d_views_check_->isChecked();
	preferences->objects.implant.collision_margin_visible_on_3d_views = implant_margin_visible_on_3d_views_check_->isChecked();
	preferences->objects.implant.always_show_implant_id = implant_always_show_implant_id_check_->isChecked();
#endif
}

void PreferencesObjectsWidget::Reset()
{
	const float kFreeDrawLineWidth = 2.f;

	//measure
	measure_line_color_ = kLineColor;
	measure_text_color_ = kTextColor;

	measure_line_color_button_->setStyleSheet(CW3Theme::getInstance()->ColoredToolButtonStyleSheet(measure_line_color_));
	measure_text_color_button_->setStyleSheet(CW3Theme::getInstance()->ColoredToolButtonStyleSheet(measure_text_color_));

	measure_text_size_combo_->setCurrentIndex(kTextSizeIndex);

	//nerve
	nerve_default_diameter_spin_->setValue(kNerveDiameter);
	nerve_default_color_ = kNerveColor;
	nerve_default_color_button_->setStyleSheet(CW3Theme::getInstance()->ColoredToolButtonStyleSheet(nerve_default_color_));

#ifndef WILL3D_VIEWER
	//implant
	implant_rendering_type_volume_radio_->setChecked(true);

	implant_default_color_volume_ = kImplantColor;
	implant_selected_color_volume_ = kImplantColorSelectd;
	implant_collided_color_volume_ = kImplantColorCollided;
	implant_default_color_wire_ = kImplantColor;
	implant_selected_color_wire_ = kImplantColorSelectd;
	implant_collided_color_wire_ = kImplantColorCollided;

	implant_default_color_volume_button_->setStyleSheet(CW3Theme::getInstance()->ColoredToolButtonStyleSheet(implant_default_color_volume_));
	implant_selected_color_volume_button_->setStyleSheet(CW3Theme::getInstance()->ColoredToolButtonStyleSheet(implant_selected_color_volume_));
	implant_collided_color_volume_button_->setStyleSheet(CW3Theme::getInstance()->ColoredToolButtonStyleSheet(implant_collided_color_volume_));
	implant_default_color_wire_button_->setStyleSheet(CW3Theme::getInstance()->ColoredToolButtonStyleSheet(implant_default_color_wire_));
	implant_selected_color_wire_button_->setStyleSheet(CW3Theme::getInstance()->ColoredToolButtonStyleSheet(implant_selected_color_wire_));
	implant_collided_color_wire_button_->setStyleSheet(CW3Theme::getInstance()->ColoredToolButtonStyleSheet(implant_collided_color_wire_));

	implant_alpha_spin_->setValue(kAlpha);
	implant_collision_margin_spin_->setValue(kCollisionMargin);
	implant_margin_visible_on_2d_views_check_->setChecked(false);
	implant_margin_visible_on_3d_views_check_->setChecked(true);
	implant_always_show_implant_id_check_->setChecked(true);
#endif
}

QVBoxLayout* PreferencesObjectsWidget::CreateObjectsLayout()
{
	QVBoxLayout* layout = new QVBoxLayout();

	layout->setAlignment(Qt::AlignTop | Qt::AlignLeft);
	layout->setSpacing(10);
	layout->setContentsMargins(kContentsMargins);

	layout->addWidget(CreateHorizontalLine());
	layout->addLayout(CreateMeasureLayout());
	layout->addWidget(CreateHorizontalLine());
	layout->addLayout(CreateNerveLayout());
#ifndef WILL3D_VIEWER
	layout->addWidget(CreateHorizontalLine());
	layout->addLayout(CreateImplantLayout());
#endif

	GetObjectsValues();

	return layout;
}

QVBoxLayout* PreferencesObjectsWidget::CreateMeasureLayout()
{
	QVBoxLayout* measure_layout = new QVBoxLayout();
	{
		QLabel* measure_title = CreateLabel(lang::LanguagePack::txt_measure(), QSizePolicy::Expanding, QSizePolicy::Fixed);

		QGridLayout* measure_grid_layout = new QGridLayout();
		{
			QHBoxLayout* measure_text_size_layout = new QHBoxLayout();
			{
				measure_text_size_combo_ = CreateComboBox(GlobalPreferences::GetInstance()->preferences_.objects.measure.text_size_string_list);

				QLabel* measure_text_size_label = CreateLabel(lang::LanguagePack::txt_text_size() + " :", QSizePolicy::Preferred, QSizePolicy::Fixed);

				measure_text_size_layout->addWidget(measure_text_size_label);
				measure_text_size_layout->addWidget(measure_text_size_combo_);
			}

			measure_line_color_button_ = CreateColoredToolButton(measure_line_color_);
			measure_text_color_button_ = CreateColoredToolButton(measure_text_color_);

			measure_grid_layout->setContentsMargins(kStepMargins);
			measure_grid_layout->setHorizontalSpacing(20);
			measure_grid_layout->addLayout(CreateMeasureColorLayout(lang::LanguagePack::txt_line_color() + " :", measure_line_color_button_), 0, 0);
			measure_grid_layout->addLayout(CreateMeasureColorLayout(lang::LanguagePack::txt_text_color() + " :", measure_text_color_button_), 1, 0);
			measure_grid_layout->addLayout(measure_text_size_layout, 1, 1);

			measure_grid_layout->setColumnStretch(0, 2);
			measure_grid_layout->setColumnStretch(1, 2);
			measure_grid_layout->setColumnStretch(2, 1);
		}

		QHBoxLayout* measure_tape_line_multi_layout = new QHBoxLayout();
		{
			measure_tape_line_multi_layout->setContentsMargins(kStepMargins);
			measure_tape_line_multi_layout->setSpacing(kSpacing10);

			QLabel* measure_tape_line_multi_label = CreateLabel(lang::LanguagePack::txt_tape_line_multi_label(), QSizePolicy::Fixed, QSizePolicy::Fixed);

			measure_tape_line_multi_check_ = new QCheckBox(this);

			measure_tape_line_multi_layout->addWidget(measure_tape_line_multi_label);
			measure_tape_line_multi_layout->addWidget(measure_tape_line_multi_check_);
		}

		measure_layout->addWidget(measure_title);
		measure_layout->addLayout(measure_grid_layout);
		measure_layout->addLayout(measure_tape_line_multi_layout);

		connect(measure_line_color_button_, &QToolButton::clicked, [=]() { ChangeBtnColor(measure_line_color_, measure_line_color_button_); });
		connect(measure_text_color_button_, &QToolButton::clicked, [=]() { ChangeBtnColor(measure_text_color_, measure_text_color_button_); });
	}

	return measure_layout;
}

QVBoxLayout* PreferencesObjectsWidget::CreateNerveLayout()
{
	QVBoxLayout* nerve_layout = new QVBoxLayout();
	{
		QLabel* nerve_title = CreateLabel(lang::LanguagePack::txt_nerve(), QSizePolicy::Expanding, QSizePolicy::Fixed);
		QGridLayout* nerve_grid_layout = new QGridLayout();
		{
			QLabel* nerve_default_color_label = CreateLabel(lang::LanguagePack::txt_default_color() + " :");
			QLabel* nerve_default_diameter_label = CreateLabel(lang::LanguagePack::txt_default_diameter() + " :");

			nerve_default_color_button_ = CreateColoredToolButton(nerve_default_color_);
			nerve_default_diameter_spin_ = CreateDoubleSpinBox(1.f, 20.f, GlobalPreferences::GetInstance()->preferences_.objects.nerve.default_diameter);

			nerve_grid_layout->addWidget(nerve_default_diameter_label, 0, 0);
			nerve_grid_layout->addWidget(nerve_default_color_label, 1, 0);
			nerve_grid_layout->addWidget(nerve_default_diameter_spin_, 0, 1);
			nerve_grid_layout->addWidget(nerve_default_color_button_, 1, 1, Qt::AlignRight | Qt::AlignVCenter);

			nerve_grid_layout->setColumnStretch(0, 1);
			nerve_grid_layout->setColumnStretch(1, 1);
			nerve_grid_layout->setColumnStretch(2, 1);

			nerve_grid_layout->setContentsMargins(kStepMargins);
		}

		nerve_layout->addWidget(nerve_title);
		nerve_layout->addLayout(nerve_grid_layout);

		connect(nerve_default_color_button_, &QToolButton::clicked, [=]() { ChangeBtnColor(nerve_default_color_, nerve_default_color_button_); });
	}

	return nerve_layout;
}

#ifndef WILL3D_VIEWER
QVBoxLayout* PreferencesObjectsWidget::CreateImplantLayout()
{
	QVBoxLayout* implant_layout = new QVBoxLayout();
	{
		QLabel* title = CreateLabel(lang::LanguagePack::txt_implant(), QSizePolicy::Expanding, QSizePolicy::Fixed);
		QVBoxLayout* contents_layout = new QVBoxLayout();
		{
			QHBoxLayout* rendering_type_radio_layout = new QHBoxLayout();
			{
				rendering_type_radio_layout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
				rendering_type_radio_layout->setSpacing(kSpacing10);

				QLabel* implant_rendering_type_label = CreateLabel(lang::LanguagePack::txt_rendering_type() + " :", QSizePolicy::Fixed, QSizePolicy::Fixed);

				implant_rendering_type_volume_radio_ = CreateTextRadioBttton(lang::LanguagePack::txt_volume(), QSizePolicy::Fixed, QSizePolicy::Fixed);
				implant_rendering_type_wire_radio_ = CreateTextRadioBttton(lang::LanguagePack::txt_wire(), QSizePolicy::Fixed, QSizePolicy::Fixed);

				QButtonGroup* rendering_type_radio_group = new QButtonGroup(this);
				rendering_type_radio_group->addButton(implant_rendering_type_volume_radio_);
				rendering_type_radio_group->addButton(implant_rendering_type_wire_radio_);

				rendering_type_radio_layout->addWidget(implant_rendering_type_label);
				rendering_type_radio_layout->addWidget(implant_rendering_type_volume_radio_);
				rendering_type_radio_layout->addWidget(implant_rendering_type_wire_radio_);
			}

			QGridLayout* color_layout = new QGridLayout();
			{
				color_layout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
				color_layout->setSpacing(kSpacing10);

				QLabel* color_volume_label = CreateLabel(lang::LanguagePack::txt_volume(), QSizePolicy::Fixed, QSizePolicy::Fixed);
				QLabel* color_wire_label = CreateLabel(lang::LanguagePack::txt_wire(), QSizePolicy::Fixed, QSizePolicy::Fixed);
				QLabel* default_color_label = CreateLabel(lang::LanguagePack::txt_default_color() + " :", QSizePolicy::Fixed, QSizePolicy::Fixed);
				QLabel* selected_color_label = CreateLabel(lang::LanguagePack::txt_selected_color() + " :", QSizePolicy::Fixed, QSizePolicy::Fixed);
				QLabel* collided_color_label = CreateLabel(lang::LanguagePack::txt_collided_color() + " :", QSizePolicy::Fixed, QSizePolicy::Fixed);
				QLabel* alpha_label = CreateLabel(lang::LanguagePack::txt_alpha() + " :", QSizePolicy::Fixed, QSizePolicy::Fixed);

				implant_default_color_volume_button_ = CreateColoredToolButton(implant_default_color_volume_);
				implant_default_color_wire_button_ = CreateColoredToolButton(implant_default_color_wire_);
				implant_selected_color_volume_button_ = CreateColoredToolButton(implant_selected_color_volume_);
				implant_selected_color_wire_button_ = CreateColoredToolButton(implant_selected_color_wire_);
				implant_collided_color_volume_button_ = CreateColoredToolButton(implant_collided_color_volume_);
				implant_collided_color_wire_button_ = CreateColoredToolButton(implant_collided_color_wire_);

				implant_alpha_spin_ = CreateDoubleSpinBox(0.1f, 1.f, GlobalPreferences::GetInstance()->preferences_.objects.implant.alpha);
				implant_alpha_spin_->setSingleStep(0.1);

				color_layout->addWidget(color_volume_label, 0, 1, Qt::AlignCenter);
				color_layout->addWidget(color_wire_label, 0, 2, Qt::AlignCenter);
				color_layout->addWidget(default_color_label, 1, 0);
				color_layout->addWidget(selected_color_label, 2, 0);
				color_layout->addWidget(collided_color_label, 3, 0);
				color_layout->addWidget(alpha_label, 4, 0);
				color_layout->addWidget(implant_default_color_volume_button_, 1, 1);
				color_layout->addWidget(implant_default_color_wire_button_, 1, 2);
				color_layout->addWidget(implant_selected_color_volume_button_, 2, 1);
				color_layout->addWidget(implant_selected_color_wire_button_, 2, 2);
				color_layout->addWidget(implant_collided_color_volume_button_, 3, 1);
				color_layout->addWidget(implant_collided_color_wire_button_, 3, 2);
				color_layout->addWidget(implant_alpha_spin_, 4, 1);

				connect(implant_default_color_volume_button_, &QToolButton::clicked, [=]() { ChangeBtnColor(implant_default_color_volume_, implant_default_color_volume_button_); });
				connect(implant_default_color_wire_button_, &QToolButton::clicked, [=]() { ChangeBtnColor(implant_default_color_wire_, implant_default_color_wire_button_); });
				connect(implant_selected_color_volume_button_, &QToolButton::clicked, [=]() { ChangeBtnColor(implant_selected_color_volume_, implant_selected_color_volume_button_); });
				connect(implant_selected_color_wire_button_, &QToolButton::clicked, [=]() { ChangeBtnColor(implant_selected_color_wire_, implant_selected_color_wire_button_); });
				connect(implant_collided_color_volume_button_, &QToolButton::clicked, [=]() { ChangeBtnColor(implant_collided_color_volume_, implant_collided_color_volume_button_); });
				connect(implant_collided_color_wire_button_, &QToolButton::clicked, [=]() { ChangeBtnColor(implant_collided_color_wire_, implant_collided_color_wire_button_); });
			}

			QHBoxLayout* collision_margin_layout = new QHBoxLayout();
			{
				collision_margin_layout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
				collision_margin_layout->setSpacing(kSpacing10);

				QLabel* collision_margin_label = CreateLabel(lang::LanguagePack::txt_collision_margin() + " :", QSizePolicy::Fixed, QSizePolicy::Fixed);

				implant_collision_margin_spin_ = CreateDoubleSpinBox(0.f, 10.f, GlobalPreferences::GetInstance()->preferences_.objects.implant.collision_margin);

				collision_margin_layout->addWidget(collision_margin_label);
				collision_margin_layout->addWidget(implant_collision_margin_spin_);
			}

			QHBoxLayout* collision_margin_visible_on_layout = new QHBoxLayout();
			{
				collision_margin_visible_on_layout->setSpacing(kSpacing10);

				QLabel* collision_margin_visible_on_label = CreateLabel(lang::LanguagePack::txt_collision_margin_visible_on() + " :", QSizePolicy::Fixed, QSizePolicy::Fixed);

				implant_margin_visible_on_2d_views_check_ = new QCheckBox(lang::LanguagePack::txt_2d_views(), this);
				implant_margin_visible_on_3d_views_check_ = new QCheckBox(lang::LanguagePack::txt_3d_views(), this);

				collision_margin_visible_on_layout->addWidget(collision_margin_visible_on_label);
				collision_margin_visible_on_layout->addWidget(implant_margin_visible_on_2d_views_check_);
				collision_margin_visible_on_layout->addWidget(implant_margin_visible_on_3d_views_check_);
			}

			QHBoxLayout* library_presets_layout = new QHBoxLayout();
			{
				library_presets_layout->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
				library_presets_layout->setSpacing(kSpacing10);

				QVBoxLayout* library_presets_label_layout = new QVBoxLayout();
				QVBoxLayout* library_presets_widget_layout = new QVBoxLayout();

				QLabel* library_label = CreateLabel(lang::LanguagePack::txt_implant_library() + " :", QSizePolicy::Fixed, QSizePolicy::Fixed);
				QLabel* presets_label = CreateLabel(lang::LanguagePack::txt_implant_presets() + " :", QSizePolicy::Fixed, QSizePolicy::Fixed);

				QToolButton* library_button = CreateTextToolButton(lang::LanguagePack::txt_manage());
				QToolButton* presets_button = CreateTextToolButton(lang::LanguagePack::txt_manage());

				library_presets_label_layout->addWidget(library_label);
				library_presets_label_layout->addWidget(presets_label);

				library_presets_widget_layout->addWidget(library_button);
				library_presets_widget_layout->addWidget(presets_button);

				library_presets_layout->addLayout(library_presets_label_layout);
				library_presets_layout->addLayout(library_presets_widget_layout);

				connect(library_button, &QToolButton::clicked, [=]() { emit sigImplantLibraryClicked(); });
				connect(presets_button, &QToolButton::clicked, [=]() { emit sigImplantPresetsClicked(); });
			}

			implant_always_show_implant_id_check_ = new QCheckBox(lang::LanguagePack::txt_always_show_implant_id(), this);

			contents_layout->setContentsMargins(kStepMargins);
			contents_layout->addLayout(rendering_type_radio_layout);
			contents_layout->addLayout(color_layout);
			contents_layout->addLayout(collision_margin_layout);
			contents_layout->addLayout(collision_margin_visible_on_layout);
			contents_layout->addWidget(implant_always_show_implant_id_check_);
			contents_layout->addLayout(library_presets_layout);
		}

		implant_layout->addWidget(title);
		implant_layout->addLayout(contents_layout);
	}

	return implant_layout;
}
#endif

QHBoxLayout* PreferencesObjectsWidget::CreateMeasureColorLayout(const QString& text, QToolButton* button)
{
	if (button == nullptr)
	{
		return nullptr;
	}

	QHBoxLayout* layout = new QHBoxLayout();
	{
		QLabel* label = CreateLabel(text, QSizePolicy::Preferred, QSizePolicy::Fixed);

		layout->addWidget(label);
		layout->addWidget(button);
	}
	return layout;
}

void PreferencesObjectsWidget::GetObjectsValues()
{
	GlobalPreferences::Objects& objects = GlobalPreferences::GetInstance()->preferences_.objects;

	measure_line_color_ = objects.measure.line_color;
	measure_line_color_button_->setStyleSheet(CW3Theme::getInstance()->ColoredToolButtonStyleSheet(measure_line_color_));
	measure_text_color_ = objects.measure.text_color;
	measure_text_color_button_->setStyleSheet(CW3Theme::getInstance()->ColoredToolButtonStyleSheet(measure_text_color_));
	int text_size = static_cast<int>(objects.measure.text_size);
	measure_text_size_combo_->setCurrentIndex((measure_text_size_combo_->count() > text_size) ? text_size : 0);
	measure_tape_line_multi_check_->setChecked(objects.measure.tape_line_multi_label);
		
	nerve_default_diameter_spin_->setValue(objects.nerve.default_diameter);
	nerve_default_color_ = objects.nerve.default_color;
	nerve_default_color_button_->setStyleSheet(CW3Theme::getInstance()->ColoredToolButtonStyleSheet(nerve_default_color_));

#ifndef WILL3D_VIEWER
	implant_rendering_type_volume_radio_->setChecked(objects.implant.rendering_type == GlobalPreferences::MeshRenderingType::Volume);
	implant_rendering_type_wire_radio_->setChecked(objects.implant.rendering_type == GlobalPreferences::MeshRenderingType::Wire);
	implant_alpha_spin_->setValue(objects.implant.alpha);
	implant_default_color_volume_ = objects.implant.default_color_volume;
	implant_default_color_volume_button_->setStyleSheet(CW3Theme::getInstance()->ColoredToolButtonStyleSheet(implant_default_color_volume_));
	implant_default_color_wire_ = objects.implant.default_color_wire;
	implant_default_color_wire_button_->setStyleSheet(CW3Theme::getInstance()->ColoredToolButtonStyleSheet(implant_default_color_wire_));
	implant_selected_color_volume_ = objects.implant.selected_color_volume;
	implant_selected_color_volume_button_->setStyleSheet(CW3Theme::getInstance()->ColoredToolButtonStyleSheet(implant_selected_color_volume_));
	implant_selected_color_wire_ = objects.implant.selected_color_wire;
	implant_selected_color_wire_button_->setStyleSheet(CW3Theme::getInstance()->ColoredToolButtonStyleSheet(implant_selected_color_wire_));
	implant_collided_color_volume_ = objects.implant.collided_color_volume;
	implant_collided_color_volume_button_->setStyleSheet(CW3Theme::getInstance()->ColoredToolButtonStyleSheet(implant_collided_color_volume_));
	implant_collided_color_wire_ = objects.implant.collided_color_wire;
	implant_collided_color_wire_button_->setStyleSheet(CW3Theme::getInstance()->ColoredToolButtonStyleSheet(implant_collided_color_wire_));
	implant_collision_margin_spin_->setValue(objects.implant.collision_margin);
	implant_margin_visible_on_2d_views_check_->setChecked(objects.implant.collision_margin_visible_on_2d_views);
	implant_margin_visible_on_3d_views_check_->setChecked(objects.implant.collision_margin_visible_on_3d_views);
	implant_always_show_implant_id_check_->setChecked(objects.implant.always_show_implant_id);
#endif
}

void PreferencesObjectsWidget::ChangeBtnColor(QColor& out_color, QToolButton* dest_btn)
{
	if (dest_btn == nullptr)
	{
		return;
	}

	ColorDialog color_dialog;
	color_dialog.SetCurrentColor(out_color);
	if (!color_dialog.exec())
	{
		return;
	}

	QColor new_color = color_dialog.SelectedColor();
	if (!new_color.isValid())
	{
		return;
	}

	out_color = new_color;
	dest_btn->setStyleSheet(CW3Theme::getInstance()->ColoredToolButtonStyleSheet(out_color));
}
