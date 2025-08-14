#include "color_dialog.h"

#include <QDebug>
#include <QColorDialog>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include "language_pack.h"

namespace
{
	//Label key
	const QString kBasicColors = "Basic colors";
	const QString kCustomColors = "Custom colors";
	const QString kHue = "Hue";
	const QString kSat = "Sat";
	const QString kVal = "Val";
	const QString kRed = "Red";
	const QString kGreen = "Green";
	const QString kBlue = "Blue";
	const QString kAlphaChannel = "Alpha channel";
	const QString kHTML = "HTML";
	const QString kScreenCoordinate = "ScreenCoordinate";

	//Button key
	const QString kPickScreenColor = "Pick Screen Color";
	const QString kAddToCustomColors = "Add to Custom Colors";
	const QString kOK = "OK";
	const QString kCancel = "Cancel";
}

ColorDialog::ColorDialog(QWidget* parent /*= 0*/, const Theme theme /*= Theme::Dark*/, bool screen_coordinate_visible /*= false*/)
	: CW3Dialog(lang::LanguagePack::txt_select_color(), parent, theme)
{
	color_dlg_ = new QColorDialog(this);

	InsertLabelObject();
	InsertQPushButtonObject();

	SetLabelText(kBasicColors, lang::LanguagePack::txt_basic_colors());
	SetLabelText(kCustomColors, lang::LanguagePack::txt_custom_colors());
	SetLabelText(kHue, lang::LanguagePack::txt_hue());
	SetLabelText(kSat, lang::LanguagePack::txt_sat());
	SetLabelText(kVal, lang::LanguagePack::txt_val());
	SetLabelText(kRed, lang::LanguagePack::txt_red());
	SetLabelText(kGreen, lang::LanguagePack::txt_green());
	SetLabelText(kBlue, lang::LanguagePack::txt_blue());
	SetLabelText(kAlphaChannel, lang::LanguagePack::txt_alpha_channel());
	SetLabelText(kHTML, lang::LanguagePack::txt_html());

	SetPushButtonText(kPickScreenColor, lang::LanguagePack::txt_pick_screen_color());
	SetPushButtonText(kAddToCustomColors, lang::LanguagePack::txt_add_to_custom_colors());
	SetPushButtonText(kOK, lang::LanguagePack::txt_ok());
	SetPushButtonText(kCancel, lang::LanguagePack::txt_cancel());

	SetScreenCoordinateLabelVisible(screen_coordinate_visible);

	Connection();

	m_contentLayout->addWidget(color_dlg_);
}

ColorDialog::~ColorDialog()
{
	label_map_.clear();
	push_button_map_.clear();
	color_dlg_->deleteLater();
}

void ColorDialog::SetCurrentColor(const QColor& color)
{
	return color_dlg_->setCurrentColor(color);
}

QColor ColorDialog::CurrentColor() const
{
	return color_dlg_->currentColor();
}

QColor ColorDialog::SelectedColor() const
{
	return color_dlg_->selectedColor();
}

int ColorDialog::exec()
{
	color_dlg_->show();
	SetFocusOKPushButton();
	return CW3Dialog::exec();
}

void ColorDialog::InsertLabelObject()
{
	if (color_dlg_ == nullptr)
	{
		return;
	}

	QList<QLabel*> label_list = color_dlg_->findChildren<QLabel*>();
	for (int i = 0; i < label_list.size(); ++i)
	{
		QString text = label_list.at(i)->text();
		if (text.compare("\n") == 0)
		{
			// Pick Screen Color 버튼을 클릭하면 마우스 포인터 위치가 버튼 아래 출력됨. 초기값 '\n'
			label_map_.insert(std::make_pair(kScreenCoordinate, label_list.at(i)));
		}
		else
		{
			text.remove(QRegExp("&"));
			text.remove(QRegExp(":"));

			label_map_.insert(std::make_pair(text, label_list.at(i)));
		}
	}
}

void ColorDialog::InsertQPushButtonObject()
{
	if (color_dlg_ == nullptr)
	{
		return;
	}

	QList<QPushButton*> button_list = color_dlg_->findChildren<QPushButton*>();
	for (int i = 0; i < button_list.size(); ++i)
	{
		QString text = button_list.at(i)->text();
		if (text.compare("\n") == 0)
		{
			continue;
		}
		text.remove(QRegExp("&"));
		text.remove(QRegExp(":"));

		push_button_map_.insert(std::make_pair(text, button_list.at(i)));
	}
}

void ColorDialog::SetLabelText(const QString& key, const QString& text)
{
	std::map<QString, QLabel*>::iterator iter = label_map_.find(key);
	if (iter != label_map_.end())
	{
		if (key.compare(kBasicColors) == 0 ||
			key.compare(kCustomColors) == 0 ||
			key.compare(kScreenCoordinate) == 0)
		{
			iter->second->setText(text);
		}
		else
		{
			QString str = text + ':';
			iter->second->setText(str);
		}
	}
}

void ColorDialog::SetPushButtonText(const QString& key, const QString& text)
{
	std::map<QString, QPushButton*>::iterator iter = push_button_map_.find(key);
	if (iter != push_button_map_.end())
	{
		iter->second->setText(text);
	}
}

void ColorDialog::SetScreenCoordinateLabelVisible(bool screen_coordinate_visible)
{
	if (screen_coordinate_visible == false)
	{
		std::map<QString, QLabel*>::iterator iter = label_map_.find(kScreenCoordinate);
		if (iter != label_map_.end())
		{
			iter->second->hide();
		}
	}
}

void ColorDialog::Connection()
{
	if (color_dlg_ == nullptr)
	{
		return;
	}

	connect(color_dlg_, &QColorDialog::accepted, this, &ColorDialog::accept);
	connect(color_dlg_, &QColorDialog::rejected, this, &ColorDialog::reject);
}

void ColorDialog::SetFocusOKPushButton()
{
	std::map<QString, QPushButton*>::iterator iter = push_button_map_.find(kOK);
	if (iter != push_button_map_.end())
	{
		iter->second->setFocus();
	}
}
