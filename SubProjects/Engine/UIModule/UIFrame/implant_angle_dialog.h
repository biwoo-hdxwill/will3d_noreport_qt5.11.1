#pragma once
/*=========================================================================

File:			class ImplantAngleDialog
Language:		C++11
Library:		Qt 5.8, Standard C++ Library
Author:			Seo Seok Man
First date:		2018-08-19
Last modify:	2018-08-19

Copyright (c) 2018 All rights reserved by HDXWILL.

=========================================================================*/
#include "../../Common/Common/W3Dialog.h"

#include "uiframe_global.h"
class QTableWidget;
class QToolButton;
class QComboBox;

class UIFRAME_EXPORT ImplantAngleDialog : public CW3Dialog {
	Q_OBJECT

public:
	ImplantAngleDialog(QWidget *parent = 0);
	virtual ~ImplantAngleDialog();

private slots:
	void slotSelectImplant(const QString& text);

protected:
	virtual void closeEvent(QCloseEvent * e) override;

private:
	void InitUI();
	void UpdateList();
	void Connections();

private:
	QComboBox* select_implant_;
	QTableWidget* implant_list_;
};
