#include "base_widget.h"

#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QToolButton>
#include <QRadioButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include "../../Common/Common/global_preferences.h"
#include "../../Common/Common/W3Theme.h"
#include "../../Common/Common/W3LayoutFunctions.h"
#include "../../Common/Common/language_pack.h"

namespace
{
	const QString kResetObjectName = "ResetBtn";
}

BaseWidget::BaseWidget(QWidget* parent /*= nullptr*/)
	: QWidget(parent)
{
	SetLayout();
}

BaseWidget::~BaseWidget()
{
	if (cover_layout_)
	{
		CW3LayoutFunctions::RemoveWidgetsAll(cover_layout_);
		cover_layout_->deleteLater();
	}
	else
	{
		CW3LayoutFunctions::RemoveWidgetsAll(contents_layout_);
		contents_layout_->deleteLater();
	}
}

void BaseWidget::ResetButtonDisable()
{
	QToolButton* reset_button = findChild<QToolButton*>(kResetObjectName, Qt::FindDirectChildrenOnly);
	//QToolButton* reset_button = findChild<QToolButton*>(kResetObjectName);

	if (reset_button)
	{
		reset_button->setEnabled(false);
		reset_button->setVisible(false);
	}
}

QFrame* BaseWidget::CreateHorizontalLine()
{
	QFrame* line = new QFrame(this);
	line->setFrameShadow(QFrame::Plain);
	line->setFrameShape(QFrame::HLine);
	line->setLineWidth(0);
	line->setMidLineWidth(0);
	line->setStyleSheet(CW3Theme::getInstance()->LineSeparatorStyleSheet());
	line->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

	return line;
}

QFrame* BaseWidget::CreateVerticalLine()
{
	QFrame* line = new QFrame(this);
	line->setFrameShadow(QFrame::Plain);
	line->setFrameShape(QFrame::VLine);
	line->setLineWidth(0);
	line->setMidLineWidth(0);
	line->setStyleSheet(CW3Theme::getInstance()->LineSeparatorStyleSheet());
	line->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

	return line;
}

QComboBox* BaseWidget::CreateComboBox(const QStringList& items, bool fixed_width /*= true*/)
{
	QComboBox* combo_box = new QComboBox(this);
	combo_box->setStyleSheet(CW3Theme::getInstance()->GlobalPreferencesDialogComboBoxStyleSheet());

	if (fixed_width)
	{
		combo_box->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
		combo_box->setFixedWidth(CW3Theme::getInstance()->size_button().width());
	}
	else
	{
		combo_box->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	}
	combo_box->setFixedHeight(CW3Theme::getInstance()->size_button().height());
	combo_box->addItems(items);

	return combo_box;
}

QLabel* BaseWidget::CreateLabel(const QString& text, QSizePolicy::Policy hor /*= QSizePolicy::Preferred*/, QSizePolicy::Policy ver /*= QSizePolicy::Preferred*/, Qt::Alignment alignment /*= Qt::AlignTop*/)
{
	QLabel* label = new QLabel(this);
	label->setText(text);
	label->setAlignment(alignment);
	label->setSizePolicy(hor, ver);

	return label;
}

QLineEdit* BaseWidget::CreateLineEdit(QSizePolicy::Policy hor /*= QSizePolicy::Preferred*/, QSizePolicy::Policy ver /*= QSizePolicy::Fixed*/)
{
	QLineEdit* line_edit = new QLineEdit(this);
	line_edit->setContentsMargins(kMarginZero);
	line_edit->setStyleSheet(CW3Theme::getInstance()->GlobalPreferencesDialogLineEditStyleSheet());
	line_edit->setSizePolicy(hor, ver);
	line_edit->setFixedHeight(CW3Theme::getInstance()->size_button().height());

	return line_edit;
}

QToolButton* BaseWidget::CreateColoredToolButton(QColor color, QSizePolicy::Policy hor /*= QSizePolicy::Fixed*/, QSizePolicy::Policy ver /*= QSizePolicy::Fixed*/)
{
	QToolButton* tool_button = new QToolButton(this);
	tool_button->setLayoutDirection(Qt::RightToLeft);
	tool_button->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
	tool_button->setIcon(QIcon(":/image/buttons/icon_corner.png"));
	tool_button->setContentsMargins(kMarginZero);
	tool_button->setStyleSheet(CW3Theme::getInstance()->ColoredToolButtonStyleSheet(color));
	tool_button->setSizePolicy(hor, ver);
	tool_button->setFixedWidth(CW3Theme::getInstance()->size_button().width());
	tool_button->setFixedHeight(CW3Theme::getInstance()->size_button().height());

	return tool_button;
}

QToolButton* BaseWidget::CreateTextToolButton(const QString& text, QSizePolicy::Policy hor /*= QSizePolicy::Fixed*/, QSizePolicy::Policy ver /*= QSizePolicy::Fixed*/)
{
	QToolButton* tool_button = new QToolButton(this);
	tool_button->setContentsMargins(kMarginZero);
	tool_button->setStyleSheet(CW3Theme::getInstance()->GlobalPreferencesDialogToolButtonStyleSheet());
	tool_button->setSizePolicy(hor, ver);
	tool_button->setFixedWidth(CW3Theme::getInstance()->size_button().width());
	tool_button->setFixedHeight(CW3Theme::getInstance()->size_button().height());
	tool_button->setText(text);

	return tool_button;
}

QRadioButton* BaseWidget::CreateTextRadioBttton(const QString& text, QSizePolicy::Policy hor /*= QSizePolicy::Preferred*/, QSizePolicy::Policy ver /*= QSizePolicy::Fixed*/)
{
	QRadioButton* radio_button = new QRadioButton(this);
	radio_button->setSizePolicy(hor, ver);
	radio_button->setText(text);

	return radio_button;
}

QSpinBox* BaseWidget::CreateSpinBox(int min, int max, int default_value)
{
	QSpinBox* spin_box = new QSpinBox();

	spin_box->setStyle(new ViewSpinBoxStyle());
	spin_box->setStyleSheet(CW3Theme::getInstance()->GlobalPreferencesDialogSpinBoxStyleSheet());

	spin_box->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);

	spin_box->setFixedWidth(CW3Theme::getInstance()->size_button().width());
	spin_box->setFixedHeight(CW3Theme::getInstance()->size_button().height());

	spin_box->setMinimum(min);
	spin_box->setMaximum(max);
	spin_box->setValue(default_value);

	return spin_box;
}

QDoubleSpinBox* BaseWidget::CreateDoubleSpinBox(float min, float max, float default_value, QSizePolicy::Policy hor /*= QSizePolicy::Fixed*/, QSizePolicy::Policy ver /*= QSizePolicy::Fixed*/)
{
	QDoubleSpinBox* spin_box = new QDoubleSpinBox(this);

	spin_box->setStyle(new ViewSpinBoxStyle());
	spin_box->setStyleSheet(CW3Theme::getInstance()->GlobalPreferencesDialogSpinBoxStyleSheet());

	spin_box->setSizePolicy(hor, ver);

	spin_box->setFixedWidth(CW3Theme::getInstance()->size_button().width());
	spin_box->setFixedHeight(CW3Theme::getInstance()->size_button().height());

	spin_box->setMinimum(min);
	spin_box->setMaximum(max);
	spin_box->setValue(default_value);
	spin_box->setDecimals(1);

	return spin_box;
}

QVBoxLayout* BaseWidget::CreateResetButtonLayout()
{
	QVBoxLayout* reset_layout = new QVBoxLayout();
	{
		reset_layout->setAlignment(Qt::AlignTop);
		reset_layout->setSpacing(kSpacing10);
		reset_layout->setContentsMargins(kStepMargins);

		QVBoxLayout* button_layout = new QVBoxLayout();
		{
			button_layout->setContentsMargins(QMargins(20, 0, 20, 10));
			button_layout->setAlignment(Qt::AlignRight);

			QToolButton* reset_button = CreateTextToolButton(lang::LanguagePack::txt_reset());
			reset_button->setObjectName(kResetObjectName);

			connect(reset_button, &QToolButton::clicked, [=]() { Reset(); });

			button_layout->addWidget(reset_button);
		}

		reset_layout->addWidget(CreateHorizontalLine());
		reset_layout->addLayout(button_layout);
	}

	return reset_layout;
}

void BaseWidget::SetLayout()
{
	contents_layout_ = new QVBoxLayout();
	contents_layout_->setContentsMargins(kMarginZero);
	contents_layout_->setSpacing(kSpacingZero);

	if (GlobalPreferences::GetInstance()->preferences_.preferences_common.reset_button_enable)
	{
		cover_layout_ = new QVBoxLayout();
		cover_layout_->setContentsMargins(kMarginZero);
		cover_layout_->setSpacing(kSpacingZero);

		cover_layout_->addLayout(contents_layout_);
		cover_layout_->addLayout(CreateResetButtonLayout());

		setLayout(cover_layout_);
	}
	else
	{
		setLayout(contents_layout_);
	}
}
