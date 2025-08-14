#pragma once

/*=========================================================================

File:			class NewUserDialog
Language:		C++11
Library:        Qt 5.8.0
Author:			Jung Dae Gun
First date:		2018-05-28
Last modify:	2016-05-28

=========================================================================*/

#include "../../Common/Common/W3Dialog.h"

#include "uiframe_global.h"

class QHBoxLayout;
class QButtonGroup;

class UIFRAME_EXPORT NewUserDialog : public CW3Dialog {
	Q_OBJECT

public:
	NewUserDialog(QWidget* parent = 0);
	~NewUserDialog();

	const QString GetUserName() const;
	const QString GetPassword() const;
	const QString GetName() const;
	const QString GetEmail() const;
	const QString GetPhone() const;
	const int GetUserType() const;

protected:
	void keyPressEvent(QKeyEvent* event) override;

private slots:
	void slotOk();

private:
	void SetLayout();
	QVBoxLayout* CreateContentsLayout();
	QHBoxLayout* CreateButtonLayout();

private:
	QLineEdit* username_input_ = nullptr;
	QLineEdit* password_input_ = nullptr;
	QLineEdit* name_input_ = nullptr;
	QLineEdit* email_input_ = nullptr;
	QLineEdit* phone_input_ = nullptr;

	QButtonGroup* user_type_radio_group_ = nullptr;
};
