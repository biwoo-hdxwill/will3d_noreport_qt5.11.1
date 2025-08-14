#pragma once
/**=================================================================================================

Project: 			UIPrimitive
File:				text_edit.h
Language:			C++11
Library:			Qt 5.8.0
author:				Seo Seok Man
First date:			2018-04-11
Last modify:		2018-04-11

Copyright (c) 2017 ~ 2018 All rights reserved by HDXWILL.

*===============================================================================================**/
#include <QTextEdit>
#include "uiprimitive_global.h"

class UIPRIMITIVE_EXPORT TextEdit : public QTextEdit {
	Q_OBJECT

public:
	explicit TextEdit(QWidget *parent = nullptr);
	~TextEdit();

private:
	void focusInEvent(QFocusEvent * e);
	void focusOutEvent(QFocusEvent * e);

private:
	QString empty_state_text_;
};
