#pragma once

/*=========================================================================

File:			class UserManagerDialog
Language:		C++11
Library:        Qt 5.8.0
Author:			Jung Dae Gun
First date:		2018-05-25
Last modify:	2016-05-25

=========================================================================*/

#include "../../Common/Common/W3Dialog.h"
#include "../../../Managers/DBManager/user_database_manager.h"

#include "uiframe_global.h"

class QLineEdit;
class QButtonGroup;
class QHBoxLayout;
class QListWidget;
class QListWidgetItem;

class UIFRAME_EXPORT UserManagerDialog : public CW3Dialog {
	Q_OBJECT

public:
	UserManagerDialog(QWidget* parent = 0);
	~UserManagerDialog();

private slots:
	void slotNewUser();
	void slotApplyChanges();
	void slotDeleteUser();
	void slotUserListRowChanged(int currentRow);

private:
	void SetLayout();
	QVBoxLayout* CreateUserListLayout();
	QVBoxLayout* CreateUserInformationLayout();
	void InitInput();
	void SetUserList();
	void SetUserInformation(int index);
	void SetUserInformation(QListWidgetItem* item);

private:
	QListWidget* user_list_widget_ = nullptr;

	QLineEdit* username_input_ = nullptr;
	QLineEdit* password_input_ = nullptr;
	QLineEdit* name_input_ = nullptr;
	QLineEdit* email_input_ = nullptr;
	QLineEdit* phone_input_ = nullptr;

	QButtonGroup* user_type_radio_group_ = nullptr;

	QToolButton* apply_changes_button_ = nullptr;
	QToolButton* delete_user_button_ = nullptr;
};
