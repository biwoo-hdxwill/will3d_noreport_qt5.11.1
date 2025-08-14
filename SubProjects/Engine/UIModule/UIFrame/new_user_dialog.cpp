#include "new_user_dialog.h"

#include <QDebug>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QToolButton>
#include <QLabel>
#include <QRadioButton>
#include <QButtonGroup>
#include <QLineEdit>
#include <QKeyEvent>

#include "../../Common/Common/language_pack.h"
#include "../../Common/Common/W3Theme.h"
#include "../../Common/Common/W3MessageBox.h"

NewUserDialog::NewUserDialog(QWidget* parent)
	: CW3Dialog(lang::LanguagePack::txt_add_a_new_user(), parent, CW3Dialog::Theme::Light) {
	SetLayout();
}

NewUserDialog::~NewUserDialog() {
}

void NewUserDialog::SetLayout() {
	QVBoxLayout* main_layout = new QVBoxLayout();
	main_layout->setContentsMargins(20, 10, 20, 10);
	main_layout->setSpacing(10);

	main_layout->addLayout(CreateContentsLayout());
	main_layout->addLayout(CreateButtonLayout());

	QFrame* main_frame = new QFrame();
	main_frame->setObjectName("LoginDialogFrame");
	main_frame->setStyleSheet(
		QString(
			"QFrame#LoginDialogFrame"
			"{"
			"	background-color: #FFFFFF;"
			"	border: 1px solid #000000"
			"}"
		)
	);

	main_frame->setLayout(main_layout);

	m_contentLayout->addWidget(main_frame);
}

QVBoxLayout* NewUserDialog::CreateContentsLayout() {
	QVBoxLayout* layout = new QVBoxLayout();
	QVBoxLayout* label_layout = new QVBoxLayout();
	QVBoxLayout* line_edit_layout = new QVBoxLayout();
	QHBoxLayout* input_layout = new QHBoxLayout();
	QHBoxLayout* user_type_layout = new QHBoxLayout();
	QLabel* username_label = new QLabel();
	QLabel* password_label = new QLabel();
	QLabel* name_label = new QLabel();
	QLabel* email_label = new QLabel();
	QLabel* phone_label = new QLabel();
	QLabel* user_type_label = new QLabel();
	QLabel* required_fields_label = new QLabel();
	username_input_ = new QLineEdit();
	password_input_ = new QLineEdit();
	name_input_ = new QLineEdit();
	email_input_ = new QLineEdit();
	phone_input_ = new QLineEdit();
	user_type_radio_group_ = new QButtonGroup(this);
	QRadioButton* user_type_user_radio = new QRadioButton();
	QRadioButton* user_type_administrator_radio = new QRadioButton();

	username_label->setText(lang::LanguagePack::txt_username() + "(*)");
	password_label->setText(lang::LanguagePack::txt_password() + "(*)");
	name_label->setText(lang::LanguagePack::txt_name());
	email_label->setText(lang::LanguagePack::txt_email() + "(*)");
	phone_label->setText(lang::LanguagePack::txt_phone());
	user_type_label->setText(lang::LanguagePack::txt_user_type());
	user_type_user_radio->setText(lang::LanguagePack::txt_user());
	user_type_administrator_radio->setText(lang::LanguagePack::txt_administrator());
	required_fields_label->setText(lang::LanguagePack::txt_required_fields());

	password_input_->setEchoMode(QLineEdit::Password);

	required_fields_label->setAlignment(Qt::AlignRight);

	user_type_user_radio->setChecked(true);

	user_type_radio_group_->addButton(user_type_user_radio, 0);
	user_type_radio_group_->addButton(user_type_administrator_radio, 1);

	label_layout->setSpacing(5);
	label_layout->addWidget(username_label);
	label_layout->addWidget(password_label);
	label_layout->addWidget(name_label);
	label_layout->addWidget(email_label);
	label_layout->addWidget(phone_label);
	label_layout->addWidget(user_type_label);

	user_type_layout->addWidget(user_type_user_radio);
	user_type_layout->addWidget(user_type_administrator_radio);

	line_edit_layout->setSpacing(5);
	line_edit_layout->addWidget(username_input_);
	line_edit_layout->addWidget(password_input_);
	line_edit_layout->addWidget(name_input_);
	line_edit_layout->addWidget(email_input_);
	line_edit_layout->addWidget(phone_input_);
	line_edit_layout->addLayout(user_type_layout);

	input_layout->setSpacing(20);
	input_layout->addLayout(label_layout);
	input_layout->addLayout(line_edit_layout);

	layout->setSpacing(5);
	layout->addLayout(input_layout);
	layout->addWidget(required_fields_label);

	return layout;
}

QHBoxLayout* NewUserDialog::CreateButtonLayout() {
	QHBoxLayout* layout = new QHBoxLayout();
	layout->setSpacing(5);
	layout->setAlignment(Qt::AlignCenter);
	
	QToolButton* ok_button = new QToolButton();
	QToolButton* cancel_button = new QToolButton();

	connect(ok_button, SIGNAL(clicked()), this, SLOT(slotOk()));
	connect(cancel_button, SIGNAL(clicked()), this, SLOT(reject()));

	ok_button->setText(lang::LanguagePack::txt_ok());
	cancel_button->setText(lang::LanguagePack::txt_cancel());

	layout->addWidget(ok_button);
	layout->addWidget(cancel_button);

	return layout;
}

const QString NewUserDialog::GetUserName() const {
	return username_input_->text();
}

const QString NewUserDialog::GetPassword() const {
	return password_input_->text();
}

const QString NewUserDialog::GetName() const {
	return name_input_->text();
}

const QString NewUserDialog::GetEmail() const {
	return email_input_->text();
}

const QString NewUserDialog::GetPhone() const {
	return phone_input_->text();
}

const int NewUserDialog::GetUserType() const {
	return user_type_radio_group_->checkedId();
}

void NewUserDialog::keyPressEvent(QKeyEvent* event) {
	if (event->key() == Qt::Key_Return) {
		slotOk();
	}
	CW3Dialog::keyPressEvent(event);
}

// slots
void NewUserDialog::slotOk() {
	if (username_input_->text().isEmpty()) {
		CW3MessageBox msgBox("Will3D", lang::LanguagePack::msg_69(), CW3MessageBox::Information, this, CW3Dialog::Theme::Light);
		msgBox.exec();
		return;
	} else if (password_input_->text().isEmpty()) {
		CW3MessageBox msgBox("Will3D", lang::LanguagePack::msg_70(), CW3MessageBox::Information, this, CW3Dialog::Theme::Light);
		msgBox.exec();
		return;
	} else if (email_input_->text().isEmpty()) {
		CW3MessageBox msgBox("Will3D", lang::LanguagePack::msg_71(), CW3MessageBox::Information, this, CW3Dialog::Theme::Light);
		msgBox.exec();
		return;
	}

	accept();
}
// slots
