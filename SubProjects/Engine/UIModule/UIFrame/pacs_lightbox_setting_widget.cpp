#include "pacs_lightbox_setting_widget.h"

#include <QVBoxLayout>

#include "../../Common/Common/language_pack.h"

namespace
{
	const int kFilterMax = 3;
	const int kThicknessMax = 50;
}

PacsLightboxSettingWidget::PacsLightboxSettingWidget(QWidget* parent /*= 0*/)
	: BaseSettingWidget(parent)
{
	SetLayout();
}

PacsLightboxSettingWidget::~PacsLightboxSettingWidget()
{

}

void PacsLightboxSettingWidget::SetFilterValue(const int filter_level)
{
	if (filter_level < 0 || filter_level > kFilterMax)
	{
		return;
	}

	int id = static_cast<int>(LightboxSliderID::FILTER);
	SetSliderValue(id, filter_level);
}

void PacsLightboxSettingWidget::SetThicknessValue(const int thickness)
{
	if (thickness < 0 || thickness > kThicknessMax)
	{
		return;
	}

	int id = static_cast<int>(LightboxSliderID::THICKNESS);
	SetSliderValue(id, thickness);
}

const int PacsLightboxSettingWidget::GetFilterValue() const
{
	int id = static_cast<int>(LightboxSliderID::FILTER);
	return GetSliderValue(id);
}

const int PacsLightboxSettingWidget::GetThicknessValue() const
{
	int id = static_cast<int>(LightboxSliderID::THICKNESS);
	return GetSliderValue(id);
}

void PacsLightboxSettingWidget::SetLayout()
{
	main_layout()->addLayout(CreateSettingLayout());
}

QVBoxLayout* PacsLightboxSettingWidget::CreateSettingLayout()
{
	QVBoxLayout* setting_layout = new QVBoxLayout();
	{
		setting_layout->setSpacing(5);
		setting_layout->setContentsMargins(kMarginZero);
		setting_layout->setAlignment(Qt::AlignTop);

		int filter_id = static_cast<int>(LightboxSliderID::FILTER);
		int thickness_id = static_cast<int>(LightboxSliderID::THICKNESS);

		setting_layout->addLayout(CreateSliderLayout(filter_id, lang::LanguagePack::txt_filter(), 0, kFilterMax));
		setting_layout->addLayout(CreateSliderLayout(thickness_id, lang::LanguagePack::txt_thickness() + " : ", 0, kThicknessMax));
	}

	return setting_layout;
}
