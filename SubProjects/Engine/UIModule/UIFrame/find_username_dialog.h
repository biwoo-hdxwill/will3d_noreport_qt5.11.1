#pragma once

/*=========================================================================

File:			class FindUsernameDialog
Language:		C++11
Library:        Qt 5.8.0
Author:			Jung Dae Gun
First date:		2018-05-30
Last modify:	2016-05-30

=========================================================================*/

#include "../../Common/Common/W3Dialog.h"

#include "uiframe_global.h"

class QHBoxLayout;

class UIFRAME_EXPORT FindUsernameDialog : public CW3Dialog {
	Q_OBJECT

public:
	FindUsernameDialog(QWidget* parent = 0);
	~FindUsernameDialog();

	const QString GetEmail() const;

protected:
	void resizeEvent(QResizeEvent* event) override;
	void keyPressEvent(QKeyEvent* event) override;

private slots:
	void slotContinue();

private:
	void SetLayout();
	QVBoxLayout* CreateContentsLayout();
	QHBoxLayout* CreateButtonLayout();

private:
	QLineEdit* email_input_ = nullptr;
};
