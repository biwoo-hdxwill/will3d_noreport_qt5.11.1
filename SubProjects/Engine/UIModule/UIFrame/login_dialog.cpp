#include "login_dialog.h"

#include <QDebug>
#include <QCheckBox>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QToolButton>
#include <QButtonGroup>
#include <QLabel>
#include <QTimer>
#include <QKeyEvent>
#include <QFrame>

#include "../../Common/Common/global_preferences.h"
#include "../../Common/Common/W3Theme.h"
#include "../../Common/Common/W3MessageBox.h"
#include "user_manager_dialog.h"
#include "find_username_dialog.h"
#include "find_password_dialog.h"

LoginDialog::LoginDialog(QWidget* parent)
	: CW3Dialog(QString(), parent, CW3Dialog::Theme::Light)
{
	Qt::WindowFlags flags = windowFlags();
	setWindowFlags(flags | Qt::WindowStaysOnTopHint);
	SetLayout();

	slotSelectUser();
}

LoginDialog::~LoginDialog()
{
	UserDatabaseManager::getInstance()->destroy();
}

void LoginDialog::SetLayout()
{
	QVBoxLayout* bottom_layout = new QVBoxLayout();
	forget_username_password_button_ = new QToolButton();

	forget_username_password_button_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

	forget_username_password_button_->setObjectName("NoBackground");

	connect(forget_username_password_button_, SIGNAL(clicked()), this, SLOT(slotForgetUsernamePassword()));

	bottom_layout->setAlignment(Qt::AlignTop);
	bottom_layout->setContentsMargins(20, 0, 20, 10);
	bottom_layout->setSpacing(10);
	bottom_layout->addLayout(CreateContentsLayout());
	bottom_layout->addLayout(CreateButtonLayout());
	bottom_layout->addWidget(forget_username_password_button_);

	QVBoxLayout* main_layout = new QVBoxLayout();
	main_layout->setContentsMargins(0, 0, 0, 0);
	main_layout->setSpacing(0);

	main_layout->addLayout(CreateTabLayout());
	main_layout->addLayout(bottom_layout);

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

QHBoxLayout* LoginDialog::CreateTabLayout()
{
	QHBoxLayout* layout = new QHBoxLayout();
	QVBoxLayout* user_layout = new QVBoxLayout();
	QVBoxLayout* admin_layout = new QVBoxLayout();
	QButtonGroup* tab_group = new QButtonGroup(this);
	QToolButton* user_button = new QToolButton();
	QToolButton* admin_button = new QToolButton();
	user_indicator_ = new QLabel();
	admin_indicator_ = new QLabel();

	user_button->setText(lang::LanguagePack::txt_user());
	admin_button->setText(lang::LanguagePack::txt_admin());

	user_button->setObjectName("Tab");
	admin_button->setObjectName("Tab");
	user_indicator_->setObjectName("SelectedTab");
	admin_indicator_->setObjectName("UnselectedTab");

	user_button->setCheckable(true);
	admin_button->setCheckable(true);

	user_button->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	admin_button->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	user_indicator_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	admin_indicator_->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

	user_indicator_->setContentsMargins(0, 0, 0, 0);
	admin_indicator_->setContentsMargins(0, 0, 0, 0);

	connect(user_button, SIGNAL(clicked()), this, SLOT(slotSelectUser()));
	connect(admin_button, SIGNAL(clicked()), this, SLOT(slotSelectAdmin()));

	tab_group->addButton(user_button, 0);
	tab_group->addButton(admin_button, 1);
	user_button->setChecked(true);

	user_layout->setSpacing(0);
	user_layout->addWidget(user_button);
	user_layout->addWidget(user_indicator_);

	admin_layout->setSpacing(0);
	admin_layout->addWidget(admin_button);
	admin_layout->addWidget(admin_indicator_);

	layout->setSpacing(0);
	layout->addLayout(user_layout);
	layout->addLayout(admin_layout);

	return layout;
}

QVBoxLayout* LoginDialog::CreateContentsLayout()
{
	QVBoxLayout* layout = new QVBoxLayout();
	QHBoxLayout* input_layout = new QHBoxLayout();
	QVBoxLayout* label_layout = new QVBoxLayout();
	QVBoxLayout* line_edit_layout = new QVBoxLayout();
	QHBoxLayout* check_box_layout = new QHBoxLayout();
	QLabel* logo_label = new QLabel();
	QLabel* username_label = new QLabel();
	QLabel* password_label = new QLabel();
	username_input_ = new QLineEdit();
	password_input_ = new QLineEdit();
	save_check_ = new QCheckBox();
	auto_login_check_ = new QCheckBox();

	username_label->setText(lang::LanguagePack::txt_username());
	password_label->setText(lang::LanguagePack::txt_password());

	save_check_->setText(lang::LanguagePack::txt_save());
	auto_login_check_->setText(lang::LanguagePack::txt_auto_login());
  

	connect(save_check_, SIGNAL(clicked(bool)), this, SLOT(slotCheckSave(bool)));
	connect(auto_login_check_, SIGNAL(clicked(bool)), this, SLOT(slotCheckAutoLogin(bool)));

	QImage logo;
	logo.load(":/image/hdxwill_logo.png");
	logo_label->setPixmap(QPixmap::fromImage(logo));
	logo_label->setContentsMargins(0, 20, 0, 20);
	logo_label->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
	logo_label->setAlignment(Qt::AlignCenter);

	username_input_->setFocusPolicy(Qt::StrongFocus);
	password_input_->setEchoMode(QLineEdit::Password);

	label_layout->setSpacing(5);
	label_layout->addWidget(username_label);
	label_layout->addWidget(password_label);

	line_edit_layout->setSpacing(5);
	line_edit_layout->addWidget(username_input_);
	line_edit_layout->addWidget(password_input_);

	input_layout->setSpacing(30);
	input_layout->addLayout(label_layout);
	input_layout->addLayout(line_edit_layout);

	check_box_layout->addWidget(save_check_);
	check_box_layout->addWidget(auto_login_check_);

	GetValues();

	layout->setSpacing(10);
	layout->addWidget(logo_label);
	layout->addLayout(input_layout);
	layout->addLayout(check_box_layout);

	return layout;
}

QHBoxLayout* LoginDialog::CreateButtonLayout()
{
	QHBoxLayout* layout = new QHBoxLayout();
	QToolButton* login_button = new QToolButton();
	QToolButton* exit_button = new QToolButton();

	connect(login_button, SIGNAL(clicked()), this, SLOT(slotLogin()));
	connect(exit_button, SIGNAL(clicked()), this, SLOT(slotExit()));

	login_button->setText(lang::LanguagePack::txt_login());
	exit_button->setText(lang::LanguagePack::txt_exit());

	layout->setSpacing(5);
	layout->setAlignment(Qt::AlignCenter);
	layout->addWidget(login_button);
	layout->addWidget(exit_button);

	return layout;
}

void LoginDialog::GetValues()
{
	bool save = GlobalPreferences::GetInstance()->preferences_.login.save;
  bool auto_login = GlobalPreferences::GetInstance()->preferences_.login.auto_login;
	save_check_->setChecked(save || auto_login);
	auto_login_check_->setChecked(auto_login);
	save_check_->setEnabled(!auto_login);

	QString username = GlobalPreferences::GetInstance()->preferences_.login.username;
	username_input_->setText(username);
	//QString saved_password = GlobalPreferences::GetInstance()->preferences_.login.password;
	//password_input_->setText(UserDatabaseManager::Decrypt(saved_password));
	if (auto_login)
	{
		username_input_->setEnabled(false);
		password_input_->setEnabled(false);
		auto_login_check_->setEnabled(false);

		//password_input_->setText("0000");

		//if (/*UserDatabaseManager::getInstance()->AutoLogin(username)*/)
		{
			QTimer* timer = new QTimer(this);
			timer->setSingleShot(true);
			connect(timer, SIGNAL(timeout()), this, SLOT(slotAutoLogin()));
			timer->start(300);
		}
		//else
		//{
		//	username_input_->setEnabled(true);
		//	password_input_->setEnabled(true);
		//	save_check_->setEnabled(true);
		//	auto_login_check_->setEnabled(true);
		//	auto_login_check_->setChecked(true);

		//	//password_input_->setText("");

		//	CW3MessageBox message_box("Will3D", lang::LanguagePack::msg_64(), CW3MessageBox::Critical, this, CW3Dialog::Theme::Light);
		//	message_box.exec();
		//}
	}
}

void LoginDialog::resizeEvent(QResizeEvent* event)
{
	username_input_->setFocus();
}

void LoginDialog::keyPressEvent(QKeyEvent* event)
{
	if (event->key() == Qt::Key_Return)
	{
		slotLogin();
	}
	CW3Dialog::keyPressEvent(event);
}

// slots
void LoginDialog::reject()
{
	// diable esc key
}

void LoginDialog::slotLogin()
{
	bool save = save_check_->isChecked();
	GlobalPreferences::GetInstance()->preferences_.login.save = save;
  bool auto_login = true;// auto_login_check_->isChecked();
	GlobalPreferences::GetInstance()->preferences_.login.auto_login = auto_login;
	if (save)
	{
		GlobalPreferences::GetInstance()->preferences_.login.username = username_input_->text();
		//QString password = password_input_->text();
		//GlobalPreferences::GetInstance()->preferences_.login.password = UserDatabaseManager::Encrypt(password);
	}

	QString username = username_input_->text();
  QString password = password_input_->text();

	if (UserDatabaseManager::getInstance()->Login(username, password, static_cast<int>(user_type_), auto_login))
	{
		GlobalPreferences::GetInstance()->SaveLogin();

		if (user_type_ == User::UserType::Admin)
		{
			UserManagerDialog user_manager_dialog(this);
			user_manager_dialog.exec();
		}
		else
		{
			accept();
		}
	}
	else
	{
		CW3MessageBox message_box("Will3D", lang::LanguagePack::msg_65(), CW3MessageBox::Critical, this, CW3Dialog::Theme::Light);
		message_box.exec();
	}
}

void LoginDialog::slotAutoLogin()
{
	accept();
}

void LoginDialog::slotExit()
{
	CW3Dialog::reject();
}

void LoginDialog::slotForgetUsernamePassword()
{
	FindUsernameDialog find_username_dialog(this);
	if (find_username_dialog.exec())
	{
		QString username = UserDatabaseManager::getInstance()->FindUsername(find_username_dialog.GetEmail());

		if (username.isEmpty())
		{
			CW3MessageBox message_box("Will3D", lang::LanguagePack::msg_66(), CW3MessageBox::Critical, this, CW3Dialog::Theme::Light);
			message_box.exec();
			return;
		}

		FindPasswordDialog find_password_dialog(username, this);
		int result = find_password_dialog.exec();
		if (result == 1)
		{
			username_input_->setText(username);
		}
		else if (result == 2)
		{
			QString password = UserDatabaseManager::getInstance()->RequestNewPassword(username);

			if (password.isEmpty())
			{
				CW3MessageBox message_box("Will3D", lang::LanguagePack::msg_67(), CW3MessageBox::Critical, this, CW3Dialog::Theme::Light);
				message_box.exec();
			}
			else
			{
				CW3MessageBox message_box("Will3D", lang::LanguagePack::msg_68() + password, CW3MessageBox::Information, this, CW3Dialog::Theme::Light);
				message_box.exec();

				password_input_->setText(password);
			}
		}
	}
}

void LoginDialog::slotSelectUser()
{
	user_type_ = User::UserType::UsertType_User;

	user_indicator_->setObjectName("SelectedTab");
	admin_indicator_->setObjectName("UnselectedTab");

	forget_username_password_button_->setText("");
	forget_username_password_button_->setEnabled(false);

	setStyleSheet(CW3Theme::getInstance()->LightDialogStyleSheet());
}

void LoginDialog::slotSelectAdmin()
{
	user_type_ = User::UserType::Admin;

	admin_indicator_->setObjectName("SelectedTab");
	user_indicator_->setObjectName("UnselectedTab");

	forget_username_password_button_->setText(lang::LanguagePack::txt_forgot_username_password());
	forget_username_password_button_->setEnabled(true);

	setStyleSheet(CW3Theme::getInstance()->LightDialogStyleSheet());
}

void LoginDialog::slotCheckSave(bool checked)
{
	if (!checked)
	{
		GlobalPreferences::GetInstance()->preferences_.login.save = false;
		GlobalPreferences::GetInstance()->preferences_.login.username = "";
		//GlobalPreferences::GetInstance()->preferences_.login.password = "";

		GlobalPreferences::GetInstance()->SaveLogin();
	}
}

void LoginDialog::slotCheckAutoLogin(bool checked)
{
	if (checked)
		save_check_->setChecked(true);

	save_check_->setEnabled(!checked);
}
// slots
