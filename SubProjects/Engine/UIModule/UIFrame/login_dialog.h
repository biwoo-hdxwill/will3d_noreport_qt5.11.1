#pragma once

/*=========================================================================

File:			class LoginDialog
Language:		C++11
Library:        Qt 5.8.0
Author:			Jung Dae Gun
First date:		2018-05-25
Last modify:	2016-05-25

=========================================================================*/

#include "../../Common/Common/W3Dialog.h"
#include "../../../Managers/DBManager/user_database_manager.h"

#include "uiframe_global.h"

class QCheckBox;
class QLineEdit;
class QHBoxLayout;

class UIFRAME_EXPORT LoginDialog : public CW3Dialog {
	Q_OBJECT

public:
	LoginDialog(QWidget* parent = 0);
	~LoginDialog();

public slots:
	virtual void reject() override;

protected:
	void resizeEvent(QResizeEvent* event) override;
	void keyPressEvent(QKeyEvent* event) override;

private slots:
	void slotLogin();
	void slotAutoLogin();
	void slotExit();
	void slotForgetUsernamePassword();
	void slotSelectUser();
	void slotSelectAdmin();
	void slotCheckSave(bool checked);
	void slotCheckAutoLogin(bool checked);

private:
	void SetLayout();
	QHBoxLayout* CreateTabLayout();
	QVBoxLayout* CreateContentsLayout();
	QHBoxLayout* CreateButtonLayout();
	void GetValues();

private:
	QLabel* user_indicator_ = nullptr;
	QLabel* admin_indicator_ = nullptr;

	QLineEdit* username_input_ = nullptr;
	QLineEdit* password_input_ = nullptr;

	QCheckBox* save_check_ = nullptr;
	QCheckBox* auto_login_check_ = nullptr;

	QToolButton* forget_username_password_button_ = nullptr;

	User::UserType user_type_ = User::UserType::UsertType_User;
};
