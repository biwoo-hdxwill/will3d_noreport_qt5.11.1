#include "custom_implant_dialog.h"

#include <QDebug>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QToolButton>
#include <QLabel>
#include <QRadioButton>
#include <QButtonGroup>
#include <QLineEdit>
#include <QComboBox>
#include <QDoubleValidator>
#include <QCompleter>

#include <Engine/Common/Common/language_pack.h>
#include <Engine/Common/Common/W3Theme.h>
#include <Engine/Common/Common/W3MessageBox.h>

CustomImplantDialog::CustomImplantDialog(QWidget* parent)
	: CW3Dialog(lang::LanguagePack::txt_custom_implant(), parent)
{
	setFixedWidth(400);

	SetLayout();
}

CustomImplantDialog::~CustomImplantDialog()
{
}

void CustomImplantDialog::SetLayout()
{
	m_contentLayout->setContentsMargins(20, 10, 20, 10);
	m_contentLayout->setSpacing(10);
	m_contentLayout->addLayout(CreateContentsLayout());
	m_contentLayout->addLayout(CreateButtonLayout());
}

QVBoxLayout* CustomImplantDialog::CreateContentsLayout()
{
	QVBoxLayout* layout = new QVBoxLayout();
	QHBoxLayout* manufacturer_layout = new QHBoxLayout();
	QHBoxLayout* product_layout = new QHBoxLayout();
	QHBoxLayout* model_layout = new QHBoxLayout();
	QHBoxLayout* type_layout = new QHBoxLayout();
	QHBoxLayout* type_radio_layout = new QHBoxLayout();
	QHBoxLayout* coronal_diameter_layout = new QHBoxLayout();
	QHBoxLayout* apical_diameter_layout = new QHBoxLayout();
	QHBoxLayout* length_layout = new QHBoxLayout();
	QHBoxLayout* coronal_diameter_input_layout = new QHBoxLayout();
	QHBoxLayout* apical_diameter_input_layout = new QHBoxLayout();
	QHBoxLayout* length_input_layout = new QHBoxLayout();
	QLabel* manufacturer_label = new QLabel();
	QLabel* product_label = new QLabel();
	QLabel* model_label = new QLabel();
	QLabel* type_label = new QLabel();
	QLabel* coronal_diameter_label = new QLabel();
	QLabel* apical_diameter_label = new QLabel();
	QLabel* length_label = new QLabel();
	QLabel* coronal_diameter_mm_label = new QLabel();
	QLabel* apical_diameter_mm_label = new QLabel();
	QLabel* length_mm_label = new QLabel();

	manufacturer_combo_ = new QComboBox();
	product_combo_ = new QComboBox();
	model_edit_ = new QLineEdit();
	coronal_diameter_edit_ = new QLineEdit();
	apical_diameter_edit_ = new QLineEdit();
	length_edit_ = new QLineEdit();

	QDoubleValidator* double_validator = new QDoubleValidator(0.01f, 99.0f, 2, this);
	coronal_diameter_edit_->setValidator(double_validator);
	apical_diameter_edit_->setValidator(double_validator);
	length_edit_->setValidator(double_validator);

	type_radio_group_ = new QButtonGroup(this);
	straight_radio_ = new QRadioButton();
	tapered_radio_ = new QRadioButton();

	connect(type_radio_group_, SIGNAL(buttonClicked(int)), this, SLOT(slotTypeChanged(int)));

	straight_radio_->setChecked(true);
	apical_diameter_edit_->setEnabled(tapered_radio_->isChecked());

	manufacturer_combo_->setEditable(true);
	product_combo_->setEditable(true);

	manufacturer_label->setText(lang::LanguagePack::txt_manufacturer() + " :");
	product_label->setText(lang::LanguagePack::txt_product() + " :");
	model_label->setText(lang::LanguagePack::txt_model() + " :");
	type_label->setText(lang::LanguagePack::txt_type());
	coronal_diameter_label->setText(lang::LanguagePack::txt_coronal_diameter());
	apical_diameter_label->setText(lang::LanguagePack::txt_apical_diameter());
	length_label->setText(lang::LanguagePack::txt_length());
	coronal_diameter_mm_label->setText("mm");
	apical_diameter_mm_label->setText("mm");
	length_mm_label->setText("mm");
	straight_radio_->setText(lang::LanguagePack::txt_straight());
	tapered_radio_->setText(lang::LanguagePack::txt_tapered());

	coronal_diameter_mm_label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
	apical_diameter_mm_label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
	length_mm_label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

	type_radio_group_->addButton(straight_radio_, 0);
	type_radio_group_->addButton(tapered_radio_, 1);

	manufacturer_layout->setSpacing(10);
	manufacturer_layout->addWidget(manufacturer_label, 1);
	manufacturer_layout->addWidget(manufacturer_combo_, 1);

	product_layout->setSpacing(10);
	product_layout->addWidget(product_label, 1);
	product_layout->addWidget(product_combo_, 1);

	model_layout->setSpacing(10);
	model_layout->addWidget(model_label, 1);
	model_layout->addWidget(model_edit_, 1);

	type_radio_layout->setSpacing(5);
	type_radio_layout->setAlignment(Qt::AlignLeft);
	type_radio_layout->addWidget(straight_radio_);
	type_radio_layout->addWidget(tapered_radio_);

	type_layout->setSpacing(10);

	type_layout->addWidget(type_label, 1);
	type_layout->addLayout(type_radio_layout, 1);

	coronal_diameter_input_layout->setSpacing(3);
	coronal_diameter_input_layout->addWidget(coronal_diameter_edit_, 1);
	coronal_diameter_input_layout->addWidget(coronal_diameter_mm_label);

	coronal_diameter_layout->setSpacing(10);
	coronal_diameter_layout->addWidget(coronal_diameter_label, 1);
	coronal_diameter_layout->addLayout(coronal_diameter_input_layout, 1);

	apical_diameter_input_layout->setSpacing(3);
	apical_diameter_input_layout->addWidget(apical_diameter_edit_, 1);
	apical_diameter_input_layout->addWidget(apical_diameter_mm_label);

	apical_diameter_layout->setSpacing(10);
	apical_diameter_layout->addWidget(apical_diameter_label, 1);
	apical_diameter_layout->addLayout(apical_diameter_input_layout, 1);

	length_input_layout->setSpacing(3);
	length_input_layout->addWidget(length_edit_, 1);
	length_input_layout->addWidget(length_mm_label);

	length_layout->setSpacing(10);
	length_layout->addWidget(length_label, 1);
	length_layout->addLayout(length_input_layout, 1);

	layout->addLayout(manufacturer_layout);
	layout->addLayout(product_layout);
	layout->addLayout(model_layout);
	layout->addWidget(CreateHorizontalLine());
	layout->addLayout(type_layout);
	layout->addLayout(coronal_diameter_layout);
	layout->addLayout(apical_diameter_layout);
	layout->addLayout(length_layout);

	return layout;
}

QHBoxLayout* CustomImplantDialog::CreateButtonLayout()
{
	QHBoxLayout* layout = new QHBoxLayout();
	layout->setSpacing(10);
	layout->setAlignment(Qt::AlignCenter);

	QToolButton* ok_button = new QToolButton();
	QToolButton* cancel_button = new QToolButton();

	connect(ok_button, SIGNAL(clicked()), this, SLOT(slotOK()));
	connect(cancel_button, SIGNAL(clicked()), this, SLOT(reject()));

	ok_button->setText(lang::LanguagePack::txt_ok());
	cancel_button->setText(lang::LanguagePack::txt_cancel());

	layout->addWidget(ok_button);
	layout->addWidget(cancel_button);

	return layout;
}

void CustomImplantDialog::SetManufacturerList(const QStringList& list)
{
	manufacturer_combo_->addItems(list);
	manufacturer_combo_->setCompleter(new QCompleter(list, manufacturer_combo_));
}

void CustomImplantDialog::SetProductList(const QStringList& list)
{
	product_combo_->addItems(list);
	product_combo_->setCompleter(new QCompleter(list, product_combo_));
}

void CustomImplantDialog::SetData(
	const QString& manufacturer,
	const QString& product,
	const QString& model,
	const float coronal_diameter,
	const float apical_diameter,
	const float length
)
{
	manufacturer_combo_->setEditText(manufacturer);
	product_combo_->setEditText(product);
	model_edit_->setText(model);
	coronal_diameter_edit_->setText(QString::number(coronal_diameter));
	apical_diameter_edit_->setText(QString::number(apical_diameter));
	length_edit_->setText(QString::number(length));

	//manufacturer_combo_->setEditable(false);
	//product_combo_->setEditable(false);

	straight_radio_->setChecked(coronal_diameter == apical_diameter);
	tapered_radio_->setChecked(coronal_diameter != apical_diameter);
	apical_diameter_edit_->setEnabled(coronal_diameter != apical_diameter);
}

void CustomImplantDialog::GetData(
	QString& manufacturer,
	QString& product,
	QString& model,
	float& coronal_diameter,
	float& apical_diameter,
	float& length
)
{
	manufacturer = manufacturer_combo_->currentText();
	product = product_combo_->currentText();
	model = model_edit_->text();
	coronal_diameter = coronal_diameter_edit_->text().toFloat();
	apical_diameter = apical_diameter_edit_->text().toFloat();
	length = length_edit_->text().toFloat();
}

void CustomImplantDialog::slotOK()
{
	bool tapered = tapered_radio_->isChecked();
	if (!tapered)
	{
		apical_diameter_edit_->setText(coronal_diameter_edit_->text());
	}

	QString manufacturer = manufacturer_combo_->currentText();
	QString product = product_combo_->currentText();
	QString model = model_edit_->text();
	QString coronal_diameter = coronal_diameter_edit_->text();
	QString apical_diameter = apical_diameter_edit_->text();
	QString length = length_edit_->text();

	if (manufacturer.isEmpty())
	{
		CW3MessageBox message_box("Will3D", lang::LanguagePack::msg_79(), CW3MessageBox::Critical);
		message_box.exec();
	}
	else if (product.isEmpty())
	{
		CW3MessageBox message_box("Will3D", lang::LanguagePack::msg_80(), CW3MessageBox::Critical);
		message_box.exec();
	}
	else if (model.isEmpty())
	{
		CW3MessageBox message_box("Will3D", lang::LanguagePack::msg_81(), CW3MessageBox::Critical);
		message_box.exec();
	}
	else if (coronal_diameter.isEmpty() ||
		coronal_diameter.toFloat() <= 0.0f)
	{
		CW3MessageBox message_box("Will3D", lang::LanguagePack::msg_82(), CW3MessageBox::Critical);
		message_box.exec();
	}
	else if (tapered &&
		(apical_diameter.isEmpty() ||
			apical_diameter.toFloat() <= 0.0f))
	{
		CW3MessageBox message_box("Will3D", lang::LanguagePack::msg_83(), CW3MessageBox::Critical);
		message_box.exec();
	}
	else if (length.isEmpty() ||
		length.toFloat() <= 0.0f)
	{
		CW3MessageBox message_box("Will3D", lang::LanguagePack::msg_84(), CW3MessageBox::Critical);
		message_box.exec();
	}
	else
	{
		//accept();
		emit sigThrowData(manufacturer, product, model, coronal_diameter.toFloat(), apical_diameter.toFloat(), length.toFloat());
	}
}

void CustomImplantDialog::slotTypeChanged(int index)
{
	switch (index)
	{
	case 0:
		apical_diameter_edit_->setText(coronal_diameter_edit_->text());
		apical_diameter_edit_->setEnabled(false);
		break;
	case 1:
		apical_diameter_edit_->setEnabled(true);
		break;
	default:
		break;
	}
}
