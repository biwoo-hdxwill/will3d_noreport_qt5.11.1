#include "find_username_dialog.h"

#include <QDebug>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QToolButton>
#include <QLabel>
#include <QLineEdit>
#include <QKeyEvent>

#include "../../Common/Common/language_pack.h"
#include "../../Common/Common/W3Theme.h"
#include "../../Common/Common/W3MessageBox.h"

FindUsernameDialog::FindUsernameDialog(QWidget* parent)
	: CW3Dialog(lang::LanguagePack::txt_find_username_and_password(), parent, CW3Dialog::Theme::Light) {
	SetLayout();
}

FindUsernameDialog::~FindUsernameDialog() {
}

void FindUsernameDialog::SetLayout() {
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

QVBoxLayout* FindUsernameDialog::CreateContentsLayout() {
	QVBoxLayout* layout = new QVBoxLayout();
	QLabel* message = new QLabel();
	email_input_ = new QLineEdit();

	message->setText("Enter your registered email address.");

	layout->setSpacing(5);
	layout->addWidget(message);
	layout->addWidget(email_input_);

	return layout;
}

QHBoxLayout* FindUsernameDialog::CreateButtonLayout() {
	QHBoxLayout* layout = new QHBoxLayout();
	layout->setSpacing(5);
	layout->setAlignment(Qt::AlignCenter);
	
	QToolButton* continue_button = new QToolButton();
	QToolButton* cancel_button = new QToolButton();

	connect(continue_button, SIGNAL(clicked()), this, SLOT(slotContinue()));
	connect(cancel_button, SIGNAL(clicked()), this, SLOT(reject()));

	continue_button->setText(lang::LanguagePack::txt_continue());
	cancel_button->setText(lang::LanguagePack::txt_cancel());

	layout->addWidget(continue_button);
	layout->addWidget(cancel_button);

	return layout;
}

const QString FindUsernameDialog::GetEmail() const {
	return email_input_->text();
}

void FindUsernameDialog::resizeEvent(QResizeEvent* event) {
	CW3Dialog::resizeEvent(event);

	email_input_->setFocus();
}

void FindUsernameDialog::keyPressEvent(QKeyEvent* event) {
	if (event->key() == Qt::Key_Return) {
		slotContinue();
	}
	CW3Dialog::keyPressEvent(event);
}

// slots
void FindUsernameDialog::slotContinue() {
	if (email_input_->text().isEmpty()) {
		CW3MessageBox msgBox("Will3D", "Email is empty.", CW3MessageBox::Information, this, CW3Dialog::Theme::Light);
		msgBox.exec();
		return;
	}

	accept();
}
// slots
