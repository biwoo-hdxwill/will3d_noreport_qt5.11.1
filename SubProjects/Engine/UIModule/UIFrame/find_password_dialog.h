#pragma once

/*=========================================================================

File:			class FindPasswordDialog
Language:		C++11
Library:        Qt 5.8.0
Author:			Jung Dae Gun
First date:		2018-05-30
Last modify:	2016-05-30

=========================================================================*/

#include "../../Common/Common/W3Dialog.h"

#include "uiframe_global.h"

class UIFRAME_EXPORT FindPasswordDialog : public CW3Dialog {
	Q_OBJECT

public:
	FindPasswordDialog(const QString& username, QWidget* parent = 0);
	~FindPasswordDialog();

	const QString GetUsername() const;

private slots:
	void slotLoginWithThisUsername();
	void slotRequestNewPassword();

private:
	void SetLayout();
	QVBoxLayout* CreateContentsLayout();
	QVBoxLayout* CreateButtonLayout();

private:
	QString username_;
};
