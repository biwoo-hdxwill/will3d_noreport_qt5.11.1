#pragma once
/*=========================================================================
File:			preferences_pacs_server_widget.h
Language:		C++11
Library:        Qt 5.9.9
Author:			Lim Tae Kyun
First date:		2021-06-23
Last modify:	2021-09-07

Copyright (c) 2018 HDXWILL. All rights reserved.
=========================================================================*/

#include "base_widget.h"

class QTableWidget;
class PreferencesPACSServerWidget : public BaseWidget
{
	Q_OBJECT
public:
	explicit PreferencesPACSServerWidget(QWidget *parent = nullptr);
	virtual ~PreferencesPACSServerWidget();

	PreferencesPACSServerWidget(const PreferencesPACSServerWidget&) = delete;
	const PreferencesPACSServerWidget& operator = (const PreferencesPACSServerWidget&) = delete;

private slots:
	void slotPACSServerSelectButton();
	void slotPACSServerAddButton();
	void slotPACSServerDeleteButton();
	void slotAddPACSServer(const QString& nickname, const QString& ae_title, const QString& ip, const QString& port);

private:
	QVBoxLayout* CreateMainContentsLayout();
	QVBoxLayout* CreatePACSServerListLayout();
	QVBoxLayout* CreateSelectedPACSServerLayout();

	void SetPacsTableWidget();
	void SetSelectedPACSServerInfo(const int index);

private:
	QTableWidget* pacs_server_table_widget_ = nullptr;

	QLineEdit* nickname_line_edit_ = nullptr;
	QLineEdit* server_ae_title_line_edit_ = nullptr;
	QLineEdit* server_ip_line_edit_ = nullptr;
	QLineEdit* port_line_edit_ = nullptr;
};
