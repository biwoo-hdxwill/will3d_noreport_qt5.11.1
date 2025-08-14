#pragma once
/*=========================================================================
File:			pacs_add_server_dialog.h
Language:		C++11
Library:        Qt 5.9.9
Author:			Lim Tae Kyun
First date:		2021-06-22
Last modify:	2021-06-22
=========================================================================*/

#include "../../Common/Common/W3Dialog.h"
#include "uiframe_global.h"

class QHBoxLayout;
class QVBoxLayout;
class QLineEdit;
class UIFRAME_EXPORT PACSAddServerDialog : public CW3Dialog
{
	Q_OBJECT
public:
	PACSAddServerDialog(QWidget* parent = nullptr);
	virtual ~PACSAddServerDialog();

	PACSAddServerDialog(const PACSAddServerDialog&) = delete;
	const PACSAddServerDialog& operator = (const PACSAddServerDialog&) = delete;

	inline void set_nickname_list(const QStringList& nickname_list) { nickname_list_ = nickname_list; }

signals:
	void sigAddPACSServerInfo(const QString& nickname, const QString& ae_title, const QString& ip, const QString& port);

private slots:
	void slotOK();

private:
	void SetLayout();

	QHBoxLayout* CreateTopLayout();
	QVBoxLayout* CreateBottomLayout();

private:
	QLineEdit* nickname_line_edit_ = nullptr;
	QLineEdit* server_ae_title_line_edit_ = nullptr;
	QLineEdit* server_ip_line_edit_ = nullptr;
	QLineEdit* port_line_edit_ = nullptr;

	QLabel* err_msg_ = nullptr;

	QStringList nickname_list_;
};
