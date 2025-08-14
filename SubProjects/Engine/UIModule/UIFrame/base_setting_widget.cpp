#include "base_setting_widget.h"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QSlider>

#include "../../Common/Common/W3Theme.h"
#include "../../Common/Common/W3LayoutFunctions.h"
#include "../../Common/Common/W3Memory.h"

namespace
{
	const QString kLabel = "_Label";
}

BaseSettingWidget::BaseSettingWidget(QWidget* parent /*= 0*/)
	: QWidget(parent)
{
	Initialize();
}

BaseSettingWidget::~BaseSettingWidget()
{
	SAFE_DELETE_LATER(main_layout_);
	ClearMap();
}

QVBoxLayout* BaseSettingWidget::CreateSliderLayout(const int id, const QString& name, const int min, const int max, const int init_value /*= 0*/)
{
	if (CheckSlider(id) || min == max)
	{
		return nullptr;
	}

	QVBoxLayout* slider_layout = new QVBoxLayout();
	{
		QSlider* slider = new QSlider(this);

		QString slider_stylesheet = CW3Theme::getInstance()->appQSliderStyleSheet();
		slider->setStyleSheet(slider_stylesheet);
		slider->setContentsMargins(kMarginZero);
		slider->setOrientation(Qt::Horizontal);

		min < max ? slider->setRange(min, max) : slider->setRange(max, min);
		init_value == 0 ? slider->setValue(min) : init_value < max ? slider->setValue(init_value) : slider->setValue(max);
		
		slider_map_.insert(std::make_pair(id, slider));

		QLabel* name_label = new QLabel(this);
		name_label->setText(name);
		name_label->setAlignment(Qt::AlignLeft);

		QLabel* value_label = new QLabel(this);
		value_label->setText(QString::number(slider->value()));
		value_label->setAlignment(Qt::AlignRight);
		value_label->setObjectName(QString::number(id) + kLabel);

		QHBoxLayout* label_layout = new QHBoxLayout();
		label_layout->setSpacing(0);
		label_layout->setContentsMargins(kMarginZero);
		label_layout->addWidget(name_label);
		label_layout->addWidget(value_label);

		CW3Theme::toolVBarSizeInfo size_info = CW3Theme::getInstance()->getToolVBarSizeInfo();

		slider_layout->setSpacing(size_info.spacingS);
		slider_layout->setContentsMargins(kMarginZero);
		slider_layout->addLayout(label_layout);
		slider_layout->addWidget(slider);

		connect(slider, &QSlider::valueChanged, [=](const int value)
		{
			value_label->setText(QString::number(value));
			emit sigSliderUpdate(id, value);
		});
	}

	return slider_layout;
}

const int BaseSettingWidget::GetSliderValue(const int id) const
{
	std::map<int, QSlider*>::const_iterator iter = slider_map_.find(id);
	if (iter == slider_map_.end())
	{
		return -1;
	}

	return iter->second ? iter->second->value() : -1;
}

void BaseSettingWidget::SetSliderValue(const int id, const int value, bool signal /*= false*/)
{
	std::map<int, QSlider*>::iterator iter = slider_map_.find(id);
	if (iter == slider_map_.end() || iter->second == nullptr)
	{
		return;
	}

	if (signal)
	{
		iter->second->setValue(value);
	}
	else
	{
		iter->second->blockSignals(true);
		iter->second->setValue(value);
		iter->second->blockSignals(false);

		QLabel* value_lable = findChild<QLabel*>(QString::number(id) + kLabel);
		if (value_lable)
		{
			value_lable->setText(QString::number(value));
		}
	}
}

void BaseSettingWidget::Initialize()
{
	main_layout_ = new QVBoxLayout();
	main_layout_->setContentsMargins(kMarginZero);
	main_layout_->setSpacing(0);

	setLayout(main_layout_);
}

void BaseSettingWidget::ClearMap()
{
	std::map<int, QSlider*>::iterator iter = slider_map_.begin();
	std::map<int, QSlider*>::iterator iter_end = slider_map_.end();

	for (; iter != iter_end; ++iter)
	{
		SAFE_DELETE_LATER(iter->second);
	}

	slider_map_.clear();
}

bool BaseSettingWidget::CheckSlider(const int id) const
{
	if (slider_map_.find(id) == slider_map_.end())
	{
		return false;
	}
	return true;
}
