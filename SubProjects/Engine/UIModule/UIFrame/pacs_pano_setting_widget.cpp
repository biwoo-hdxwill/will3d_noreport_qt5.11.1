#include "pacs_pano_setting_widget.h"

#include <QVBoxLayout>

#include "../../Common/Common/language_pack.h"

namespace
{
	const int kFilterMax = 3;
	const int kThicknessMax = 20;
}

PacsPanoSettingWidget::PacsPanoSettingWidget(QWidget* parent /*= 0*/)
	: BaseSettingWidget(parent)
{
	SetLayout();
	Connection();
}

PacsPanoSettingWidget::~PacsPanoSettingWidget()
{
	
}

void PacsPanoSettingWidget::RestoreSliderValue()
{
	SetSliderValue(static_cast<int>(PanoSliderID::FILTER), prev_filter_);
	SetSliderValue(static_cast<int>(PanoSliderID::THICKNESS), prev_thickness_);
}

void PacsPanoSettingWidget::SetFilterValue(const int filter_level)
{
	if (filter_level < 0 || filter_level > kFilterMax)
	{
		return;
	}

	int id = static_cast<int>(PanoSliderID::FILTER);
	prev_thickness_ = GetSliderValue(id);
	SetSliderValue(id, filter_level);
}

void PacsPanoSettingWidget::SetThicknessValue(const int thickness)
{
	if (thickness < 0 || thickness > kThicknessMax)
	{
		return;
	}

	int id = static_cast<int>(PanoSliderID::THICKNESS);
	prev_thickness_ = GetSliderValue(id);
	SetSliderValue(id, thickness);
}

const int PacsPanoSettingWidget::GetFilterValue() const
{
	int id = static_cast<int>(PanoSliderID::FILTER);
	return GetSliderValue(id);
}

const int PacsPanoSettingWidget::GetThicknessValue() const
{
	int id = static_cast<int>(PanoSliderID::THICKNESS);
	return GetSliderValue(id);
}

void PacsPanoSettingWidget::SetLayout()
{
	main_layout()->addLayout(CreateSettingLayout());
}

void PacsPanoSettingWidget::Connection()
{
	connect(this, &BaseSettingWidget::sigSliderUpdate, [=](const int id, const int value)
	{
		if (id == static_cast<int>(PanoSliderID::FILTER))
		{
			sigUpdateFilter(value);
		}
		else if (id == static_cast<int>(PanoSliderID::THICKNESS))
		{
			sigUpdateThickness(value);
		}
	});
}

QVBoxLayout* PacsPanoSettingWidget::CreateSettingLayout()
{
	QVBoxLayout* setting_layout = new QVBoxLayout();
	{
		setting_layout->setSpacing(5);
		setting_layout->setContentsMargins(kMarginZero);
		setting_layout->setAlignment(Qt::AlignTop);

		int filter_id = static_cast<int>(PanoSliderID::FILTER);
		int thickness_id = static_cast<int>(PanoSliderID::THICKNESS);

		setting_layout->addLayout(CreateSliderLayout(filter_id, lang::LanguagePack::txt_filter(), 0, kFilterMax));
		setting_layout->addLayout(CreateSliderLayout(thickness_id, lang::LanguagePack::txt_thickness() + " : ", 0, kThicknessMax));
	}

	return setting_layout;
}
