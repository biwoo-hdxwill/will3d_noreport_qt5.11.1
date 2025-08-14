#pragma once
/*=========================================================================
File:			button_list_dialog.h
Language:		C++11
Library:        Qt 5.9.9
Author:			Lim Tae Kyun
First date:		2021-10-19
Last modify:	2021-10-25

Copyright (c) 2018 HDXWILL. All rights reserved.
=========================================================================*/

#include <QDialog>

#include "../../Common/Common/W3Enum.h"
#include "uiframe_global.h"

enum class ButtonID
{
	BTN_WINDOWING = 0,	// 0
	BTN_HIDE_UI,		// 1
	BTN_GRID_ONOFF,		// 2
	BTN_IMPALNT_ANGLE,	// 3
	BTN_CD_USB_EXPORT,	// 4
	BTN_STL_EXPORT,		// 5
	BTN_MEASURE_LIST,	// 6
	//BTN_LIGHTBOX,		// 7
	BTN_END
};

class QVBoxLayout;
class QButtonGroup;
class QToolButton;
class UIFRAME_EXPORT ButtonListDialog : public QDialog
{
	Q_OBJECT
public:
	explicit ButtonListDialog(QWidget* parent = nullptr);
	virtual ~ButtonListDialog();

	ButtonListDialog(const ButtonListDialog&) = delete;
	const ButtonListDialog& operator = (const ButtonListDialog&) = delete;

	void InitButtonSetting(const TabType& type, const int mpr_type = -1);
	void SetCheckedButton(const int id, const bool checked);
	int GetButtonState(const int id) const; // -1 error, 0 false, 1 true

signals:
	void sigButtonToggle(const int id, const bool button_on);
	void sigSyncControlButtonOut();

private slots:
	void slotButtonToggle(const int, const bool);

private:
	virtual void keyReleaseEvent(QKeyEvent* event) override;

	void SetVisibleButton(const int id, const bool is_visible);
	void AllVisibleButton();
	void SetButtonList();
	QToolButton* CreateTextToolButton(const QString& text);

private:
	QVBoxLayout* main_layout_ = nullptr;
	QButtonGroup* button_group_ = nullptr;
};
