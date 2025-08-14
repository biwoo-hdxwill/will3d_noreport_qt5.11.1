#pragma once

/**=================================================================================================

Project:		UIFrame
File:			tab_slot_layout.h
Language:		C++11
Library:		Qt 5.8.0
Author:			Tae Hoon Yoo
First date:		2018-11-20
Last modify: 	2018-11-20

	Copyright (c) 2018 HDXWILL. All rights reserved.

 *===============================================================================================**/

#include <QWidget>
#include <qlayout.h>

class TabSlotLayout : public QWidget {
	Q_OBJECT
public:
	explicit TabSlotLayout(QWidget *parent = 0);
	~TabSlotLayout() {}

	TabSlotLayout(const TabSlotLayout&) = delete;
	TabSlotLayout& operator=(TabSlotLayout&) = delete;
public:
	void setViewLayout(QLayout *layout);
	
private:
	QLayout*	main_layout_ = nullptr;
	QLayout*	view_layout_ = nullptr;
};
