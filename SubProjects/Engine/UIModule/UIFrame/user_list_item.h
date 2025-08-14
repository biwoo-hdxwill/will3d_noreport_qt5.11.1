#pragma once

/*=========================================================================

File:			class UserManagerDialog
Language:		C++11
Library:        Qt 5.8.0
Author:			Jung Dae Gun
First date:		2018-05-25
Last modify:	2016-05-25

=========================================================================*/

#include <QWidget>

#include "uiframe_global.h"

class QLabel;

class UIFRAME_EXPORT UserListItem : public QWidget {
public:
	UserListItem(QWidget* parent = 0);
	~UserListItem();

	void SetName(const QString& name);
	void SetUserType(const QString& user_type);

private:
	QLabel* name_label_ = nullptr;
	QLabel* user_type_label_ = nullptr;
};

