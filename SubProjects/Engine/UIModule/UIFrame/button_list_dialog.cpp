#include "button_list_dialog.h"

#include <QVBoxLayout>
#include <QButtonGroup>
#include <QToolButton>
#include <QKeyEvent>

#include "../../Common/Common/language_pack.h"
#include "../../Common/Common/W3Theme.h"

#include "../../Resource/Resource/implant_resource.h"
#include "../../Resource/ResContainer/resource_container.h"


ButtonListDialog::ButtonListDialog(QWidget* parent /*= nullptr*/)
	: QDialog(parent)
{
	setWindowFlags(Qt::Popup);

	main_layout_ = new QVBoxLayout();
	main_layout_->setContentsMargins(QMargins(2, 2, 2, 2));
	main_layout_->setSpacing(0);

	setLayout(main_layout_);

	SetButtonList();
}

ButtonListDialog::~ButtonListDialog()
{
	if (main_layout_)
	{
		main_layout_->deleteLater();
	}

	if (button_group_)
	{
		QList<QAbstractButton*> buttons = button_group_->buttons();
		int size = buttons.size();
		for (int i = 0; i < size; ++i)
		{
			buttons.at(i)->deleteLater();
		}
		button_group_->deleteLater();
	}
}

void ButtonListDialog::InitButtonSetting(const TabType& type, const int mpr_type /*= -1*/)
{
	AllVisibleButton();

	/*if (mpr_type == -1)
	{
		SetVisibleButton(static_cast<int>(ButtonID::BTN_LIGHTBOX), false);
	}*/

	if (type != TabType::TAB_MPR)
	{
		SetVisibleButton(static_cast<int>(ButtonID::BTN_STL_EXPORT), false);
	}

	if (type != TabType::TAB_IMPLANT)
	{
		SetVisibleButton(static_cast<int>(ButtonID::BTN_IMPALNT_ANGLE), false);
	}
	else
	{
		const auto& implant_resource = ResourceContainer::GetInstance()->GetImplantResource();
		if (!implant_resource.IsSetImplant() || implant_resource.data().size() <= 1)
		{
			SetVisibleButton(static_cast<int>(ButtonID::BTN_IMPALNT_ANGLE), false);
		}
	}

	if (type == TabType::TAB_FACESIM)
	{
		SetVisibleButton(static_cast<int>(ButtonID::BTN_HIDE_UI), false);
		SetVisibleButton(static_cast<int>(ButtonID::BTN_GRID_ONOFF), false);
		SetVisibleButton(static_cast<int>(ButtonID::BTN_WINDOWING), false);
	}
	else if (type == TabType::TAB_ENDO)
	{
		SetVisibleButton(static_cast<int>(ButtonID::BTN_WINDOWING), false);
		SetVisibleButton(static_cast<int>(ButtonID::BTN_MEASURE_LIST), false);
	}
}

void ButtonListDialog::SetCheckedButton(const int id, const bool checked)
{
	if (button_group_ == nullptr || button_group_->buttons().empty())
	{
		return;
	}

	int min_num = static_cast<int>(ButtonID::BTN_WINDOWING);
	int max_num = static_cast<int>(ButtonID::BTN_MEASURE_LIST);
	if (id < min_num || id > max_num)
	{
		return;
	}

	button_group_->button(id)->setChecked(checked);
}

int ButtonListDialog::GetButtonState(const int id) const
{
	if (button_group_ == nullptr || button_group_->buttons().empty())
	{
		return -1;
	}

	int min_num = static_cast<int>(ButtonID::BTN_WINDOWING);
	int max_num = static_cast<int>(ButtonID::BTN_MEASURE_LIST);
	if (id < min_num || id > max_num)
	{
		return -1;
	}

	return button_group_->button(id)->isChecked();
}

void ButtonListDialog::slotButtonToggle(const int id, const bool button_on)
{
	if (button_on)
	{
		int size = static_cast<int>(ButtonID::BTN_END);
		for (int i = 0; i < size; ++i)
		{
			QAbstractButton* button = button_group_->button(i);
			if (i == static_cast<int>(ButtonID::BTN_WINDOWING) ||
				i == static_cast<int>(ButtonID::BTN_HIDE_UI) ||
				i == static_cast<int>(ButtonID::BTN_GRID_ONOFF) ||
				button->isVisible() == false)
			{
				continue;
			}
			button->setChecked(false);
		}
	}

	if (id > 2 || (id == 0 && button_on))
	{
		accept();
	}
		
	emit sigButtonToggle(id, button_on);
}

void ButtonListDialog::keyReleaseEvent(QKeyEvent* event)
{
	QWidget::keyReleaseEvent(event);

	if (event->key() == Qt::Key_Control)
	{
		emit sigSyncControlButtonOut();
	}
}

void ButtonListDialog::SetVisibleButton(const int id, bool is_visible)
{
	if (button_group_ == nullptr || button_group_->buttons().empty())
	{
		return;
	}

	int min_num = static_cast<int>(ButtonID::BTN_WINDOWING);
	int max_num = static_cast<int>(ButtonID::BTN_MEASURE_LIST);
	if (id < min_num || id > max_num)
	{
		return;
	}

	if (is_visible == false)
	{
		if (button_group_->button(id)->isChecked())
		{
			button_group_->blockSignals(true);
			button_group_->button(id)->setChecked(false);
			button_group_->blockSignals(false);
		}
	}

	button_group_->button(id)->setVisible(is_visible);
}

void ButtonListDialog::AllVisibleButton()
{
	if (button_group_ == nullptr || button_group_->buttons().empty())
	{
		return;
	}

	int size = static_cast<int>(ButtonID::BTN_END);

	for (int i = 0; i < size; ++i)
	{
		button_group_->button(i)->setVisible(true);
	}
}


void ButtonListDialog::SetButtonList()
{
	if (button_group_ != nullptr)
	{
		return;
	}

	QString hide_onoff = lang::LanguagePack::txt_hide_ui() + " " + lang::LanguagePack::txt_on() + "/" + lang::LanguagePack::txt_off();
	QString implant_angle = lang::LanguagePack::txt_implant() + " " + lang::LanguagePack::txt_angle();
	QString grid_onoff = lang::LanguagePack::txt_grid() + " " + lang::LanguagePack::txt_on() + "/" + lang::LanguagePack::txt_off();

	int button_id = static_cast<int>(ButtonID::BTN_WINDOWING);
	button_group_ = new QButtonGroup(this);
	button_group_->setExclusive(false);
	button_group_->addButton(CreateTextToolButton(lang::LanguagePack::txt_windowing()), button_id++);
	button_group_->addButton(CreateTextToolButton(hide_onoff), button_id++);
	button_group_->addButton(CreateTextToolButton(grid_onoff), button_id++);
	button_group_->addButton(CreateTextToolButton(implant_angle), button_id++);
	button_group_->addButton(CreateTextToolButton(lang::LanguagePack::txt_cd_usb_export()), button_id++);
	button_group_->addButton(CreateTextToolButton(lang::LanguagePack::txt_stl_export()), button_id++);
	button_group_->addButton(CreateTextToolButton(lang::LanguagePack::txt_measure_list()), button_id++);
	//button_group_->addButton(CreateTextToolButton(lang::LanguagePack::txt_lightbox()), button_id);

	int size = static_cast<int>(ButtonID::BTN_END);
	for (int i = 0; i < size; ++i)
	{
		main_layout_->addWidget(button_group_->button(i));
	}

	connect(button_group_, SIGNAL(buttonToggled(int, bool)), this, SLOT(slotButtonToggle(int, bool)));
}

QToolButton* ButtonListDialog::CreateTextToolButton(const QString& text)
{
	QToolButton* tool_button = new QToolButton(this);
	tool_button->setCheckable(true);
	tool_button->setStyleSheet(CW3Theme::getInstance()->GlobalPreferencesDialogToolButtonStyleSheet());
	tool_button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
	tool_button->setFixedWidth(CW3Theme::getInstance()->size_button().width());
	tool_button->setFixedHeight(CW3Theme::getInstance()->size_button().height());
	tool_button->setText(text);

	return tool_button;
}
