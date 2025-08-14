#include "user_manager_dialog.h"

#include <QDebug>
#include <QListWidget>
#include <QLineEdit>
#include <QRadioButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QToolButton>
#include <QButtonGroup>
#include <QLabel>

#include "../../Common/Common/language_pack.h"
#include "../../Common/Common/W3Theme.h"
#include "../../Common/Common/W3MessageBox.h"
#include "../../Common/Common/W3Memory.h"
#include "user_list_item.h"
#include "new_user_dialog.h"

UserManagerDialog::UserManagerDialog(QWidget* parent)
	: CW3Dialog(lang::LanguagePack::txt_user_manager(), parent, CW3Dialog::Theme::Light) {
	SetLayout();

	SetUserList();
}

UserManagerDialog::~UserManagerDialog() {
}

void UserManagerDialog::SetLayout() {
	QHBoxLayout* main_layout = new QHBoxLayout();
	main_layout->setContentsMargins(20, 10, 20, 10);
	main_layout->setSpacing(10);

	main_layout->addLayout(CreateUserListLayout());
	main_layout->addLayout(CreateUserInformationLayout());
	main_layout->setStretch(0, 2);
	main_layout->setStretch(1, 3);

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

QVBoxLayout* UserManagerDialog::CreateUserListLayout() {
	QVBoxLayout* layout = new QVBoxLayout();
	QVBoxLayout* user_list_layout = new QVBoxLayout();
	QHBoxLayout* button_layout = new QHBoxLayout();
	QLabel* title = new QLabel();
	user_list_widget_ = new QListWidget();
	QToolButton* new_user_button = new QToolButton();

	title->setText(lang::LanguagePack::txt_user_list());
	new_user_button->setText(lang::LanguagePack::txt_new_user());

	connect(new_user_button, SIGNAL(clicked()), this, SLOT(slotNewUser()));
	connect(user_list_widget_, SIGNAL(currentRowChanged(int)), this, SLOT(slotUserListRowChanged(int)));

	button_layout->setAlignment(Qt::AlignRight);
	button_layout->addWidget(new_user_button);

	layout->setSpacing(5);
	layout->addWidget(title);
	layout->addWidget(user_list_widget_);
	layout->addLayout(button_layout);

	return layout;
}

QVBoxLayout* UserManagerDialog::CreateUserInformationLayout() {
	QVBoxLayout* layout = new QVBoxLayout();
	QVBoxLayout* contents_layout = new QVBoxLayout();
	QVBoxLayout* label_layout = new QVBoxLayout();
	QVBoxLayout* line_edit_layout = new QVBoxLayout();
	QHBoxLayout* input_layout = new QHBoxLayout();
	QHBoxLayout* user_type_layout = new QHBoxLayout();
	QHBoxLayout* button_layout = new QHBoxLayout();
	QFrame* frame = new QFrame();
	QLabel* title = new QLabel();
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
	apply_changes_button_ = new QToolButton();
	delete_user_button_ = new QToolButton();

	title->setText(lang::LanguagePack::txt_user_information());
	username_label->setText(lang::LanguagePack::txt_username() + "(*)");
	password_label->setText(lang::LanguagePack::txt_password() + "(*)");
	name_label->setText(lang::LanguagePack::txt_name());
	email_label->setText(lang::LanguagePack::txt_email() + "(*)");
	phone_label->setText(lang::LanguagePack::txt_phone());
	user_type_label->setText(lang::LanguagePack::txt_user_type());
	user_type_user_radio->setText(lang::LanguagePack::txt_user());
	user_type_administrator_radio->setText(lang::LanguagePack::txt_administrator());
	required_fields_label->setText(lang::LanguagePack::txt_required_fields());
	apply_changes_button_->setText(lang::LanguagePack::txt_apply_changes());
	delete_user_button_->setText(lang::LanguagePack::txt_delete_user());

	connect(apply_changes_button_, SIGNAL(clicked()), this, SLOT(slotApplyChanges()));
	connect(delete_user_button_, SIGNAL(clicked()), this, SLOT(slotDeleteUser()));

	password_input_->setEchoMode(QLineEdit::Password);

	required_fields_label->setAlignment(Qt::AlignRight);

	frame->setObjectName("ToolBox");
	frame->setStyleSheet(CW3Theme::getInstance()->toolBoxStyleSheet());

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

	contents_layout->addLayout(input_layout);
	contents_layout->addWidget(required_fields_label);
	frame->setLayout(contents_layout);

	button_layout->setAlignment(Qt::AlignRight);
	button_layout->setSpacing(5);
	button_layout->addWidget(apply_changes_button_);
	button_layout->addWidget(delete_user_button_);

	layout->setSpacing(5);
	layout->addWidget(title);
	layout->addWidget(frame);
	layout->addLayout(button_layout);

	return layout;
}

void UserManagerDialog::InitInput() {
	QList<QAbstractButton*> buttons = user_type_radio_group_->buttons();

	username_input_->clear();
	password_input_->clear();
	name_input_->clear();
	email_input_->clear();
	phone_input_->clear();
	buttons.at(0)->setChecked(true);

	username_input_->setEnabled(false);
	password_input_->setEnabled(false);
	name_input_->setEnabled(false);
	email_input_->setEnabled(false);
	phone_input_->setEnabled(false);
	for (int i = 0; i < buttons.size(); i++)
		buttons.at(i)->setEnabled(false);
	apply_changes_button_->setEnabled(false);
	delete_user_button_->setEnabled(false);
}

void UserManagerDialog::SetUserList() {
	user_list_widget_->clear();
	InitInput();

	QList<User> user_list = UserDatabaseManager::getInstance()->GetUserList();

	for (int i = 0; i < user_list.size(); i++) {
		UserListItem* widget = new UserListItem(this);
		widget->SetName(user_list.at(i).username);
		widget->SetUserType(user_list.at(i).type == User::UserType::UsertType_User ? lang::LanguagePack::txt_user() : lang::LanguagePack::txt_administrator());

		QListWidgetItem* item = new QListWidgetItem(user_list_widget_);
		item->setSizeHint(widget->sizeHint());

		user_list_widget_->addItem(item);
		user_list_widget_->setItemWidget(item, widget);

		user_list_widget_->update();
	}
}

void UserManagerDialog::SetUserInformation(int index) {
	password_input_->setEnabled(true);
	name_input_->setEnabled(true);
	email_input_->setEnabled(true);
	phone_input_->setEnabled(true);
	apply_changes_button_->setEnabled(true);

	QList<User> user_list = UserDatabaseManager::getInstance()->GetUserList();

	if (user_list.size() > index) {
		User user = user_list.at(index);

		username_input_->setText(user.username);
		//password_input_->setText(user.password);
		name_input_->setText(user.name);
		email_input_->setText(user.email);
		phone_input_->setText(user.phone);
		QAbstractButton* button = user_type_radio_group_->button(static_cast<int>(user.type));
		if (button)
			button->setChecked(true);

		bool admin = user.username.compare("admin") == 0;

		QList<QAbstractButton*> buttons = user_type_radio_group_->buttons();
		for (int i = 0; i < buttons.size(); i++)
			buttons.at(i)->setEnabled(!admin);

		delete_user_button_->setEnabled(!admin);
	}
}

void UserManagerDialog::SetUserInformation(QListWidgetItem* item) {
	int index = user_list_widget_->row(item);
	SetUserInformation(index);
}

// slots
void UserManagerDialog::slotNewUser() {
	NewUserDialog new_user_dialog(this);
	if (new_user_dialog.exec()) {
		QString username = new_user_dialog.GetUserName();
		QString password = new_user_dialog.GetPassword();
		QString name = new_user_dialog.GetName();
		QString email = new_user_dialog.GetEmail();
		QString phone = new_user_dialog.GetPhone();
		int user_type = new_user_dialog.GetUserType();

		// username, email 각각 중복 예외 처리
		if (UserDatabaseManager::getInstance()->AddNewUser(username, password, name, email, phone, user_type)) {
			SetUserList();
		} else {
			CW3MessageBox message_box("Will3D", lang::LanguagePack::msg_64(), CW3MessageBox::Critical, this, CW3Dialog::Theme::Light);
			message_box.exec();
		}
	}
}

void UserManagerDialog::slotApplyChanges() {
	int currentRow = user_list_widget_->currentRow();

	QString username = username_input_->text();
	QString password = password_input_->text();
	QString name = name_input_->text();
	QString email = email_input_->text();
	QString phone = phone_input_->text();
	int user_type = user_type_radio_group_->checkedId();

	/*if (password.isEmpty()) {
		CW3MessageBox msgBox("Will3D", "Password is empty.", CW3MessageBox::Information, this, CW3Dialog::Theme::Light);
		msgBox.exec();
		return;
	} else */if (email.isEmpty()) {
		CW3MessageBox message_box("Will3D", lang::LanguagePack::msg_71(), CW3MessageBox::Information, this, CW3Dialog::Theme::Light);
		message_box.exec();
		return;
	}

	if (UserDatabaseManager::getInstance()->UpdateUser(username, password, name, email, phone, user_type)) {
		SetUserList();

		if (0 <= currentRow && currentRow < user_list_widget_->count())
			user_list_widget_->setCurrentRow(currentRow);
	} else {
		CW3MessageBox message_box("Will3D", lang::LanguagePack::msg_67(), CW3MessageBox::Critical, this, CW3Dialog::Theme::Light);
		message_box.exec();
	}
}

void UserManagerDialog::slotDeleteUser() {
	CW3MessageBox message_box("Will3D", lang::LanguagePack::msg_72(), CW3MessageBox::Question, this, CW3Dialog::Theme::Light);
	int result = message_box.exec();
	if (result) {
		if (UserDatabaseManager::getInstance()->DeleteUser(user_list_widget_->currentRow())) {
			SetUserList();
		} else {
			CW3MessageBox message_box("Will3D", lang::LanguagePack::msg_67(), CW3MessageBox::Critical, this, CW3Dialog::Theme::Light);
			message_box.exec();
		}
	}
}

void UserManagerDialog::slotUserListRowChanged(int currentRow) {
	if (0 <= currentRow && currentRow < user_list_widget_->count())
		SetUserInformation(currentRow);
}
// slots
