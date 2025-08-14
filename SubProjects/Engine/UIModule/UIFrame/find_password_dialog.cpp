#include "find_password_dialog.h"

#include <QDebug>
#include <QVBoxLayout>
#include <QToolButton>
#include <QLabel>

#include "../../Common/Common/language_pack.h"
#include "../../Common/Common/W3Theme.h"

FindPasswordDialog::FindPasswordDialog(const QString& username, QWidget* parent)
	: CW3Dialog(lang::LanguagePack::txt_find_username_and_password(), parent, CW3Dialog::Theme::Light),
	username_(username) {
	SetLayout();
}

FindPasswordDialog::~FindPasswordDialog() {
}

void FindPasswordDialog::SetLayout() {
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

QVBoxLayout* FindPasswordDialog::CreateContentsLayout() {
	QVBoxLayout* layout = new QVBoxLayout();
	QLabel* message = new QLabel();

	message->setText("Your username is : " + username_);

	layout->addWidget(message);

	return layout;
}

QVBoxLayout* FindPasswordDialog::CreateButtonLayout() {
	QVBoxLayout* layout = new QVBoxLayout();
	QToolButton* login_with_this_username_button = new QToolButton();
	QToolButton* request_new_password_button = new QToolButton();

	login_with_this_username_button->setText(lang::LanguagePack::txt_login_with_this_username());
	request_new_password_button->setText(lang::LanguagePack::txt_request_new_password());

	login_with_this_username_button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
	request_new_password_button->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);


	connect(login_with_this_username_button, SIGNAL(clicked()), this, SLOT(slotLoginWithThisUsername()));
	connect(request_new_password_button, SIGNAL(clicked()), this, SLOT(slotRequestNewPassword()));

	layout->setSpacing(5);
	layout->setAlignment(Qt::AlignCenter);
	layout->addWidget(login_with_this_username_button);
	layout->addWidget(request_new_password_button);

	return layout;
}

const QString FindPasswordDialog::GetUsername() const {
	return username_;
}

// slots
void FindPasswordDialog::slotLoginWithThisUsername() {
	done(1);
}

void FindPasswordDialog::slotRequestNewPassword() {
	done(2);
}
// slots
