#pragma once
/*=========================================================================

File:			class ImplantListDlg
Language:		C++11
Library:		Qt 5.8, Standard C++ Library
Author:			Seo Seok Man
First date:		2018-04-03
Last modify:	2018-04-03

Copyright (c) 2018 All rights reserved by HDXWILL.

=========================================================================*/
#include "../../Common/Common/W3Dialog.h"
#include "uiframe_global.h"

class QTableWidget;
class QToolButton;

class UIFRAME_EXPORT ImplantListDlg : public CW3Dialog
{
	Q_OBJECT

public:
	ImplantListDlg(const QString& patient_id, QWidget* parent = nullptr);
	virtual ~ImplantListDlg();

signals:
	void sigSelectImplant(int implant_id);

private slots:
	void slotImplantSelected(int, int, int, int);
	void slotClose();

private:
	void InitUI();
	void UpdateList();
	void Connections();
	void Disconnections();

	virtual void closeEvent(QCloseEvent* event) override;

private:
	QTableWidget* implant_list_ = nullptr;
	QToolButton* close_button_ = nullptr;
};
